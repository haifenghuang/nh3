/*

    MPSL - Minimum Profit Scripting Language 3.x
    Copyright (C) 2003/2012 Angel Ortega <angel@triptico.com>

    mpsl_c.c - Lexer, Parser, Code Generator and Virtual Machine.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    http://triptico.com

*/

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <time.h> /* for clock() */

#include "mpsl.h"


/** tokens **/

typedef enum {
    T_EOP,    T_ERROR,
    T_IF,     T_ELSE,    T_WHILE,   T_BREAK,
    T_LOCAL,  T_GLOBAL,  T_SUB,     T_RETURN,  T_NULL,    T_THIS,
    T_LBRACE, T_RBRACE,  T_LPAREN,  T_RPAREN,  T_LBRACK,  T_RBRACK,
    T_COLON,  T_SEMI,    T_DOT,     T_COMMA,
    T_GT,     T_LT,      T_PIPE,    T_AMP,
    T_PLUS,   T_MINUS,   T_ASTER,   T_SLASH,   T_PERCENT,  
    T_EQUAL,  T_BANG,    T_CARET,
    T_DGT,    T_DLT,     T_DPIPE,   T_DAMP,    T_DPLUS,   T_DMINUS,
    T_GTEQ,   T_LTEQ,    T_PIPEEQ,  T_AMPEQ,
    T_PLUSEQ, T_MINUSEQ, T_ASTEREQ, T_SLASHEQ, T_PERCEQ,
    T_EQEQ,   T_BANGEQ,  T_CARETEQ,
    T_DGTEQ,  T_DLTEQ,   T_DPIPEEQ, T_DAMPEQ,
    T_SYMBOL, T_LITERAL
} mpsl_token_t;


/** compiler structure **/

struct mpsl_c {
    mpsl_token_t token; /* token found */
    wchar_t *token_s;   /* token as string */
    int token_i;        /* token size */
    int token_o;        /* offset to end of token */
    mpdm_t node;        /* generated nodes */
    mpdm_t prg;         /* generated program */
    int x;              /* x source position */
    int y;              /* y source position */
    wchar_t c;          /* last char read from input */
    wchar_t *ptr;       /* program source */
    FILE *f;            /* program file */
    int error;          /* non-zero if syntax error */
};


/** lexer **/

static wchar_t nc(struct mpsl_c *c)
/* gets the next char */
{
    c->c = c->ptr ? *(c->ptr++) : fgetwc(c->f);

    /* update position in source */
    if (c->c == L'\n') {
        c->y++;
        c->x = -1;
    }
    else
        c->x++;

    return c->c;
}

static void s_error(mpdm_t s1, mpdm_t s2) { mpdm_hset_s(mpdm_root(), L"ERROR", mpdm_strcat(s1, s2)); }

static void c_error(struct mpsl_c *c)
{
    char tmp[64];

    sprintf(tmp, "%d:%d: ", c->y, c->x);
    s_error(MPDM_MBS(tmp), MPDM_LS(L"syntax error"));

    c->error = 1;
}

void POKE(struct mpsl_c *c, wchar_t k) { c->token_s = mpdm_poke_o(c->token_s, &c->token_i, &c->token_o, &k, sizeof(wchar_t), 1); }

#define STORE(COND) while (COND) { POKE(c, c->c); nc(c); } POKE(c, L'\0')

#define COMP(d,e,de) if (c->c == i) { \
        if (d != -1) { nc(c); t = d; } \
        if (c->c == L'=' && de != -1) { nc(c); t = de; } \
    } else if (c->c == L'=' && e != -1) { nc(c); t = e; }

#define STOKEN(s,v) if (t == T_ERROR && wcscmp(c->token_s, s) == 0) t = v

#define DIGIT(d) ((d) >= L'0' && (d) <= L'9')
#define ALPHA(a) ((a) == L'_' || (((a) >= L'a') && ((a) <= L'z')) || (((a) >= L'A') && ((a) <= L'Z')))
#define ALNUM(d) (DIGIT(d) || ALPHA(d))
#define HEXDG(h) (DIGIT(h) || ((h) >= L'a' && (h) <= L'f') || ((h) >= L'A' && (h) <= L'F'))
#define OCTDG(h) ((h) >= L'0' && (h) <= L'7')


static mpsl_token_t token(struct mpsl_c *c)
{
    mpsl_token_t t = T_ERROR;
    wchar_t i;

    c->token_o = 0;

again:
    i = c->c;

    switch (i) {
    case L' ': case L'\t': case L'\r': case L'\n': nc(c); goto again;
    case L'\0': case WEOF: t = T_EOP; break;
    case L'\'': nc(c); STORE(c->c != L'\''); nc(c); t = T_LITERAL; break;
    case L'{':  t = T_LBRACE;  nc(c); break;
    case L'}':  t = T_RBRACE;  nc(c); break;
    case L'(':  t = T_LPAREN;  nc(c); break;
    case L')':  t = T_RPAREN;  nc(c); break;
    case L'[':  t = T_LBRACK;  nc(c); break;
    case L']':  t = T_RBRACK;  nc(c); break;
    case L':':  t = T_COLON;   nc(c); break;
    case L';':  t = T_SEMI;    nc(c); break;
    case L'.':  t = T_DOT;     nc(c); break;
    case L',':  t = T_COMMA;   nc(c); break;
    case L'>':  t = T_GT;      nc(c); COMP(T_DGT, T_GTEQ, T_DGTEQ); break;
    case L'<':  t = T_LT;      nc(c); COMP(T_DLT, T_LTEQ, T_DLTEQ); break;
    case L'|':  t = T_PIPE;    nc(c); COMP(T_DPIPE, T_PIPEEQ, T_DPIPEEQ); break;
    case L'&':  t = T_AMP;     nc(c); COMP(T_DAMP, T_AMPEQ, T_DAMPEQ); break;
    case L'+':  t = T_PLUS;    nc(c); COMP(T_DPLUS, T_PLUSEQ, -1); break;
    case L'-':  t = T_MINUS;   nc(c); COMP(T_DMINUS, T_MINUSEQ, -1); break;
    case L'=':  t = T_EQUAL;   nc(c); COMP(T_EQEQ, T_EQEQ, -1); break;
    case L'!':  t = T_BANG;    nc(c); COMP(-1, T_BANGEQ, -1); break;
    case L'*':  t = T_ASTER;   nc(c); COMP(-1, T_EQEQ, -1); break;
    case L'%':  t = T_PERCENT; nc(c); COMP(-1, T_PERCEQ, -1); break;
    case L'^':  t = T_CARET;   nc(c); COMP(-1, T_CARETEQ, -1); break;
    case L'/':  t = T_SLASH;   nc(c);
        if (c->c == L'*') {
            /* C-style comments */
            nc(c);
            while (c->c != L'\0' && c->c != WEOF) {
                if (c->c == L'*' && nc(c) == L'/') break;
                nc(c);
            }
            goto again;
        }
        COMP(-1, T_SLASHEQ, -1);
        break;
    case L'"':
        while (nc(c) != L'"') {
            wchar_t m = c->c;

            if (m == L'\\') {
                m = nc(c);
                switch (m) {
                case L'n': m = L'\n';   break;
                case L'r': m = L'\r';   break;
                case L't': m = L'\t';   break;
                case L'e': m = 27;      break;
                case L'\\': m = L'\\';  break;
                case L'"': m = L'"';    break;
                case L'x':
                    break;
                }
            }
            POKE(c, m);
        }
        POKE(c, L'\0');
        t = T_LITERAL;
        break;

    default:
        if (DIGIT(i)) {
            t = T_LITERAL;

            if (i == L'0') {
                POKE(c, c->c); nc(c);

                if (c->c == L'b' || c->c == L'B') {
                    POKE(c, c->c); nc(c);
                    STORE(c->c == L'0' || c->c == L'1');
                    break;
                }
                else
                if (c->c == L'x' || c->c == L'X') {
                    POKE(c, c->c); nc(c);
                    STORE(HEXDG(c->c));
                    break;
                }
                else
                if (OCTDG(c->c)) {
                    STORE(OCTDG(c->c));
                    break;
                }
                else
                if (c->c != L'.')
                    break;
            }

            while (DIGIT(c->c)) { POKE(c, c->c); nc(c); };

            if (c->c == L'.' || c->c == L'e' || c->c == L'E') {
                POKE(c, c->c); nc(c);
                while (DIGIT(c->c)) { POKE(c, c->c); nc(c); };
            }

            POKE(c, '\0');
        }
        else
        if (ALPHA(i)) {
            STORE(ALNUM(c->c));
            STOKEN(L"if",       T_IF);
            STOKEN(L"else",     T_ELSE);
            STOKEN(L"while",    T_WHILE);
            STOKEN(L"break",    T_BREAK);
            STOKEN(L"local",    T_LOCAL);
            STOKEN(L"global",   T_GLOBAL);
            STOKEN(L"sub",      T_SUB);
            STOKEN(L"return",   T_RETURN);
            STOKEN(L"NULL",     T_NULL);
            STOKEN(L"this",     T_THIS);

            if (t == T_ERROR) t = T_SYMBOL;
        }

        break;
    }

    if (t == T_ERROR)
        c_error(c);

    return c->token = t;
}


/** parser **/

typedef enum {
    /* order matters (operator precedence) */
    N_NULL,   N_LITERAL,
    N_ARRAY,  N_HASH,   N_PARTOF,
    N_SUBSCR, N_FUNCAL,
    N_UMINUS, N_NOT,
    N_MOD,    N_DIV,    N_MUL,  N_SUB,  N_ADD,
    N_EQ,     N_NE,     N_GT,   N_GE,   N_LT,  N_LE,
    N_AND,    N_OR,
    N_BINAND, N_BINOR,  N_XOR,  N_SHL,  N_SHR,
    N_IF,     N_WHILE,
    N_NOP,    N_SEQ,
    N_SYMID,  N_SYMVAL, N_ASSIGN,
    N_THIS,
    N_LOCAL,  N_GLOBAL,
    N_SUBDEF, N_RETURN,
    N_VOID,
    N_EOP
} mpsl_node_t;


#define RF(v) mpdm_ref(v)
#define UF(v) mpdm_unref(v)
#define UFND(v) mpdm_unrefnd(v)

static mpdm_t node0(mpsl_node_t t) { mpdm_t r = RF(MPDM_A(1)); mpdm_aset(r, MPDM_I(t), 0); return UFND(r); }
static mpdm_t node1(mpsl_node_t t, mpdm_t n1) { mpdm_t r = RF(node0(t)); mpdm_push(r, n1); return UFND(r); }
static mpdm_t node2(mpsl_node_t t, mpdm_t n1, mpdm_t n2) { mpdm_t r = RF(node1(t, n1)); mpdm_push(r, n2); return UFND(r); }
static mpdm_t node3(mpsl_node_t t, mpdm_t n1, mpdm_t n2, mpdm_t n3) { mpdm_t r = RF(node2(t, n1, n2)); mpdm_push(r, n3); return UFND(r); }

static mpdm_t tstr(struct mpsl_c *c)
/* returns the current token as a string */
{
    mpdm_t r = MPDM_ENS(c->token_s, c->token_o);

    c->token_s = NULL;
    c->token_i = c->token_o = 0;

    return r;
}


static mpdm_t expr(struct mpsl_c *c);
static mpdm_t expr_p(struct mpsl_c *c, mpsl_node_t p_op);

static mpdm_t paren_expr(struct mpsl_c *c)
/* parses a parenthesized expression */
{
    mpdm_t v = NULL;

    if (c->error) {}
    else
    if (c->token == T_LPAREN) {
        token(c);
        v = expr(c);

        if (c->token == T_RPAREN)
            token(c);
        else
            c_error(c);
    }
    else
        c_error(c);

    return v;
}


static mpdm_t term(struct mpsl_c *c)
/* parses a term of an expression */
{
    mpdm_t v = NULL;

    if (c->error) {}
    else
    if (c->token == T_BANG) {
        token(c);
        v = node1(N_NOT, expr_p(c, N_NOT));
    }
    else
    if (c->token == T_MINUS) {
        token(c);
        v = node1(N_UMINUS, expr_p(c, N_UMINUS));
    }
    else
    if (c->token == T_THIS) {
        token(c);
        v = node0(N_THIS);
    }
    else
    if (c->token == T_LPAREN)
        v = paren_expr(c);
    else
    if (c->token == T_LBRACE) {
        /* inline hash */
        token(c);

        v = mpdm_ref(node0(N_HASH));

        while (!c->error && c->token != T_RBRACE) {

            if (c->token == T_SYMBOL) {
                mpdm_push(v, node1(N_LITERAL, tstr(c)));
                token(c);
            }
            else
                mpdm_push(v, expr(c));

            if (c->token == T_COLON) {
                token(c);

                mpdm_push(v, expr(c));

                if (c->token == T_COMMA)
                    token(c);
            }
            else
                c_error(c);
        }

        mpdm_unrefnd(v);
        token(c);
    }
    else
    if (c->token == T_LBRACK) {
        /* inline array */
        token(c);

        v = mpdm_ref(node0(N_ARRAY));

        while (!c->error && c->token != T_RBRACK) {
            mpdm_push(v, expr(c));

            if (c->token == T_COMMA)
                token(c);
        }

        mpdm_unrefnd(v);
        token(c);
    }
    else
    if (c->token == T_LITERAL) {
        v = node1(N_LITERAL, tstr(c));
        token(c);
    }
    else
    if (c->token == T_SYMBOL) {
        v = node1(N_SYMID, tstr(c));
        token(c);
    }

    return v;
}


static mpsl_node_t node_by_token(struct mpsl_c *c)
/* returns the operand associated by a token */
{
    int n;
    static int tokens[] = {
        T_LBRACK, T_DOT, T_PLUS, T_MINUS, T_ASTER, T_SLASH, T_PERCENT, 
        T_LPAREN, T_EQEQ, T_BANGEQ, T_GT, T_GTEQ, T_LT, T_LTEQ, 
        T_DAMP, T_DPIPE, T_LOCAL, T_GLOBAL, T_EQUAL,
        T_AMP, T_PIPE, T_CARET, T_DLT, T_DGT, -1
    };
    static mpsl_node_t binop[] = {
        N_SUBSCR, N_PARTOF, N_ADD, N_SUB, N_MUL, N_DIV, N_MOD,
        N_FUNCAL, N_EQ, N_NE, N_GT, N_GE, N_LT, N_LE,
        N_AND, N_OR, N_LOCAL, N_GLOBAL, N_ASSIGN,
        N_BINAND, N_BINOR, N_XOR, N_SHL, N_SHR, -1
    };

    for (n = 0; tokens[n] != -1; n++)
        if (c->token == tokens[n])
            break;

    return binop[n];
}


static mpdm_t expr_p(struct mpsl_c *c, mpsl_node_t p_op)
/* returns an expression, with the previous operand for precedence */
{
    mpdm_t v = NULL;

    if (c->error) {}
    else {
        mpsl_token_t t = c->token;
        mpsl_node_t op;

        v = term(c);

        if (t == T_SYMBOL && c->token != T_EQUAL)
            v = node1(N_SYMVAL, v);

        while (!c->error && (op = node_by_token(c)) > 0 && op <= p_op) {
            if (c->token == T_LBRACK) {
                /* subindexes */
                token(c);

                v = node2(op, v, expr(c));

                if (c->token == T_RBRACK)
                    token(c);
                else
                    c_error(c);

                if (c->token != T_EQUAL)
                    v = node1(N_SYMVAL, v);
            }
            else
            if (c->token == T_LPAREN) {
                mpdm_t a;

                /* function call */
                token(c);

                a = mpdm_ref(node0(N_ARRAY));

                while (!c->error && c->token != T_RPAREN) {
                    mpdm_push(a, expr(c));

                    if (c->token == T_COMMA)
                        token(c);
                }

                token(c);
                mpdm_unrefnd(a);

                v = node2(N_FUNCAL, a, v);
            }
            else {
                token(c);

                if (op == N_PARTOF && c->token != T_SYMBOL)
                    c_error(c);
                else
                    v = node2(op, v, expr_p(c, op));
            }
        }
    }

    return v;
}


static mpdm_t expr(struct mpsl_c *c)
/* returns a complete expression */
{
    /* call expr_p with the lower precedence */
    return expr_p(c, N_EOP);
}


static mpdm_t statement(struct mpsl_c *c)
/* returns a statement */
{
    mpdm_t v = NULL;
    mpdm_t w;

    if (c->error) {}
    else
    if (c->token == T_IF) {
        token(c);
        if ((w = paren_expr(c)) != NULL) {
            v = node2(N_IF, w, statement(c));

            if (c->token == T_ELSE) {
                token(c);
                mpdm_ref(v);
                mpdm_push(v, statement(c));
                mpdm_unrefnd(v);
            }
        }
    }
    else
    if (c->token == T_WHILE) {
        token(c);
        if ((w = paren_expr(c)) != NULL)
            v = node2(N_WHILE, w, statement(c));
    }
    else
    if (c->token == T_LOCAL || c->token == T_GLOBAL) {
        mpdm_t w1, w2;
        mpsl_node_t op = node_by_token(c);

        token(c);
        v = node0(N_NOP);

        do {
            if (c->token == T_SYMBOL) {
                w1 = node1(N_LITERAL, tstr(c));
                token(c);

                /* has initialization value? */
                if (c->token == T_EQUAL) {
                    token(c);
                    w2 = expr(c);
                }
                else
                    w2 = node0(N_NULL);

                v = node2(N_SEQ, v, node2(op, w1, w2));

                if (c->token == T_COMMA)
                    token(c);
            }
            else
                c_error(c);

        } while (!c->error && c->token != T_SEMI);

        token(c);
    }
    else
    if (c->token == T_SUB) {
        token(c);

        if (c->token == T_SYMBOL) {
            w = term(c);

            /* new symbol name must have only symbols and dots */
            while (!c->error && c->token == T_DOT) {
                token(c);

                if (c->token == T_SYMBOL)
                    w = node2(N_PARTOF, w, term(c));
                else
                    c_error(c);
            }

            /* argument name array */
            mpdm_t a = mpdm_ref(MPDM_A(0));

            /* does it have arguments? */
            if (c->token == T_LPAREN) {
                token(c);

                while (!c->error && c->token == T_SYMBOL) {
                    mpdm_push(a, tstr(c));
                    token(c);

                    if (c->token == T_COMMA)
                        token(c);
                }

                if (c->token == T_RPAREN)
                    token(c);
                else
                    c_error(c);
            }

            v = node3(N_SUBDEF, w, node1(N_LITERAL, a), statement(c));
            mpdm_unref(a);
        }
        else
            c_error(c);
    }
    else
    if (c->token == T_RETURN) {
        token(c);

        v = node1(N_RETURN, c->token == T_SEMI ? node0(N_NULL) : expr(c));

        if (c->token == T_SEMI)
            token(c);
        else
            c_error(c);
    }
    else
    if (c->token == T_LBRACE) {
        /* code block */
        token(c);
        v = node0(N_NOP);

        while (!c->error && c->token != T_RBRACE)
            v = node2(N_SEQ, v, statement(c));

        token(c);
    }
    else {
        /* expression */
        v = node1(N_VOID, expr(c));

        if (c->token == T_SEMI)
            token(c);
        else
            c_error(c);
    }

    return v;
}


static int parse(struct mpsl_c *c)
/* parses an MPSL program and creates a tree of nodes */
{
    mpdm_t v;

    nc(c);
    token(c);

    v = node0(N_NOP);

    while (!c->error && c->token != T_EOP)
        v = node2(N_SEQ, v, statement(c));

    v = node2(N_SEQ, v, node0(N_EOP));

    mpdm_set(&c->node, v);

    return c->error;
}


/** code generator ("assembler") **/

typedef enum {
    OP_EOP,
    OP_LIT, OP_NUL, OP_ARR, OP_HSH, OP_ROO,
    OP_POP, OP_SWP, OP_DUP,
    OP_GET, OP_SET, OP_STI, OP_TBL,
    OP_TPU, OP_TPO, OP_TLT, OP_THS,
    OP_CAL, OP_RET, OP_ARG,
    OP_JMP, OP_JT,  OP_JF,
    OP_AND, OP_OR,  OP_XOR, OP_SHL, OP_SHR,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_NOT, OP_EQ,  OP_NE,  OP_GT,  OP_GE,  OP_LT, OP_LE,
    OP_REM, OP_DMP
} mpsl_op_t;


static int o(struct mpsl_c *c, mpsl_op_t op) { mpdm_push(c->prg, MPDM_I(op)); return mpdm_size(c->prg); }
static int o2(struct mpsl_c *c, mpsl_op_t op, mpdm_t v) { int r = o(c, op); mpdm_push(c->prg, v); return r; }
static void fix(struct mpsl_c *c, int n) { mpdm_aset(c->prg, MPDM_I(mpdm_size(c->prg)), n); }
static int here(struct mpsl_c *c) { return mpdm_size(c->prg); }
#define O(n) gen(c, mpdm_aget(node, n))


static int gen(struct mpsl_c *c, mpdm_t node)
/* generates MPSL VM code from a tree of nodes */
{
    int n, i;

    mpsl_node_t nt = mpdm_ival(mpdm_aget(node, 0));

    switch (nt) {
    case N_NOP:     break;
    case N_EOP:     o(c, OP_EOP); break;
    case N_NULL:    o(c, OP_NUL); break;
    case N_SYMID:   o2(c, OP_LIT, mpdm_aget(node, 1)); o(c, OP_TBL); break;
    case N_LITERAL: o2(c, OP_LIT, mpdm_aget(node, 1)); break;
    case N_SEQ:     O(1); O(2); break;
    case N_ADD:     O(1); O(2); o(c, OP_ADD); break;
    case N_SUB:     O(1); O(2); o(c, OP_SUB); break;
    case N_MUL:     O(1); O(2); o(c, OP_MUL); break;
    case N_DIV:     O(1); O(2); o(c, OP_DIV); break;
    case N_MOD:     O(1); O(2); o(c, OP_MOD); break;
    case N_UMINUS:  o2(c, OP_LIT, MPDM_I(-1)); O(1); o(c, OP_MUL); break;
    case N_NOT:     O(1); o(c, OP_NOT); break;
    case N_EQ:      O(1); O(2); o(c, OP_EQ); break;
    case N_NE:      O(1); O(2); o(c, OP_NE); break;
    case N_GT:      O(1); O(2); o(c, OP_GT); break;
    case N_GE:      O(1); O(2); o(c, OP_GE); break;
    case N_LT:      O(1); O(2); o(c, OP_LT); break;
    case N_LE:      O(1); O(2); o(c, OP_LE); break;
    case N_ASSIGN:  O(1); O(2); o(c, OP_SET); break;
    case N_SYMVAL:  O(1); o(c, OP_GET); break;
    case N_PARTOF:  O(1); o(c, OP_TPU); O(2); o(c, OP_TPO); break;
    case N_THIS:    o(c, OP_THS); break;
    case N_SUBSCR:  O(1); O(2); break;
    case N_VOID:    O(1); o(c, OP_POP); break;
    case N_GLOBAL:  o(c, OP_ROO); O(1); O(2); o(c, OP_STI); break;
    case N_LOCAL:   o(c, OP_TLT); O(1); O(2); o(c, OP_STI); break;
    case N_RETURN:  O(1); o(c, OP_TPO); o(c, OP_RET); break;
    case N_FUNCAL:  O(1); O(2); o(c, OP_CAL); break;
    case N_BINAND:  O(1); O(2); o(c, OP_AND); break;
    case N_BINOR:   O(1); O(2); o(c, OP_OR); break;
    case N_XOR:     O(1); O(2); o(c, OP_XOR); break;
    case N_SHL:     O(1); O(2); o(c, OP_SHL); break;
    case N_SHR:     O(1); O(2); o(c, OP_SHR); break;

    case N_ARRAY:
        o(c, OP_ARR);
        for (n = 1; n < mpdm_size(node); n++) {
            o(c, OP_DUP);
            o2(c, OP_LIT, MPDM_I(n - 1));
            O(n);
            o(c, OP_STI);
        }
        break;

    case N_HASH:
        o(c, OP_HSH);
        for (n = 1; n < mpdm_size(node); n += 2) {
            o(c, OP_DUP);
            O(n);
            O(n + 1);
            o(c, OP_STI);
        }
        break;

    case N_IF:
        O(1); n = o2(c, OP_JF, NULL); O(2);

        if (mpdm_size(node) == 4) {
            i = o2(c, OP_JMP, NULL); fix(c, n); O(3); n = i;
        }

        fix(c, n);

        break;

    case N_WHILE:
        n = here(c); O(1); i = o2(c, OP_JF, NULL); O(2); o2(c, OP_JMP, MPDM_I(n)); fix(c, i); break;

    case N_OR:
        O(1); o(c, OP_DUP); n = o2(c, OP_JT, NULL);
        o(c, OP_POP); O(2); fix(c, n); break;

    case N_AND:
        O(1); o(c, OP_DUP); n = o2(c, OP_JF, NULL);
        o(c, OP_POP); O(2); fix(c, n); break;

    case N_SUBDEF:
        O(1); n = o2(c, OP_LIT, NULL); o(c, OP_SET); i = o2(c, OP_JMP, NULL);
        fix(c, n); O(2); o(c, OP_ARG); O(3); o(c, OP_RET); fix(c, i); break;
    }

    return c->error;
}


/** virtual machine **/

struct mpsl_vm {
    mpdm_t prg;             /* program */
    mpdm_t ctxt;            /* context */
    mpdm_t stack;           /* stack */
    mpdm_t c_stack;         /* call stack */
    mpdm_t symtbl;          /* local symbol table */
    int pc;                 /* program counter */
    int sp;                 /* stack pointer */
    int cs;                 /* call stack pointer */
    int tt;                 /* symbol table top */
    int mode;               /* running mode */
    int ins;                /* # of executed instructions */
};


static void reset_vm(struct mpsl_vm *m, mpdm_t prg)
{
    mpdm_set(&m->prg,   prg);

    if (prg != NULL) {
        mpdm_set(&m->ctxt,  MPDM_A(0));

        m->stack    = mpdm_push(m->ctxt, MPDM_A(0));
        m->c_stack  = mpdm_push(m->ctxt, MPDM_A(0));
        m->symtbl   = mpdm_push(m->ctxt, MPDM_A(0));

        mpdm_push(m->symtbl, mpdm_root());
        mpdm_push(m->symtbl, MPDM_H(0));

        m->pc = m->sp = m->cs = 0;
        m->tt = mpdm_size(m->symtbl);
        m->mode = VM_IDLE;
    }
    else
        mpdm_set(&m->ctxt, NULL);
}


static void vm_error(struct mpsl_vm *m, mpdm_t s1, mpdm_t s2) { m->mode = VM_ERROR; s_error(s1, s2); }

static mpdm_t PUSH(struct mpsl_vm *m, mpdm_t v) { return mpdm_aset(m->stack, v, m->sp++); }
static mpdm_t POP(struct mpsl_vm *m) { return mpdm_aget(m->stack, --m->sp); }
static mpdm_t TOS(struct mpsl_vm *m) { return mpdm_aget(m->stack, m->sp - 1); }
static mpdm_t PC(struct mpsl_vm *m) { return mpdm_aget(m->prg, m->pc++); }

static mpdm_t GET(struct mpsl_vm *m, mpdm_t h, mpdm_t k)
{
    mpdm_t r = NULL;

    if (MPDM_IS_HASH(h))
        r = mpdm_hget(h, k);
    else
    if (MPDM_IS_ARRAY(h))
        r = mpdm_aget(h, mpdm_ival(k)); 
    else
        vm_error(m, MPDM_LS(L"bad holder in get for key "), k);

    return r;
}

static mpdm_t SET(struct mpsl_vm *m, mpdm_t h, mpdm_t k, mpdm_t v)
{
    mpdm_t r = NULL;

    if (MPDM_IS_HASH(h))
        r = mpdm_hset(h, k, v);
    else
    if (MPDM_IS_ARRAY(h))
        r = mpdm_aset(h, v, mpdm_ival(k)); 
    else
        vm_error(m, MPDM_LS(L"bad holder in set for key "), k);

    return r;
}

static mpdm_t TBL(struct mpsl_vm *m)
{
    int n;
    mpdm_t s = mpdm_ref(POP(m));
    mpdm_t l = NULL;

    /* local symtable */
    for (n = m->tt - 1; n >= 0; n--) {
        if ((l = mpdm_aget(m->symtbl, n)) || (l = mpdm_aget(m->symtbl, (n = 0))))
            if (mpdm_exists(l, s))
                break;
    }

    if (n < 0)
        vm_error(m, MPDM_LS(L"undefined symbol "), s);
    else {
        PUSH(m, l);
        PUSH(m, s);
    }

    mpdm_unref(s);

    return l;
}


static void ARG(struct mpsl_vm *m)
{
    mpdm_t h, k, v;
    int n;

    h = mpdm_ref(MPDM_H(0));
    k = POP(m);
    v = POP(m);

    for (n = 0; n < mpdm_size(k); n++)
        mpdm_hset(h, mpdm_aget(k, n), mpdm_aget(v, n));
    for (; n < mpdm_size(k); n++)
        mpdm_hset(h, mpdm_aget(k, n), NULL);

    mpdm_aset(m->symtbl, h, m->tt++); 

    mpdm_unref(h);
}


#define IPOP(m) mpdm_ival(POP(m))
#define RPOP(m) mpdm_rval(POP(m))
#define ISTRU(v) mpdm_ival(v)
#define BOOL(i) MPDM_I(i)

static int exec_vm(struct mpsl_vm *m, int msecs)
{
    clock_t max;
    mpdm_t v, w;
    double r1, r2;
    int i1, i2;

    /* maximum running time */
    max = msecs ? (clock() + (msecs * CLOCKS_PER_SEC) / 1000) : 0;

    /* start running if there is no error */
    if (m->mode != VM_ERROR)
        m->mode = VM_RUNNING;

    m->ins = 0;

    while (m->mode == VM_RUNNING) {

        /* get the opcode */
        mpsl_op_t opcode = mpdm_ival(PC(m));
    
        switch (opcode) {
        case OP_EOP: m->mode = VM_IDLE; break;
        case OP_LIT: PUSH(m, mpdm_clone(PC(m))); break;
        case OP_NUL: PUSH(m, NULL); break;
        case OP_ARR: PUSH(m, MPDM_A(0)); break;
        case OP_HSH: PUSH(m, MPDM_H(0)); break;
        case OP_ROO: PUSH(m, mpdm_root()); break;
        case OP_POP: --m->sp; break;
        case OP_SWP: v = POP(m); w = RF(POP(m)); PUSH(m, v); UF(PUSH(m, w)); break;
        case OP_DUP: PUSH(m, TOS(m)); break;
        case OP_TBL: TBL(m); break;
        case OP_GET: w = POP(m); v = POP(m); PUSH(m, GET(m, v, w)); break;
        case OP_SET: w = POP(m); v = POP(m); PUSH(m, SET(m, POP(m), v, w)); break;
        case OP_STI: w = POP(m); v = POP(m); SET(m, POP(m), v, w); break;
        case OP_TPU: mpdm_aset(m->symtbl, POP(m), m->tt++); break;
        case OP_TPO: --m->tt; break;
        case OP_TLT: PUSH(m, mpdm_aget(m->symtbl, m->tt - 1)); break;
        case OP_THS: PUSH(m, mpdm_aget(m->symtbl, m->tt - 2)); break;
        case OP_RET: m->pc = mpdm_ival(mpdm_aget(m->c_stack, --m->cs)); break;
        case OP_ARG: ARG(m); break;
        case OP_JMP: m->pc = mpdm_ival(PC(m)); break;
        case OP_JT:  if (ISTRU(POP(m))) m->pc = mpdm_ival(PC(m)); else m->pc++; break;
        case OP_JF:  if (!ISTRU(POP(m))) m->pc = mpdm_ival(PC(m)); else m->pc++; break;
        case OP_ADD: r2 = RPOP(m); r1 = RPOP(m); PUSH(m, MPDM_R(r1 + r2)); break;
        case OP_SUB: r2 = RPOP(m); r1 = RPOP(m); PUSH(m, MPDM_R(r1 - r2)); break;
        case OP_MUL: r2 = RPOP(m); r1 = RPOP(m); PUSH(m, MPDM_R(r1 * r2)); break;
        case OP_DIV: r2 = RPOP(m); r1 = RPOP(m); PUSH(m, MPDM_R(r1 / r2)); break;
        case OP_MOD: i2 = IPOP(m); i1 = IPOP(m); PUSH(m, MPDM_I(i1 % i2)); break;
        case OP_NOT: PUSH(m, MPDM_I(!ISTRU(POP(m)))); break;
        case OP_EQ:  r2 = RPOP(m); r1 = RPOP(m); PUSH(m, BOOL(r1 == r2)); break;
        case OP_NE:  r2 = RPOP(m); r1 = RPOP(m); PUSH(m, BOOL(r1 != r2)); break;
        case OP_GT:  r2 = RPOP(m); r1 = RPOP(m); PUSH(m, BOOL(r1 >  r2)); break;
        case OP_GE:  r2 = RPOP(m); r1 = RPOP(m); PUSH(m, BOOL(r1 >= r2)); break;
        case OP_LT:  r2 = RPOP(m); r1 = RPOP(m); PUSH(m, BOOL(r1 <  r2)); break;
        case OP_LE:  r2 = RPOP(m); r1 = RPOP(m); PUSH(m, BOOL(r1 <= r2)); break;
        case OP_AND: i2 = IPOP(m); i1 = IPOP(m); PUSH(m, MPDM_I(i1 &  i2)); break;
        case OP_OR:  i2 = IPOP(m); i1 = IPOP(m); PUSH(m, MPDM_I(i1 |  i2)); break;
        case OP_XOR: i2 = IPOP(m); i1 = IPOP(m); PUSH(m, MPDM_I(i1 ^  i2)); break;
        case OP_SHL: i2 = IPOP(m); i1 = IPOP(m); PUSH(m, MPDM_I(i1 << i2)); break;
        case OP_SHR: i2 = IPOP(m); i1 = IPOP(m); PUSH(m, MPDM_I(i1 >> i2)); break;
        case OP_REM: m->pc++; break;
        case OP_DMP: mpdm_dump(POP(m)); break;
        case OP_CAL:
            v = POP(m);
            if (MPDM_IS_EXEC(v))
                mpdm_exec(v, POP(m), NULL);
            else {
                mpdm_aset(m->c_stack, MPDM_I(m->pc), m->cs++);
                m->pc = mpdm_ival(v);
            }
            break;
        }

        m->ins++;

        /* if out of slice time, break */        
        if (max && clock() > max)
            m->mode = VM_TIMEOUT;
    }

    return m->mode;
}


static mpdm_t exec_vm_a0(mpdm_t c, mpdm_t a, mpdm_t ctxt)
{
    mpdm_t r = NULL;
    struct mpsl_vm m;

    memset(&m, '\0', sizeof(m));
    reset_vm(&m, c);

    r = MPDM_I(exec_vm(&m, 0));

    reset_vm(&m, NULL);

    return r;
}


mpdm_t mpsl_compile(mpdm_t src)
/* compiles an MPSL source to MPSL VM code */
{
    mpdm_t r = NULL;
    struct mpsl_c c;
    FILE *f;

    mpdm_ref(src);

    memset(&c, '\0', sizeof(c));
    mpdm_set(&c.prg, MPDM_A(0));

    /* src can be a file or a string */
    if ((f = mpdm_get_filehandle(src)) != NULL)
        c.f = f;
    else
        c.ptr = mpdm_string(src);

    if (parse(&c) == 0 && gen(&c, c.node) == 0)
        r = MPDM_X2(exec_vm_a0, c.prg);

    mpdm_unref(src);

    /* cleanup */
    mpdm_set(&c.node,   NULL);
    mpdm_set(&c.prg,    NULL);

    return r;
}


void mpsl_disasm(mpdm_t prg)
{
    int n;
    static char *ops[] = {
        "EOP",
        "LIT", "NUL", "ARR", "HSH", "ROO",
        "POP", "SWP", "DUP",
        "GET", "SET", "STI", "TBL",
        "TPU", "TPO", "TLT", "THS",
        "CAL", "RET", "ARG",
        "JMP", "JT", "JF",
        "AND", "OR", "XOR", "SHL", "SHR",
        "ADD", "SUB", "MUL", "DIV", "MOD",
        "NOT", "EQ", "NE", "GT", "GE",
        "REM", "DMP"
    };

    mpdm_ref(prg);

    for (n = 0; n < mpdm_size(prg); n++) {
        mpsl_op_t i = mpdm_ival(mpdm_aget(prg, n));

        printf("%4d: ", n);
        printf("%s", ops[i]);

        if (i == OP_LIT || i == OP_REM)
            printf(" \"%ls\"", mpdm_string(mpdm_aget(prg, ++n)));
        if (i == OP_JMP || i == OP_JT || i == OP_JF)
            printf(" %d", mpdm_ival(mpdm_aget(prg, ++n)));

        printf("\n");
    }

    mpdm_unref(prg);
}
