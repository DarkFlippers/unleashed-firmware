/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MJS_TOK_H_
#define MJS_TOK_H_

#include "mjs_internal.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

struct tok {
    int tok;
    int len;
    const char* ptr;
};

struct pstate {
    const char* file_name; /* Source code file name */
    const char* buf; /* Nul-terminated source code buffer */
    const char* pos; /* Current position */
    int line_no; /* Line number */
    int last_emitted_line_no;
    struct mbuf offset_lineno_map;
    int prev_tok; /* Previous token, for prefix increment / decrement */
    struct tok tok; /* Parsed token */
    struct mjs* mjs;
    int start_bcode_idx; /* Index in mjs->bcode at which parsing was started */
    int cur_idx; /* Index in mjs->bcode at which newly generated code is inserted
                  */
    int depth;
};

enum {
    TOK_EOF,
    TOK_INVALID,

    TOK_COLON,
    TOK_SEMICOLON,
    TOK_COMMA,
    TOK_ASSIGN,
    TOK_OPEN_CURLY,
    TOK_CLOSE_CURLY,
    TOK_OPEN_PAREN,
    TOK_CLOSE_PAREN,
    TOK_OPEN_BRACKET,
    TOK_CLOSE_BRACKET,
    TOK_MUL,
    TOK_PLUS,
    TOK_MINUS,
    TOK_DIV,
    TOK_REM,
    TOK_AND,
    TOK_OR,
    TOK_XOR,
    TOK_DOT,
    TOK_QUESTION,
    TOK_NOT,
    TOK_TILDA,
    TOK_LT,
    TOK_GT,
    TOK_LSHIFT,
    TOK_RSHIFT,
    TOK_MINUS_MINUS,
    TOK_PLUS_PLUS,
    TOK_PLUS_ASSIGN,
    TOK_MINUS_ASSIGN,
    TOK_MUL_ASSIGN,
    TOK_DIV_ASSIGN,
    TOK_AND_ASSIGN,
    TOK_OR_ASSIGN,
    TOK_REM_ASSIGN,
    TOK_XOR_ASSIGN,
    TOK_EQ,
    TOK_NE,
    TOK_LE,
    TOK_GE,
    TOK_LOGICAL_AND,
    TOK_LOGICAL_OR,
    TOK_EQ_EQ,
    TOK_NE_NE,
    TOK_LSHIFT_ASSIGN,
    TOK_RSHIFT_ASSIGN,
    TOK_URSHIFT,
    TOK_URSHIFT_ASSIGN,

    TOK_UNARY_PLUS,
    TOK_UNARY_MINUS,
    TOK_POSTFIX_PLUS,
    TOK_POSTFIX_MINUS,

    TOK_NUM = 200, /* Make sure they don't clash with ascii '+', '{', etc */
    TOK_STR,
    TOK_IDENT,
    TOK_KEYWORD_BREAK,
    TOK_KEYWORD_CASE,
    TOK_KEYWORD_CATCH,
    TOK_KEYWORD_CONTINUE,
    TOK_KEYWORD_DEBUGGER,
    TOK_KEYWORD_DEFAULT,
    TOK_KEYWORD_DELETE,
    TOK_KEYWORD_DO,
    TOK_KEYWORD_ELSE,
    TOK_KEYWORD_FALSE,
    TOK_KEYWORD_FINALLY,
    TOK_KEYWORD_FOR,
    TOK_KEYWORD_FUNCTION,
    TOK_KEYWORD_IF,
    TOK_KEYWORD_IN,
    TOK_KEYWORD_INSTANCEOF,
    TOK_KEYWORD_NEW,
    TOK_KEYWORD_NULL,
    TOK_KEYWORD_RETURN,
    TOK_KEYWORD_SWITCH,
    TOK_KEYWORD_THIS,
    TOK_KEYWORD_THROW,
    TOK_KEYWORD_TRUE,
    TOK_KEYWORD_TRY,
    TOK_KEYWORD_TYPEOF,
    TOK_KEYWORD_VAR,
    TOK_KEYWORD_VOID,
    TOK_KEYWORD_WHILE,
    TOK_KEYWORD_WITH,
    TOK_KEYWORD_LET,
    TOK_KEYWORD_UNDEFINED,
    TOK_MAX
};

MJS_PRIVATE void pinit(const char* file_name, const char* buf, struct pstate*);
MJS_PRIVATE int pnext(struct pstate*);
MJS_PRIVATE int mjs_is_ident(int c);
MJS_PRIVATE int mjs_is_digit(int c);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MJS_TOK_H_ */
