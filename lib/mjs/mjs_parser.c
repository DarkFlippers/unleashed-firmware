/*
 * Copyright (c) 2017 Cesanta Software Limited
 * All rights reserved
 */

#include "common/cs_varint.h"

#include "mjs_bcode.h"
#include "mjs_core.h"
#include "mjs_internal.h"
#include "mjs_parser.h"
#include "mjs_string.h"
#include "mjs_tok.h"

#ifndef MAX_TOKS_IN_EXPR
#define MAX_TOKS_IN_EXPR 40
#endif

#define FAIL_ERR(p, code)                                                                \
    do {                                                                                 \
        mjs_set_errorf(                                                                  \
            p->mjs, code, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); \
        return code;                                                                     \
    } while(0)

#define pnext1(p)                                        \
    do {                                                 \
        LOG(LL_VERBOSE_DEBUG, ("  PNEXT %d", __LINE__)); \
        pnext(p);                                        \
    } while(0)

#define SYNTAX_ERROR(p) FAIL_ERR(p, MJS_SYNTAX_ERROR)
#undef EXPECT
#define EXPECT(p, t)        \
    if((p)->tok.tok != (t)) \
        SYNTAX_ERROR(p);    \
    else                    \
        pnext1(p);

static mjs_err_t parse_statement(struct pstate* p);
static mjs_err_t parse_expr(struct pstate* p);

static int ptest(struct pstate* p) {
    struct pstate saved = *p;
    int tok = pnext(p);
    *p = saved;
    return tok;
}

static int s_unary_ops[] = {
    TOK_NOT,
    TOK_TILDA,
    TOK_PLUS_PLUS,
    TOK_MINUS_MINUS,
    TOK_KEYWORD_TYPEOF,
    TOK_MINUS,
    TOK_PLUS,
    TOK_EOF};
static int s_comparison_ops[] = {TOK_LT, TOK_LE, TOK_GT, TOK_GE, TOK_EOF};
static int s_postfix_ops[] = {TOK_PLUS_PLUS, TOK_MINUS_MINUS, TOK_EOF};
static int s_equality_ops[] = {TOK_EQ, TOK_NE, TOK_EQ_EQ, TOK_NE_NE, TOK_EOF};
static int s_assign_ops[] = {
    TOK_ASSIGN,
    TOK_PLUS_ASSIGN,
    TOK_MINUS_ASSIGN,
    TOK_MUL_ASSIGN,
    TOK_DIV_ASSIGN,
    TOK_REM_ASSIGN,
    TOK_LSHIFT_ASSIGN,
    TOK_RSHIFT_ASSIGN,
    TOK_URSHIFT_ASSIGN,
    TOK_AND_ASSIGN,
    TOK_XOR_ASSIGN,
    TOK_OR_ASSIGN,
    TOK_EOF};

static int findtok(int* toks, int tok) {
    int i = 0;
    while(tok != toks[i] && toks[i] != TOK_EOF) i++;
    return toks[i];
}

static void emit_op(struct pstate* pstate, int tok) {
    assert(tok >= 0 && tok <= 255);
    emit_byte(pstate, OP_EXPR);
    emit_byte(pstate, (uint8_t)tok);
}

#define BINOP_STACK_FRAME_SIZE 16
#define STACK_LIMIT 8192

// Intentionally left as macro rather than a function, to let the
// compiler to inline calls and mimimize runtime stack usage.
#define PARSE_LTR_BINOP(p, f1, f2, ops, prev_op)                                    \
    do {                                                                            \
        mjs_err_t res = MJS_OK;                                                     \
        p->depth++;                                                                 \
        if(p->depth > (STACK_LIMIT / BINOP_STACK_FRAME_SIZE)) {                     \
            mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parser stack overflow");      \
            res = MJS_SYNTAX_ERROR;                                                 \
            goto binop_clean;                                                       \
        }                                                                           \
        if((res = f1(p, TOK_EOF)) != MJS_OK) goto binop_clean;                      \
        if(prev_op != TOK_EOF) emit_op(p, prev_op);                                 \
        if(findtok(ops, p->tok.tok) != TOK_EOF) {                                   \
            int op = p->tok.tok;                                                    \
            size_t off_if = 0;                                                      \
            /* For AND/OR, implement short-circuit evaluation */                    \
            if(ops[0] == TOK_LOGICAL_AND || ops[0] == TOK_LOGICAL_OR) {             \
                emit_byte(                                                          \
                    p,                                                              \
                    (uint8_t)(ops[0] == TOK_LOGICAL_AND ? OP_JMP_NEUTRAL_FALSE :    \
                                                          OP_JMP_NEUTRAL_TRUE));    \
                off_if = p->cur_idx;                                                \
                emit_init_offset(p);                                                \
                /* No need to emit TOK_LOGICAL_AND and TOK_LOGICAL_OR: */           \
                /* Just drop the first value, and evaluate the second one. */       \
                emit_byte(p, (uint8_t)OP_DROP);                                     \
                op = TOK_EOF;                                                       \
            }                                                                       \
            pnext1(p);                                                              \
            if((res = f2(p, op)) != MJS_OK) goto binop_clean;                       \
                                                                                    \
            if(off_if != 0) {                                                       \
                mjs_bcode_insert_offset(                                            \
                    p, p->mjs, off_if, p->cur_idx - off_if - MJS_INIT_OFFSET_SIZE); \
            }                                                                       \
        }                                                                           \
    binop_clean:                                                                    \
        p->depth--;                                                                 \
        return res;                                                                 \
    } while(0)

#define PARSE_RTL_BINOP(p, f1, f2, ops, prev_op)             \
    do {                                                     \
        mjs_err_t res = MJS_OK;                              \
        (void)prev_op;                                       \
        if((res = f1(p, TOK_EOF)) != MJS_OK) return res;     \
        if(findtok(ops, p->tok.tok) != TOK_EOF) {            \
            int op = p->tok.tok;                             \
            pnext1(p);                                       \
            if((res = f2(p, TOK_EOF)) != MJS_OK) return res; \
            emit_op(p, op);                                  \
        }                                                    \
        return res;                                          \
    } while(0)

#if MJS_INIT_OFFSET_SIZE > 0
static void emit_init_offset(struct pstate* p) {
    size_t i;
    for(i = 0; i < MJS_INIT_OFFSET_SIZE; i++) {
        emit_byte(p, 0);
    }
}
#else
static void emit_init_offset(struct pstate* p) {
    (void)p;
}
#endif

static mjs_err_t parse_statement_list(struct pstate* p, int et) {
    mjs_err_t res = MJS_OK;
    int drop = 0;
    pnext1(p);
    while(res == MJS_OK && p->tok.tok != TOK_EOF && p->tok.tok != et) {
        if(drop) emit_byte(p, OP_DROP);
        res = parse_statement(p);
        drop = 1;
        while(p->tok.tok == TOK_SEMICOLON) pnext1(p);
    }

    /*
   * Client code expects statement list to contain a value, so if the statement
   * list was empty, push `undefined`.
   */
    if(!drop) {
        emit_byte(p, OP_PUSH_UNDEF);
    }
    return res;
}

static mjs_err_t parse_block(struct pstate* p, int mkscope) {
    mjs_err_t res = MJS_OK;
    p->depth++;
    if(p->depth > (STACK_LIMIT / BINOP_STACK_FRAME_SIZE)) {
        mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parser stack overflow");
        res = MJS_SYNTAX_ERROR;
        return res;
    }
    LOG(LL_VERBOSE_DEBUG, ("[%.*s]", 10, p->tok.ptr));
    if(mkscope) emit_byte(p, OP_NEW_SCOPE);
    res = parse_statement_list(p, TOK_CLOSE_CURLY);
    EXPECT(p, TOK_CLOSE_CURLY);
    if(mkscope) emit_byte(p, OP_DEL_SCOPE);
    return res;
}

static mjs_err_t parse_function(struct pstate* p) {
    size_t prologue, off;
    int arg_no = 0;
    int name_provided = 0;
    mjs_err_t res = MJS_OK;

    EXPECT(p, TOK_KEYWORD_FUNCTION);

    if(p->tok.tok == TOK_IDENT) {
        /* Function name was provided */
        struct tok tmp = p->tok;
        name_provided = 1;
        emit_byte(p, OP_PUSH_STR);
        emit_str(p, tmp.ptr, tmp.len);
        emit_byte(p, OP_PUSH_SCOPE);
        emit_byte(p, OP_CREATE);
        emit_byte(p, OP_PUSH_STR);
        emit_str(p, tmp.ptr, tmp.len);
        emit_byte(p, OP_FIND_SCOPE);
        pnext1(p);
    }

    emit_byte(p, OP_JMP);
    off = p->cur_idx;
    emit_init_offset(p);

    prologue = p->cur_idx;

    EXPECT(p, TOK_OPEN_PAREN);
    emit_byte(p, OP_NEW_SCOPE);
    // Emit names of function arguments
    while(p->tok.tok != TOK_CLOSE_PAREN) {
        if(p->tok.tok != TOK_IDENT) SYNTAX_ERROR(p);
        emit_byte(p, OP_SET_ARG);
        emit_int(p, arg_no);
        arg_no++;
        emit_str(p, p->tok.ptr, p->tok.len);
        if(ptest(p) == TOK_COMMA) pnext1(p);
        pnext1(p);
    }
    EXPECT(p, TOK_CLOSE_PAREN);
    if((res = parse_block(p, 0)) != MJS_OK) return res;
    emit_byte(p, OP_RETURN);
    prologue += mjs_bcode_insert_offset(p, p->mjs, off, p->cur_idx - off - MJS_INIT_OFFSET_SIZE);
    emit_byte(p, OP_PUSH_FUNC);
    emit_int(p, p->cur_idx - 1 /* OP_PUSH_FUNC */ - prologue);
    if(name_provided) {
        emit_op(p, TOK_ASSIGN);
    }

    return res;
}

static mjs_err_t parse_object_literal(struct pstate* p) {
    mjs_err_t res = MJS_OK;
    EXPECT(p, TOK_OPEN_CURLY);
    emit_byte(p, OP_PUSH_OBJ);
    while(p->tok.tok != TOK_CLOSE_CURLY) {
        if(p->tok.tok != TOK_IDENT && p->tok.tok != TOK_STR) SYNTAX_ERROR(p);
        emit_byte(p, OP_DUP);
        emit_byte(p, OP_PUSH_STR);
        emit_str(p, p->tok.ptr, p->tok.len);
        emit_byte(p, OP_SWAP);
        pnext1(p);
        EXPECT(p, TOK_COLON);
        if((res = parse_expr(p)) != MJS_OK) return res;
        emit_op(p, TOK_ASSIGN);
        emit_byte(p, OP_DROP);
        if(p->tok.tok == TOK_COMMA) {
            pnext1(p);
        } else if(p->tok.tok != TOK_CLOSE_CURLY) {
            SYNTAX_ERROR(p);
        }
    }
    return res;
}

static mjs_err_t parse_array_literal(struct pstate* p) {
    mjs_err_t res = MJS_OK;
    EXPECT(p, TOK_OPEN_BRACKET);
    emit_byte(p, OP_PUSH_ARRAY);
    while(p->tok.tok != TOK_CLOSE_BRACKET) {
        emit_byte(p, OP_DUP);
        if((res = parse_expr(p)) != MJS_OK) return res;
        emit_byte(p, OP_APPEND);
        if(p->tok.tok == TOK_COMMA) pnext1(p);
    }
    return res;
}

static enum mjs_err parse_literal(struct pstate* p, const struct tok* t) {
    struct mbuf* bcode_gen = &p->mjs->bcode_gen;
    enum mjs_err res = MJS_OK;
    int tok = t->tok;
    LOG(LL_VERBOSE_DEBUG, ("[%.*s] %p", p->tok.len, p->tok.ptr, (void*)&t));
    switch(t->tok) {
    case TOK_KEYWORD_FALSE:
        emit_byte(p, OP_PUSH_FALSE);
        break;
    case TOK_KEYWORD_TRUE:
        emit_byte(p, OP_PUSH_TRUE);
        break;
    case TOK_KEYWORD_UNDEFINED:
        emit_byte(p, OP_PUSH_UNDEF);
        break;
    case TOK_KEYWORD_NULL:
        emit_byte(p, OP_PUSH_NULL);
        break;
    case TOK_IDENT: {
        int prev_tok = p->prev_tok;
        int next_tok = ptest(p);
        emit_byte(p, OP_PUSH_STR);
        emit_str(p, t->ptr, t->len);
        emit_byte(p, (uint8_t)(prev_tok == TOK_DOT ? OP_SWAP : OP_FIND_SCOPE));
        if(!findtok(s_assign_ops, next_tok) && !findtok(s_postfix_ops, next_tok) &&
           /* TODO(dfrank): fix: it doesn't work for prefix ops */
           !findtok(s_postfix_ops, prev_tok)) {
            emit_byte(p, OP_GET);
        }
        break;
    }
    case TOK_NUM: {
        double iv, d = strtod(t->ptr, NULL);
        unsigned long uv = strtoul(t->ptr + 2, NULL, 16);
        if(t->ptr[0] == '0' && t->ptr[1] == 'x') d = uv;
        if(modf(d, &iv) == 0) {
            emit_byte(p, OP_PUSH_INT);
            emit_int(p, (int64_t)d);
        } else {
            emit_byte(p, OP_PUSH_DBL);
            emit_str(p, t->ptr, t->len);
        }
        break;
    }
    case TOK_STR: {
        size_t oldlen;
        emit_byte(p, OP_PUSH_STR);
        oldlen = bcode_gen->len;
        embed_string(bcode_gen, p->cur_idx, t->ptr, t->len, EMBSTR_UNESCAPE);
        p->cur_idx += bcode_gen->len - oldlen;
    } break;
    case TOK_OPEN_BRACKET:
        res = parse_array_literal(p);
        break;
    case TOK_OPEN_CURLY:
        res = parse_object_literal(p);
        break;
    case TOK_OPEN_PAREN:
        pnext1(p);
        res = parse_expr(p);
        if(p->tok.tok != TOK_CLOSE_PAREN) SYNTAX_ERROR(p);
        break;
    case TOK_KEYWORD_FUNCTION:
        res = parse_function(p);
        break;
    case TOK_KEYWORD_THIS:
        emit_byte(p, OP_PUSH_THIS);
        break;
    default:
        SYNTAX_ERROR(p);
    }
    if(tok != TOK_KEYWORD_FUNCTION) pnext1(p);
    return res;
}

static mjs_err_t parse_call_dot_mem(struct pstate* p, int prev_op) {
    int ops[] = {TOK_DOT, TOK_OPEN_PAREN, TOK_OPEN_BRACKET, TOK_EOF};
    mjs_err_t res = MJS_OK;
    if((res = parse_literal(p, &p->tok)) != MJS_OK) return res;
    while(findtok(ops, p->tok.tok) != TOK_EOF) {
        if(p->tok.tok == TOK_OPEN_BRACKET) {
            int prev_tok = p->prev_tok;
            EXPECT(p, TOK_OPEN_BRACKET);
            if((res = parse_expr(p)) != MJS_OK) return res;
            emit_byte(p, OP_SWAP);
            EXPECT(p, TOK_CLOSE_BRACKET);
            if(!findtok(s_assign_ops, p->tok.tok) && !findtok(s_postfix_ops, p->tok.tok) &&
               /* TODO(dfrank): fix: it doesn't work for prefix ops */
               !findtok(s_postfix_ops, prev_tok)) {
                emit_byte(p, OP_GET);
            }
        } else if(p->tok.tok == TOK_OPEN_PAREN) {
            EXPECT(p, TOK_OPEN_PAREN);
            emit_byte(p, OP_ARGS);
            while(p->tok.tok != TOK_CLOSE_PAREN) {
                if((res = parse_expr(p)) != MJS_OK) return res;
                if(p->tok.tok == TOK_COMMA) pnext1(p);
            }
            emit_byte(p, OP_CALL);
            EXPECT(p, TOK_CLOSE_PAREN);
        } else if(p->tok.tok == TOK_DOT) {
            EXPECT(p, TOK_DOT);
            if((res = parse_call_dot_mem(p, TOK_DOT)) != MJS_OK) return res;
        }
    }
    (void)prev_op;
    return res;
}

static mjs_err_t parse_postfix(struct pstate* p, int prev_op) {
    mjs_err_t res = MJS_OK;
    if((res = parse_call_dot_mem(p, prev_op)) != MJS_OK) return res;
    if(p->tok.tok == TOK_PLUS_PLUS || p->tok.tok == TOK_MINUS_MINUS) {
        int op = p->tok.tok == TOK_PLUS_PLUS ? TOK_POSTFIX_PLUS : TOK_POSTFIX_MINUS;
        emit_op(p, op);
        pnext1(p);
    }
    return res;
}

static mjs_err_t parse_unary(struct pstate* p, int prev_op) {
    mjs_err_t res = MJS_OK;
    int op = TOK_EOF;
    if(findtok(s_unary_ops, p->tok.tok) != TOK_EOF) {
        op = p->tok.tok;
        pnext1(p);
    }
    if(findtok(s_unary_ops, p->tok.tok) != TOK_EOF) {
        res = parse_unary(p, prev_op);
    } else {
        res = parse_postfix(p, prev_op);
    }
    if(res != MJS_OK) return res;
    if(op != TOK_EOF) {
        if(op == TOK_MINUS) op = TOK_UNARY_MINUS;
        if(op == TOK_PLUS) op = TOK_UNARY_PLUS;
        emit_op(p, op);
    }
    return res;
}

static mjs_err_t parse_mul_div_rem(struct pstate* p, int prev_op) {
    int ops[] = {TOK_MUL, TOK_DIV, TOK_REM, TOK_EOF};
    PARSE_LTR_BINOP(p, parse_unary, parse_mul_div_rem, ops, prev_op);
}

static mjs_err_t parse_plus_minus(struct pstate* p, int prev_op) {
    int ops[] = {TOK_PLUS, TOK_MINUS, TOK_EOF};
    PARSE_LTR_BINOP(p, parse_mul_div_rem, parse_plus_minus, ops, prev_op);
}

static mjs_err_t parse_shifts(struct pstate* p, int prev_op) {
    int ops[] = {TOK_LSHIFT, TOK_RSHIFT, TOK_URSHIFT, TOK_EOF};
    PARSE_LTR_BINOP(p, parse_plus_minus, parse_shifts, ops, prev_op);
}

static mjs_err_t parse_comparison(struct pstate* p, int prev_op) {
    PARSE_LTR_BINOP(p, parse_shifts, parse_comparison, s_comparison_ops, prev_op);
}

static mjs_err_t parse_equality(struct pstate* p, int prev_op) {
    PARSE_LTR_BINOP(p, parse_comparison, parse_equality, s_equality_ops, prev_op);
}

static mjs_err_t parse_bitwise_and(struct pstate* p, int prev_op) {
    int ops[] = {TOK_AND, TOK_EOF};
    PARSE_LTR_BINOP(p, parse_equality, parse_bitwise_and, ops, prev_op);
}

static mjs_err_t parse_bitwise_xor(struct pstate* p, int prev_op) {
    int ops[] = {TOK_XOR, TOK_EOF};
    PARSE_LTR_BINOP(p, parse_bitwise_and, parse_bitwise_xor, ops, prev_op);
}

static mjs_err_t parse_bitwise_or(struct pstate* p, int prev_op) {
    int ops[] = {TOK_OR, TOK_EOF};
    PARSE_LTR_BINOP(p, parse_bitwise_xor, parse_bitwise_or, ops, prev_op);
}

static mjs_err_t parse_logical_and(struct pstate* p, int prev_op) {
    int ops[] = {TOK_LOGICAL_AND, TOK_EOF};
    PARSE_LTR_BINOP(p, parse_bitwise_or, parse_logical_and, ops, prev_op);
}

static mjs_err_t parse_logical_or(struct pstate* p, int prev_op) {
    int ops[] = {TOK_LOGICAL_OR, TOK_EOF};
    PARSE_LTR_BINOP(p, parse_logical_and, parse_logical_or, ops, prev_op);
}

static mjs_err_t parse_ternary(struct pstate* p, int prev_op) {
    mjs_err_t res = MJS_OK;
    if((res = parse_logical_or(p, TOK_EOF)) != MJS_OK) return res;
    if(prev_op != TOK_EOF) emit_op(p, prev_op);

    if(p->tok.tok == TOK_QUESTION) {
        size_t off_if, off_endif, off_else;
        EXPECT(p, TOK_QUESTION);

        emit_byte(p, OP_JMP_FALSE);
        off_if = p->cur_idx;
        emit_init_offset(p);

        if((res = parse_ternary(p, TOK_EOF)) != MJS_OK) return res;

        emit_byte(p, OP_JMP);
        off_else = p->cur_idx;
        emit_init_offset(p);
        off_endif = p->cur_idx;

        emit_byte(p, OP_DROP);

        EXPECT(p, TOK_COLON);
        if((res = parse_ternary(p, TOK_EOF)) != MJS_OK) return res;

        /*
     * NOTE: if inserting offset causes the code to move, off_endif needs to be
     * adjusted
     */
        off_endif += mjs_bcode_insert_offset(
            p, p->mjs, off_else, p->cur_idx - off_else - MJS_INIT_OFFSET_SIZE);

        mjs_bcode_insert_offset(p, p->mjs, off_if, off_endif - off_if - MJS_INIT_OFFSET_SIZE);
    }

    return res;
}

static mjs_err_t parse_assignment(struct pstate* p, int prev_op) {
    PARSE_RTL_BINOP(p, parse_ternary, parse_assignment, s_assign_ops, prev_op);
}

static mjs_err_t parse_expr(struct pstate* p) {
    return parse_assignment(p, TOK_EOF);
}

static mjs_err_t parse_let(struct pstate* p) {
    mjs_err_t res = MJS_OK;
    LOG(LL_VERBOSE_DEBUG, ("[%.*s]", 10, p->tok.ptr));
    EXPECT(p, TOK_KEYWORD_LET);
    for(;;) {
        struct tok tmp = p->tok;
        EXPECT(p, TOK_IDENT);

        emit_byte(p, OP_PUSH_STR);
        emit_str(p, tmp.ptr, tmp.len);
        emit_byte(p, OP_PUSH_SCOPE);
        emit_byte(p, OP_CREATE);

        if(p->tok.tok == TOK_ASSIGN) {
            pnext1(p);
            emit_byte(p, OP_PUSH_STR);
            emit_str(p, tmp.ptr, tmp.len);
            emit_byte(p, OP_FIND_SCOPE);
            if((res = parse_expr(p)) != MJS_OK) return res;
            emit_op(p, TOK_ASSIGN);
        } else {
            emit_byte(p, OP_PUSH_UNDEF);
        }
        if(p->tok.tok == TOK_COMMA) {
            emit_byte(p, OP_DROP);
            pnext1(p);
        }
        if(p->tok.tok == TOK_SEMICOLON || p->tok.tok == TOK_EOF) break;
    }
    return res;
}

static mjs_err_t parse_block_or_stmt(struct pstate* p, int cs) {
    if(ptest(p) == TOK_OPEN_CURLY) {
        return parse_block(p, cs);
    } else {
        return parse_statement(p);
    }
}

static mjs_err_t parse_for_in(struct pstate* p) {
    mjs_err_t res = MJS_OK;
    size_t off_b, off_check_end;

    /* new scope should be pushed before OP_LOOP instruction */
    emit_byte(p, OP_NEW_SCOPE);

    /* Put iterator variable name to the stack */
    if(p->tok.tok == TOK_KEYWORD_LET) {
        EXPECT(p, TOK_KEYWORD_LET);
        emit_byte(p, OP_PUSH_STR);
        emit_str(p, p->tok.ptr, p->tok.len);
        emit_byte(p, OP_PUSH_SCOPE);
        emit_byte(p, OP_CREATE);
    }
    emit_byte(p, OP_PUSH_STR);
    emit_str(p, p->tok.ptr, p->tok.len);

    /* Put object to the stack */
    EXPECT(p, TOK_IDENT);
    EXPECT(p, TOK_KEYWORD_IN);
    parse_expr(p);
    EXPECT(p, TOK_CLOSE_PAREN);

    emit_byte(p, OP_PUSH_UNDEF); /* Push iterator */

    /* Before parsing condition statement, push break/continue offsets  */
    emit_byte(p, OP_LOOP);
    off_b = p->cur_idx;
    emit_init_offset(p);
    emit_byte(p, 0); /* Point OP_CONTINUE to the next instruction */

    emit_byte(p, OP_FOR_IN_NEXT);
    emit_byte(p, OP_DUP);
    emit_byte(p, OP_JMP_FALSE);
    off_check_end = p->cur_idx;
    emit_init_offset(p);

    // Parse loop body
    if(p->tok.tok == TOK_OPEN_CURLY) {
        if((res = parse_statement_list(p, TOK_CLOSE_CURLY)) != MJS_OK) return res;
        pnext1(p);
    } else {
        if((res = parse_statement(p)) != MJS_OK) return res;
    }
    emit_byte(p, OP_DROP);
    emit_byte(p, OP_CONTINUE);

    /* jump cond -> break */
    mjs_bcode_insert_offset(
        p, p->mjs, off_check_end, p->cur_idx - off_check_end - MJS_INIT_OFFSET_SIZE);

    /* NOTE: jump C -> cond link is already established, it's constant: zero */

    emit_byte(p, OP_BREAK);

    /* jump B -> cond */
    mjs_bcode_insert_offset(p, p->mjs, off_b, p->cur_idx - off_b - MJS_INIT_OFFSET_SIZE);

    emit_byte(p, OP_DROP);
    emit_byte(p, OP_DROP);
    emit_byte(p, OP_DROP);
    emit_byte(p, OP_DEL_SCOPE);

    return res;
}

static int check_for_in(struct pstate* p) {
    struct pstate saved = *p;
    int forin = 0;
    if(p->tok.tok == TOK_KEYWORD_LET) pnext1(p);
    if(p->tok.tok == TOK_IDENT) {
        pnext1(p);
        if(p->tok.tok == TOK_KEYWORD_IN) forin = 1;
    }
    *p = saved;
    return forin;
}

static mjs_err_t parse_for(struct pstate* p) {
    mjs_err_t res = MJS_OK;
    size_t off_b, off_c, off_init_end;
    size_t off_incr_begin, off_cond_begin, off_cond_end;
    int buf_cur_idx;

    LOG(LL_VERBOSE_DEBUG, ("[%.*s]", 10, p->tok.ptr));
    EXPECT(p, TOK_KEYWORD_FOR);
    EXPECT(p, TOK_OPEN_PAREN);

    /* Look forward - is it for..in ? */
    if(check_for_in(p)) return parse_for_in(p);

    /*
   * BC is a break+continue offsets (a part of OP_LOOP opcode)
   *
   *  BC init  incr  cond  body  break  del_scope
   *  ||    |  ^     ^  |        ^      ^
   *  ||    +--|-----+  |        |      |
   *  |+-------+        +--------+      |
   *  +---------------------------------+
   *
   * The order to setup links:
   *
   *   cond -> break
   *   init -> cond
   *   C -> incr
   *   B -> del_scope
   */

    /* new scope should be pushed before OP_LOOP instruction */
    emit_byte(p, OP_NEW_SCOPE);

    /* Before parsing condition statement, push break/continue offsets  */
    emit_byte(p, OP_LOOP);
    off_b = p->cur_idx;
    emit_init_offset(p);
    off_c = p->cur_idx;
    emit_init_offset(p);

    /* Parse init statement */
    if(p->tok.tok == TOK_KEYWORD_LET) {
        if((res = parse_let(p)) != MJS_OK) return res;
    } else {
        if((res = parse_expr(p)) != MJS_OK) return res;
    }
    EXPECT(p, TOK_SEMICOLON);
    emit_byte(p, OP_DROP);

    emit_byte(p, OP_JMP);
    off_init_end = p->cur_idx;
    emit_init_offset(p);

    off_incr_begin = p->cur_idx;
    off_cond_begin = p->cur_idx;

    /* Parse cond statement */
    if((res = parse_expr(p)) != MJS_OK) return res;
    EXPECT(p, TOK_SEMICOLON);

    /* Parse incr statement */
    /* Incr statement should be placed before cond, so, adjust cur_idx */
    buf_cur_idx = p->cur_idx;
    p->cur_idx = off_incr_begin;

    if((res = parse_expr(p)) != MJS_OK) return res;
    EXPECT(p, TOK_CLOSE_PAREN);
    emit_byte(p, OP_DROP);

    /*
   * Now incr is inserted before cond, so we adjust cur_idx back, and set
   * off_cond_begin to the correct value
   */
    {
        int incr_size = p->cur_idx - off_incr_begin;
        off_cond_begin += incr_size;
        p->cur_idx = buf_cur_idx + incr_size;
    }

    /* p->cur_idx is now at the end of "cond" */
    /* Exit the loop if false */
    emit_byte(p, OP_JMP_FALSE);
    off_cond_end = p->cur_idx;
    emit_init_offset(p);

    /* Parse loop body */
    if(p->tok.tok == TOK_OPEN_CURLY) {
        if((res = parse_statement_list(p, TOK_CLOSE_CURLY)) != MJS_OK) return res;
        pnext1(p);
    } else {
        if((res = parse_statement(p)) != MJS_OK) return res;
    }
    emit_byte(p, OP_DROP);
    emit_byte(p, OP_CONTINUE);

    /* p->cur_idx is at the "break" item now */

    /* jump cond -> break */
    mjs_bcode_insert_offset(
        p, p->mjs, off_cond_end, p->cur_idx - off_cond_end - MJS_INIT_OFFSET_SIZE);

    /* jump init -> cond (and adjust off_incr_begin which may move) */
    off_incr_begin += mjs_bcode_insert_offset(
        p, p->mjs, off_init_end, off_cond_begin - off_init_end - MJS_INIT_OFFSET_SIZE);

    /* jump C -> incr */
    mjs_bcode_insert_offset(p, p->mjs, off_c, off_incr_begin - off_c - MJS_INIT_OFFSET_SIZE);

    emit_byte(p, OP_BREAK);

    /* jump B -> del_scope */
    mjs_bcode_insert_offset(p, p->mjs, off_b, p->cur_idx - off_b - MJS_INIT_OFFSET_SIZE);

    emit_byte(p, OP_DEL_SCOPE);

    return res;
}

static mjs_err_t parse_while(struct pstate* p) {
    size_t off_cond_end, off_b;
    mjs_err_t res = MJS_OK;

    EXPECT(p, TOK_KEYWORD_WHILE);
    EXPECT(p, TOK_OPEN_PAREN);

    /* new scope should be pushed before OP_LOOP instruction */
    emit_byte(p, OP_NEW_SCOPE);

    /*
   * BC is a break+continue offsets (a part of OP_LOOP opcode)
   *
   *   BC cond body break del_scope
   *   || ^  |      ^     ^
   *   || |  |      |     |
   *   |+-+  +------+     |
   *   +------------------+
   *
   * The order to setup links:
   *
   *    cond -> break
   *    C -> cond
   *    B -> del_scope
   */

    emit_byte(p, OP_LOOP);
    off_b = p->cur_idx;
    emit_init_offset(p);
    emit_byte(p, 0); /* Point OP_CONTINUE to the next instruction */

    // parse condition statement
    if((res = parse_expr(p)) != MJS_OK) return res;
    EXPECT(p, TOK_CLOSE_PAREN);

    // Exit the loop if false
    emit_byte(p, OP_JMP_FALSE);
    off_cond_end = p->cur_idx;
    emit_init_offset(p);

    // Parse loop body
    if(p->tok.tok == TOK_OPEN_CURLY) {
        if((res = parse_statement_list(p, TOK_CLOSE_CURLY)) != MJS_OK) return res;
        pnext1(p);
    } else {
        if((res = parse_statement(p)) != MJS_OK) return res;
    }
    emit_byte(p, OP_DROP);
    emit_byte(p, OP_CONTINUE);

    /* jump cond -> break */
    mjs_bcode_insert_offset(
        p, p->mjs, off_cond_end, p->cur_idx - off_cond_end - MJS_INIT_OFFSET_SIZE);

    /* NOTE: jump C -> cond link is already established, it's constant: zero */

    emit_byte(p, OP_BREAK);

    /* jump B -> cond */
    mjs_bcode_insert_offset(p, p->mjs, off_b, p->cur_idx - off_b - MJS_INIT_OFFSET_SIZE);

    emit_byte(p, OP_DEL_SCOPE);
    return res;
}

static mjs_err_t parse_if(struct pstate* p) {
    size_t off_if, off_endif;
    mjs_err_t res = MJS_OK;
    LOG(LL_VERBOSE_DEBUG, ("[%.*s]", 10, p->tok.ptr));
    EXPECT(p, TOK_KEYWORD_IF);
    EXPECT(p, TOK_OPEN_PAREN);
    if((res = parse_expr(p)) != MJS_OK) return res;

    emit_byte(p, OP_JMP_FALSE);
    off_if = p->cur_idx;
    emit_init_offset(p);

    EXPECT(p, TOK_CLOSE_PAREN);
    if((res = parse_block_or_stmt(p, 1)) != MJS_OK) return res;

    if(p->tok.tok == TOK_KEYWORD_ELSE) {
        /*
     * Else clause is present, so, if the condition is not true, the jump
     * target (off_endif) should be not the current offset, but the offset
     * after jump-over-else opcode
     */
        size_t off_else, off_endelse;
        pnext1(p);
        emit_byte(p, OP_JMP);
        off_else = p->cur_idx;
        emit_init_offset(p);
        off_endif = p->cur_idx;

        emit_byte(p, OP_DROP);
        if((res = parse_block_or_stmt(p, 1)) != MJS_OK) return res;
        off_endelse = p->cur_idx;

        /*
     * NOTE: if inserting offset causes the code to move, off_endif needs to be
     * adjusted
     */
        off_endif += mjs_bcode_insert_offset(
            p, p->mjs, off_else, off_endelse - off_else - MJS_INIT_OFFSET_SIZE);
    } else {
        /* Else clause is not present, so, current offset is a jump target
     * (off_endif) */
        off_endif = p->cur_idx;
    }

    mjs_bcode_insert_offset(p, p->mjs, off_if, off_endif - off_if - MJS_INIT_OFFSET_SIZE);

    return res;
}

static void pstate_revert(struct pstate* p, struct pstate* old, int old_bcode_gen_len) {
    p->pos = old->pos;
    p->line_no = old->line_no;
    p->last_emitted_line_no = old->last_emitted_line_no;
    p->offset_lineno_map.len = old->offset_lineno_map.len;
    p->prev_tok = old->prev_tok;
    p->tok = old->tok;
    p->mjs->bcode_gen.len = old_bcode_gen_len;
    p->cur_idx = old->cur_idx;
    p->depth = old->depth;
}

static mjs_err_t parse_return(struct pstate* p) {
    int old_bcode_gen_len;
    struct pstate p_saved;
    EXPECT(p, TOK_KEYWORD_RETURN);
    p_saved = *p;
    old_bcode_gen_len = p->mjs->bcode_gen.len;
    if(parse_expr(p) != MJS_OK) {
        /*
     * Failed to parse an expression to return, so return the parser to the
     * prior state and push undefined.
     */
        pstate_revert(p, &p_saved, old_bcode_gen_len);
        emit_byte(p, OP_PUSH_UNDEF);
    }
    emit_byte(p, OP_SETRETVAL);
    emit_byte(p, OP_RETURN);
    return MJS_OK;
}

static mjs_err_t parse_statement(struct pstate* p) {
    LOG(LL_VERBOSE_DEBUG, ("[%.*s]", 10, p->tok.ptr));
    switch(p->tok.tok) {
    case TOK_SEMICOLON:
        emit_byte(p, OP_PUSH_UNDEF);
        pnext1(p);
        return MJS_OK;
    case TOK_KEYWORD_LET:
        return parse_let(p);
    case TOK_OPEN_CURLY:
        return parse_block(p, 1);
    case TOK_KEYWORD_RETURN:
        return parse_return(p);
    case TOK_KEYWORD_FOR:
        return parse_for(p);
    case TOK_KEYWORD_WHILE:
        return parse_while(p);
    case TOK_KEYWORD_BREAK:
        emit_byte(p, OP_PUSH_UNDEF);
        emit_byte(p, OP_BREAK);
        pnext1(p);
        return MJS_OK;
    case TOK_KEYWORD_CONTINUE:
        emit_byte(p, OP_CONTINUE);
        pnext1(p);
        return MJS_OK;
    case TOK_KEYWORD_IF:
        return parse_if(p);
    case TOK_KEYWORD_CASE:
    case TOK_KEYWORD_CATCH:
    case TOK_KEYWORD_DELETE:
    case TOK_KEYWORD_DO:
    case TOK_KEYWORD_INSTANCEOF:
    case TOK_KEYWORD_NEW:
    case TOK_KEYWORD_SWITCH:
    case TOK_KEYWORD_THROW:
    case TOK_KEYWORD_TRY:
    case TOK_KEYWORD_VAR:
    case TOK_KEYWORD_VOID:
    case TOK_KEYWORD_WITH:
        mjs_set_errorf(
            p->mjs, MJS_SYNTAX_ERROR, "[%.*s] is not implemented", p->tok.len, p->tok.ptr);
        return MJS_SYNTAX_ERROR;
    default: {
        mjs_err_t res = MJS_OK;
        for(;;) {
            if((res = parse_expr(p)) != MJS_OK) return res;
            if(p->tok.tok != TOK_COMMA) break;
            emit_byte(p, OP_DROP);
            pnext1(p);
        }
        return res;
    }
    }
}

MJS_PRIVATE mjs_err_t mjs_parse(const char* path, const char* buf, struct mjs* mjs) {
    mjs_err_t res = MJS_OK;
    struct pstate p;
    size_t start_idx, llen;
    int map_len;
    mjs_header_item_t bcode_offset, map_offset, total_size;

    pinit(path, buf, &p);
    p.mjs = mjs;
    p.cur_idx = p.mjs->bcode_gen.len;
    emit_byte(&p, OP_BCODE_HEADER);

    /*
   * TODO(dfrank): don't access mjs->bcode_gen directly, use emit_... API which
   * takes care of p->cur_idx
   */

    /* Remember starting bcode position, and reserve the room for bcode header */
    start_idx = p.mjs->bcode_gen.len;
    mbuf_append(&p.mjs->bcode_gen, NULL, sizeof(mjs_header_item_t) * MJS_HDR_ITEMS_CNT);

    /* Append NULL-terminated filename */
    mbuf_append(&p.mjs->bcode_gen, path, strlen(path) + 1 /* null-terminate */);

    bcode_offset = p.mjs->bcode_gen.len - start_idx;
    memcpy(
        p.mjs->bcode_gen.buf + start_idx + sizeof(mjs_header_item_t) * MJS_HDR_ITEM_BCODE_OFFSET,
        &bcode_offset,
        sizeof(mjs_header_item_t));

    p.start_bcode_idx = p.mjs->bcode_gen.len;
    p.cur_idx = p.mjs->bcode_gen.len;

    res = parse_statement_list(&p, TOK_EOF);
    emit_byte(&p, OP_EXIT);

    /* remember map offset */
    map_offset = p.mjs->bcode_gen.len - start_idx;
    memcpy(
        p.mjs->bcode_gen.buf + start_idx + sizeof(mjs_header_item_t) * MJS_HDR_ITEM_MAP_OFFSET,
        &map_offset,
        sizeof(mjs_header_item_t));

    /* put map length varint */
    map_len = p.offset_lineno_map.len;
    llen = cs_varint_llen(map_len);
    mbuf_resize(&p.mjs->bcode_gen, p.mjs->bcode_gen.size + llen);
    cs_varint_encode(map_len, (uint8_t*)p.mjs->bcode_gen.buf + p.mjs->bcode_gen.len, llen);
    p.mjs->bcode_gen.len += llen;

    /* put the map itself */
    mbuf_append(&p.mjs->bcode_gen, p.offset_lineno_map.buf, p.offset_lineno_map.len);

    total_size = p.mjs->bcode_gen.len - start_idx;
    memcpy(
        p.mjs->bcode_gen.buf + start_idx + sizeof(mjs_header_item_t) * MJS_HDR_ITEM_TOTAL_SIZE,
        &total_size,
        sizeof(mjs_header_item_t));

    mbuf_free(&p.offset_lineno_map);

    /*
   * If parsing was successful, commit the bcode; otherwise drop generated
   * bcode
   */
    if(res == MJS_OK) {
        mjs_bcode_commit(mjs);
    } else {
        mbuf_free(&mjs->bcode_gen);
    }

    return res;
}
