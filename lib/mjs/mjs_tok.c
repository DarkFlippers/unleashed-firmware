/*
 * Copyright (c) 2017 Cesanta Software Limited
 * All rights reserved
 */

#include <stdlib.h>
#include <string.h>

#include "common/cs_dbg.h"
#include "mjs_tok.h"

MJS_PRIVATE void pinit(const char* file_name, const char* buf, struct pstate* p) {
    memset(p, 0, sizeof(*p));
    p->line_no = 1;
    p->last_emitted_line_no = 1;
    p->file_name = file_name;
    p->buf = p->pos = buf;
    mbuf_init(&p->offset_lineno_map, 0);
}

// We're not relying on the target libc ctype, as it may incorrectly
// handle negative arguments, e.g. isspace(-1).
static int mjs_is_space(int c) {
    return c == ' ' || c == '\r' || c == '\n' || c == '\t' || c == '\f' || c == '\v';
}

MJS_PRIVATE int mjs_is_digit(int c) {
    return c >= '0' && c <= '9';
}

static int mjs_is_alpha(int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

MJS_PRIVATE int mjs_is_ident(int c) {
    return c == '_' || c == '$' || mjs_is_alpha(c);
}

// Try to parse a token that can take one or two chars.
static int longtok(struct pstate* p, const char* first_chars, const char* second_chars) {
    if(strchr(first_chars, p->pos[0]) == NULL) return TOK_EOF;
    if(p->pos[1] != '\0' && strchr(second_chars, p->pos[1]) != NULL) {
        p->tok.len++;
        p->pos++;
        return p->pos[-1] << 8 | p->pos[0];
    }
    return p->pos[0];
}

// Try to parse a token that takes exactly 3 chars.
static int longtok3(struct pstate* p, char a, char b, char c) {
    if(p->pos[0] == a && p->pos[1] == b && p->pos[2] == c) {
        p->tok.len += 2;
        p->pos += 2;
        return p->pos[-2] << 16 | p->pos[-1] << 8 | p->pos[0];
    }
    return TOK_EOF;
}

// Try to parse a token that takes exactly 4 chars.
static int longtok4(struct pstate* p, char a, char b, char c, char d) {
    if(p->pos[0] == a && p->pos[1] == b && p->pos[2] == c && p->pos[3] == d) {
        p->tok.len += 3;
        p->pos += 3;
        return p->pos[-3] << 24 | p->pos[-2] << 16 | p->pos[-1] << 8 | p->pos[0];
    }
    return TOK_EOF;
}

static int getnum(struct pstate* p) {
    if(p->pos[0] == '0' && p->pos[1] == 'x') {
        // MSVC6 strtod cannot parse 0x... numbers, thus this ugly workaround.
        strtoul(p->pos + 2, (char**)&p->pos, 16);
    } else {
        strtod(p->pos, (char**)&p->pos);
    }
    p->tok.len = p->pos - p->tok.ptr;
    p->pos--;
    return TOK_NUM;
}

static int is_reserved_word_token(const char* s, int len) {
    const char* reserved[] = {"break",     "case",   "catch", "continue",   "debugger", "default",
                              "delete",    "do",     "else",  "false",      "finally",  "for",
                              "function",  "if",     "in",    "instanceof", "new",      "null",
                              "return",    "switch", "this",  "throw",      "true",     "try",
                              "typeof",    "var",    "void",  "while",      "with",     "let",
                              "undefined", NULL};
    int i;
    if(!mjs_is_alpha(s[0])) return 0;
    for(i = 0; reserved[i] != NULL; i++) {
        if(len == (int)strlen(reserved[i]) && strncmp(s, reserved[i], len) == 0) return i + 1;
    }
    return 0;
}

static int getident(struct pstate* p) {
    while(mjs_is_ident(p->pos[0]) || mjs_is_digit(p->pos[0])) p->pos++;
    p->tok.len = p->pos - p->tok.ptr;
    p->pos--;
    return TOK_IDENT;
}

static int getstr(struct pstate* p) {
    int quote = *p->pos++;
    p->tok.ptr++;
    while(p->pos[0] != '\0' && p->pos[0] != quote) {
        if(p->pos[0] == '\\' && p->pos[1] != '\0' &&
           (p->pos[1] == quote || strchr("bfnrtv\\", p->pos[1]) != NULL)) {
            p->pos += 2;
        } else {
            p->pos++;
        }
    }
    p->tok.len = p->pos - p->tok.ptr;
    return TOK_STR;
}

static void skip_spaces_and_comments(struct pstate* p) {
    const char* pos;
    do {
        pos = p->pos;
        while(mjs_is_space(p->pos[0])) {
            if(p->pos[0] == '\n') p->line_no++;
            p->pos++;
        }
        if(p->pos[0] == '/' && p->pos[1] == '/') {
            while(p->pos[0] != '\0' && p->pos[0] != '\n') p->pos++;
        }
        if(p->pos[0] == '/' && p->pos[1] == '*') {
            p->pos += 2;
            while(p->pos[0] != '\0') {
                if(p->pos[0] == '\n') p->line_no++;
                if(p->pos[0] == '*' && p->pos[1] == '/') {
                    p->pos += 2;
                    break;
                }
                p->pos++;
            }
        }
    } while(pos < p->pos);
}

static int ptranslate(int tok) {
#define DT(a, b) ((a) << 8 | (b))
#define TT(a, b, c) ((a) << 16 | (b) << 8 | (c))
#define QT(a, b, c, d) ((a) << 24 | (b) << 16 | (c) << 8 | (d))
    /* Map token ID produced by mjs_tok.c to token ID produced by lemon */
    /* clang-format off */
  switch (tok) {
    case ':': return TOK_COLON;
    case ';': return TOK_SEMICOLON;
    case ',': return TOK_COMMA;
    case '=': return TOK_ASSIGN;
    case '{': return TOK_OPEN_CURLY;
    case '}': return TOK_CLOSE_CURLY;
    case '(': return TOK_OPEN_PAREN;
    case ')': return TOK_CLOSE_PAREN;
    case '[': return TOK_OPEN_BRACKET;
    case ']': return TOK_CLOSE_BRACKET;
    case '*': return TOK_MUL;
    case '+': return TOK_PLUS;
    case '-': return TOK_MINUS;
    case '/': return TOK_DIV;
    case '%': return TOK_REM;
    case '&': return TOK_AND;
    case '|': return TOK_OR;
    case '^': return TOK_XOR;
    case '.': return TOK_DOT;
    case '?': return TOK_QUESTION;
    case '!': return TOK_NOT;
    case '~': return TOK_TILDA;
    case '<': return TOK_LT;
    case '>': return TOK_GT;
    case DT('<','<'): return TOK_LSHIFT;
    case DT('>','>'): return TOK_RSHIFT;
    case DT('-','-'): return TOK_MINUS_MINUS;
    case DT('+','+'): return TOK_PLUS_PLUS;
    case DT('+','='): return TOK_PLUS_ASSIGN;
    case DT('-','='): return TOK_MINUS_ASSIGN;
    case DT('*','='): return TOK_MUL_ASSIGN;
    case DT('/','='): return TOK_DIV_ASSIGN;
    case DT('&','='): return TOK_AND_ASSIGN;
    case DT('|','='): return TOK_OR_ASSIGN;
    case DT('%','='): return TOK_REM_ASSIGN;
    case DT('^','='): return TOK_XOR_ASSIGN;
    case DT('=','='): return TOK_EQ;
    case DT('!','='): return TOK_NE;
    case DT('<','='): return TOK_LE;
    case DT('>','='): return TOK_GE;
    case DT('&','&'): return TOK_LOGICAL_AND;
    case DT('|','|'): return TOK_LOGICAL_OR;
    case TT('=','=','='): return TOK_EQ_EQ;
    case TT('!','=','='): return TOK_NE_NE;
    case TT('<','<','='): return TOK_LSHIFT_ASSIGN;
    case TT('>','>','='): return TOK_RSHIFT_ASSIGN;
    case TT('>','>','>'): return TOK_URSHIFT;
    case QT('>','>','>','='): return TOK_URSHIFT_ASSIGN;
  }
    /* clang-format on */
    return tok;
}

MJS_PRIVATE int pnext(struct pstate* p) {
    int tmp, tok = TOK_INVALID;

    skip_spaces_and_comments(p);
    p->tok.ptr = p->pos;
    p->tok.len = 1;

    if(p->pos[0] == '\0') {
        tok = TOK_EOF;
    } else if(mjs_is_digit(p->pos[0])) {
        tok = getnum(p);
    } else if(p->pos[0] == '\'' || p->pos[0] == '"') {
        tok = getstr(p);
    } else if(mjs_is_ident(p->pos[0])) {
        tok = getident(p);
        /*
     * NOTE: getident() has side effects on `p`, and `is_reserved_word_token()`
     * relies on them. Since in C the order of evaluation of the operands is
     * undefined, `is_reserved_word_token()` should be called in a separate
     * statement.
     */
        tok += is_reserved_word_token(p->tok.ptr, p->tok.len);
    } else if(strchr(",.:;{}[]()?", p->pos[0]) != NULL) {
        tok = p->pos[0];
    } else if(
        (tmp = longtok3(p, '<', '<', '=')) != TOK_EOF ||
        (tmp = longtok3(p, '>', '>', '=')) != TOK_EOF ||
        (tmp = longtok4(p, '>', '>', '>', '=')) != TOK_EOF ||
        (tmp = longtok3(p, '>', '>', '>')) != TOK_EOF ||
        (tmp = longtok3(p, '=', '=', '=')) != TOK_EOF ||
        (tmp = longtok3(p, '!', '=', '=')) != TOK_EOF ||
        (tmp = longtok(p, "&", "&=")) != TOK_EOF || (tmp = longtok(p, "|", "|=")) != TOK_EOF ||
        (tmp = longtok(p, "<", "<=")) != TOK_EOF || (tmp = longtok(p, ">", ">=")) != TOK_EOF ||
        (tmp = longtok(p, "-", "-=")) != TOK_EOF || (tmp = longtok(p, "+", "+=")) != TOK_EOF) {
        tok = tmp;
    } else if((tmp = longtok(p, "^~+-%/*<>=!|&", "=")) != TOK_EOF) {
        tok = tmp;
    }
    if(p->pos[0] != '\0') p->pos++;
    LOG(LL_VERBOSE_DEBUG, ("  --> %d [%.*s]", tok, p->tok.len, p->tok.ptr));
    p->prev_tok = p->tok.tok;
    p->tok.tok = ptranslate(tok);
    return p->tok.tok;
}
