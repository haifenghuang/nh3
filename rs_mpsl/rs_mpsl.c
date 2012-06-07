/*

    Reverse Stack MPSL

    Angel Ortega <angel@triptico.com>

    This is an experiment of a reverse-stack MPSL.

    Also, the executor includes a maximum number of
    milliseconds before yielding, with the capability
    of restarting where it left in the subsequent call.

*/

#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <mpdm.h>


/** lexer **/

typedef enum {
    T_EOP, T_ERROR,

    T_IF, T_ELSE, T_WHILE, T_BREAK,
    T_LOCAL, T_GLOBAL, T_SUB, T_RETURN, T_NULL,

    T_LBRACE, T_RBRACE,
    T_LPAREN, T_RPAREN,
    T_LBRACK, T_RBRACK,
    T_COLON, T_SEMI,
    T_DOT, T_COMMA,

    /* can be doubled and/or followed by = */
    T_GT,       T_LT,       T_PIPE,     T_AMPERSAND,

    /* can be doubled or followed by = */
    T_PLUS,     T_MINUS,

    /* can be followed by = */
    T_EQUAL,    T_BANG, 
    T_ASTERISK, T_SLASH,    T_PERCENT,
    T_DGT,      T_DLT,      T_DPIPE,    T_DAMPERSAND,

    /* no more combinations */
    T_DPLUS,    T_DMINUS,

    T_PLUSEQ,   T_MINUSEQ,
    T_GTEQ,     T_LTEQ,     T_PIPEEQ,  T_AMPEREQ,
    T_EQEQ,     T_BANGEQ,
    T_ASTEREQ,  T_SLASHEQ,  T_PERCEQ,
    T_DGTEQ,    T_DLTEQ,    T_DPIPEEQ, T_DAMPEREQ,

    T_SYMBOL, T_LITERAL
} mpsl_token_t;

static wchar_t *tokens_s[] = {
    /* should match token enum */
    L"if", L"else", L"while", L"break",
    L"local", L"global", L"sub", L"return", L"NULL", NULL
};

/* dynamic string manipulation macros */
#ifndef ds_init
struct ds {
    wchar_t *d;
    int p;
    int s;
};
#define ds_init(x) do { x.d = (wchar_t *)0; x.p = x.s = 0; } while(0)
#define ds_rewind(x) x.p = 0;
#define ds_free(x) do { if(x.d) free(x.d); ds_init(x); } while(0)
#define ds_redim(x) do { if(x.p >= x.s) x.d = realloc(x.d, ++x.s * sizeof(wchar_t)); } while(0)
#define ds_poke(x,c) do { ds_redim(x); x.d[x.p++] = c; } while(0)
#define ds_pokes(x,t) do { wchar_t *p = t; while(*p) ds_poke(x, *p++); } while(0)
#endif                          /* ds_init */

struct mpsl_c {
    mpsl_token_t token; /* token found */
    struct ds token_s;  /* token as string */
    mpdm_t node;        /* generated nodes */
    mpdm_t prg;         /* generated program */
    int x;              /* x source position */
    int y;              /* y source position */
    wchar_t c;          /* last char read from input */
    wchar_t *ptr;       /* program source */
    FILE *f;            /* program file */
    int error;          /* non-zero if syntax error */
};


static void next_c(struct mpsl_c *l)
/* gets the next char */
{
    if (l->ptr != NULL)
        l->c = *(l->ptr++);
    else
        l->c = fgetwc(l->f);

    /* update position in source */
    if (l->c == L'\n') {
        l->y++;
        l->x = -1;
    }
    else
        l->x++;
}

#define STORE(COND) while (COND) { \
    ds_poke(l->token_s, l->c); \
    next_c(l); \
    } \
    ds_poke(l->token_s, L'\0')


static int t_blanks(struct mpsl_c *c)
/* skip blanks */
{
    while (c->c == L' ' || c->c == L'\t' || c->c == L'\r' || c->c == L'\n')
        next_c(c);

    return 1;
}


static int t_eop(struct mpsl_c *l)
/* tokenize end of program */
{
    if (l->c == L'\0' || l->c == WEOF) {
        l->token = T_EOP;
        return 0;
    }

    return 1;
}


static int t_martians(struct mpsl_c *c)
/* tokenize funny characters */
{
    int ret = 0;
    wchar_t i = c->c;

    switch (i) {
    case L'{': next_c(c); c->token = T_LBRACE;      break;
    case L'}': next_c(c); c->token = T_RBRACE;      break;
    case L'(': next_c(c); c->token = T_LPAREN;      break;
    case L')': next_c(c); c->token = T_RPAREN;      break;
    case L'[': next_c(c); c->token = T_LBRACK;      break;
    case L']': next_c(c); c->token = T_RBRACK;      break;
    case L':': next_c(c); c->token = T_COLON;       break;
    case L';': next_c(c); c->token = T_SEMI;        break;
    case L'.': next_c(c); c->token = T_DOT;         break;
    case L',': next_c(c); c->token = T_COMMA;       break;
    case L'=': next_c(c); c->token = T_EQUAL;       break;
    case L'!': next_c(c); c->token = T_BANG;        break;
    case L'>': next_c(c); c->token = T_GT;          break;
    case L'<': next_c(c); c->token = T_LT;          break;
    case L'+': next_c(c); c->token = T_PLUS;        break;
    case L'-': next_c(c); c->token = T_MINUS;       break;
    case L'*': next_c(c); c->token = T_ASTERISK;    break;
    case L'/': next_c(c); c->token = T_SLASH;       break;
    case L'%': next_c(c); c->token = T_PERCENT;     break;
    case L'|': next_c(c); c->token = T_PIPE;        break;
    case L'&': next_c(c); c->token = T_AMPERSAND;   break;
    default: ret = 1; break;
    }

    if (!ret) {
        /* is it doubled? */
        if (c->c == i && c->token >= T_GT && c->token <= T_MINUS) {
            next_c(c);
            c->token += (T_DGT - T_GT);
        }

        /* is it followed by = ? */
        if (c->c == L'=' && c->token >= T_PLUS && c->token <= T_DAMPERSAND) {
            next_c(c);
            c->token += (T_PLUSEQ - T_PLUS);
        }
    }

    return ret;
}


static int t_string(struct mpsl_c *l)
/* tokenize strings */
{
    if (l->c == L'"') {
    }

    return 1;
}


static int t_vstring(struct mpsl_c *l)
/* tokenize verbatim strings */
{
    if (l->c == L'\'') {
        next_c(l);
        STORE(l->c != L'\'');
        next_c(l);
        l->token = T_LITERAL;

        return 0;
    }

    return 1;
}


static int t_symbol(struct mpsl_c *l)
/* tokenize symbols and keywords */
{
    if (iswalpha(l->c)) {
        int n;

        ds_poke(l->token_s, l->c);
        next_c(l);

        STORE(iswalnum(l->c));

        /* is it a special token? */
        for (n = 0; tokens_s[n] != NULL; n++) {
            if (wcscmp(l->token_s.d, tokens_s[n]) == 0)
                break;
        }

        if (tokens_s[n] == NULL)
            l->token = T_SYMBOL;
        else
            l->token = n + T_IF;

        return 0;
    }

    return 1;
}


static int t_number(struct mpsl_c *l)
/* tokenize real, integer and scientific numbers */
{
    if (iswdigit(l->c)) {
        /* numbers */
        STORE(iswdigit(l->c));

        /* is it a dot or scientific notation? */
        if (l->c == L'.' || l->c == L'e' || l->c == L'E') {
            /* store it and another set of digits */
            ds_poke(l->token_s, l->c);
            STORE(iswdigit(l->c));
        }

        l->token = T_LITERAL;
        return 0;
    }

    return 1;
}


static int t_nd_number(struct mpsl_c *l)
/* tokenize non-decimal base numbers */
{
    if (l->c == L'0') {
        ds_poke(l->token_s, l->c);
        next_c(l);

        if (l->c == L'.') {
            ds_poke(l->token_s, l->c);
            return t_number(l);
        }
        else
        if (l->c == L'b' || l->c == L'B') {
            /* binary */
            ds_poke(l->token_s, l->c);
            next_c(l);
            STORE(l->c == L'0' || l->c == L'1');
            l->token = T_LITERAL;
            return 0;
        }
        else
        if (l->c == L'x' || l->c == L'X') {
            /* hex */
            ds_poke(l->token_s, l->c);
            next_c(l);
            STORE(iswxdigit(l->c));
            l->token = T_LITERAL;
            return 0;
        }
        else {
            /* octal */
            STORE(l->c >= L'0' && l->c <= L'7');
            l->token = T_LITERAL;
            return 0;
        }
    }

    return 1;
}


static int token(struct mpsl_c *l)
{
    ds_rewind(l->token_s);

    if (t_blanks(l) && t_eop(l) && t_martians(l) && t_string(l) &&
        t_vstring(l) && t_symbol(l) && t_nd_number(l) && t_number(l)) {
        l->error = 1;
        l->token = T_ERROR;
    }

    return l->token;
}


/** parser **/

enum {
    N_LITERAL, N_NULL,
    N_IF, N_WHILE,
    N_NOP, N_SEQ,
    N_SYMID, N_SYMVAL, N_ASSIGN,
    N_UMINUS, N_NOT,
    N_PARTOF, N_EXECSYM,
    N_ADD, N_SUB, N_MUL, N_DIV, N_MOD,
    N_EQ,  N_NE,  N_GT,  N_GE,  N_LT,  N_LE,
    N_AND, N_OR,

    N_ARRAY, N_HASH
};

static mpdm_t node0(int type)
{
    mpdm_t r = mpdm_ref(MPDM_A(1));
    mpdm_aset(r, MPDM_I(type), 0);
    return mpdm_unrefnd(r);
}


static mpdm_t node1(int type, mpdm_t n1)
{
    mpdm_t r = mpdm_ref(MPDM_A(2));
    mpdm_aset(r, MPDM_I(type), 0);
    mpdm_aset(r, n1, 1);
    return mpdm_unrefnd(r);
}


static mpdm_t node2(int type, mpdm_t n1, mpdm_t n2)
{
    mpdm_t r = mpdm_ref(MPDM_A(3));
    mpdm_aset(r, MPDM_I(type), 0);
    mpdm_aset(r, n1, 1);
    mpdm_aset(r, n2, 2);
    return mpdm_unrefnd(r);
}


static mpdm_t symid(struct mpsl_c *c)
{
    mpdm_t v = NULL;

    if (c->error) {}
    else
    if (c->token == T_SYMBOL) {
        mpdm_t s = mpdm_ref(MPDM_S(c->token_s.d));
        token(c);

        if (c->token == T_DOT) {
            token(c);

            if ((v = symid(c)) != NULL)
                v = node2(N_PARTOF, node1(N_SYMID, s), v);
        }
        else
            v = node1(N_SYMID, s);

        mpdm_unref(s);
    }

    return v;
}


static mpdm_t expr(struct mpsl_c *c);

static mpdm_t paren_term(struct mpsl_c *c)
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
            c->error = 2;
    }
    else
        c->error = 2;

    return v;
}


static mpdm_t term(struct mpsl_c *c)
{
    mpdm_t v = NULL;

    if (c->error) {}
    else
    if (c->token == T_LPAREN) {
        v = paren_term(c);
    }
    else
    if (c->token == T_LBRACE) {
        /* inline hash */
        token(c);

        v = mpdm_ref(node0(N_HASH));

        while (!c->error && c->token != T_RBRACE) {
            mpdm_push(v, expr(c));

            if (c->token == T_COLON) {
                token(c);

                mpdm_push(v, expr(c));

                if (c->token == T_COMMA)
                    token(c);
            }
            else
                c->error = 2;
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
        v = node1(N_LITERAL, MPDM_S(c->token_s.d));
        token(c);
    }

    return v;
}


static mpdm_t expr(struct mpsl_c *c)
{
    mpdm_t v = NULL;

    if (c->error) {}
    else
    if ((v = symid(c)) != NULL && c->token == T_EQUAL) {
        token(c);
        v = node2(N_ASSIGN, v, expr(c));
    }
    else {
        if (v == NULL)
            v = term(c);
        else
            v = node1(N_SYMVAL, v);

        if (v != NULL) {
            switch (c->token) {
            case T_PLUS:       token(c); v = node2(N_ADD, v, expr(c)); break;
            case T_MINUS:      token(c); v = node2(N_SUB, v, expr(c)); break;
            case T_ASTERISK:   token(c); v = node2(N_MUL, v, expr(c)); break;
            case T_SLASH:      token(c); v = node2(N_DIV, v, expr(c)); break;
            case T_PERCENT:    token(c); v = node2(N_MOD, v, expr(c)); break;
            case T_EQUAL:      token(c); v = node2(N_EQ, v, expr(c));  break;
            case T_BANGEQ:     token(c); v = node2(N_NE, v, expr(c));  break;
            case T_GT:         token(c); v = node2(N_GT, v, expr(c));  break;
            case T_GTEQ:       token(c); v = node2(N_GE, v, expr(c));  break;
            case T_LT:         token(c); v = node2(N_LT, v, expr(c));  break;
            case T_LTEQ:       token(c); v = node2(N_LE, v, expr(c));  break;
            case T_DAMPERSAND: token(c); v = node2(N_AND, v, expr(c)); break;
            case T_DPIPE:      token(c); v = node2(N_OR, v, expr(c));  break;
            default: break;
            }
        }
    }

    return v;
}


static mpdm_t statement(struct mpsl_c *c)
{
    mpdm_t v = NULL;
    mpdm_t w;

    if (c->error) {}
    else
    if (c->token == T_IF) {
        if ((w = paren_term(c)) != NULL) {
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
        if ((w = paren_term(c)) != NULL)
            v = node2(N_WHILE, w, statement(c));
    }
    else
    if (c->token == T_LOCAL) {
    }
    else
    if (c->token == T_GLOBAL) {
    }
    else
    if (c->token == T_SUB) {
    }
    else
    if (c->token == T_RETURN) {
    }
    else
    if (c->token == T_LBRACE) {
        token(c);
        v = node0(N_NOP);

        while (!c->error && c->token != T_RBRACE)
            v = node2(N_SEQ, v, statement(c));

        token(c);
    }
    else {
        /* expression */
        v = expr(c);

        if (c->token == T_SEMI)
            token(c);
    }

    return v;
}


static void parse(struct mpsl_c *c)
{
    mpdm_t v;

    next_c(c);
    token(c);

    v = node0(N_NOP);

    while (!c->error && c->token != T_EOP)
        v = node2(N_SEQ, v, statement(c));

    mpdm_set(&c->node, v);
}


/** basic MPSL runtime **/

#define mpsl_is_true(v) mpdm_ival(v)

/** virtual machine **/

typedef enum {
    OP_EOP,
    OP_LIT, OP_NUL, OP_ARR, OP_HSH, OP_ROT,
    OP_POP, OP_SWP,
    OP_HGT, OP_HST,
    OP_TPU, OP_TPO,
    OP_CAL, OP_RET,
    OP_JMP, OP_JT, OP_JF,

    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_EQ, OP_NE, OP_LT, OP_LE, OP_GT, OP_GE,
    OP_AND, OP_OR,
    OP_DMP
} mpsl_op_t;

enum {
    VM_IDLE, VM_RUNNING, VM_TIMEOUT, VM_ERROR
};

struct mpsl_vm {
    mpdm_t prg;             /* program */
    mpdm_t stack;           /* stack */
    mpdm_t c_stack;         /* call stack */
    mpdm_t symtbl;          /* local symbol table */
    int pc;                 /* program counter */
    int sp;                 /* stack pointer */
    int cs;                 /* call stack pointer */
    int tt;                 /* symbol table top */
    int mode;               /* running mode */
};


static mpdm_t find_symtbl(struct mpsl_vm *m, mpdm_t s)
/* finds the local symbol table that stores s */
{
    int n;
    mpdm_t l = NULL;

    for (n = m->tt - 1; n >= 0; n--) {
        if ((l = mpdm_aget(m->symtbl, n)) == NULL)
            break;

        if (mpdm_exists(l, s))
            break;
    }

    if (l == NULL || n < 0)
        l = mpdm_root();

    return l;
}


mpdm_t mpsl_get_symbol(struct mpsl_vm *m, mpdm_t s)
{
    return mpdm_hget(find_symtbl(m, s), s);
}


mpdm_t mpsl_set_symbol(struct mpsl_vm *m, mpdm_t s, mpdm_t v)
{
    return mpdm_hset(find_symtbl(m, s), s, v);
}



void mpsl_reset_vm(struct mpsl_vm *m, mpdm_t prg)
{
    if (prg)
        mpdm_set(&m->prg, prg);

    mpdm_set(&m->stack,     MPDM_A(0));
    mpdm_set(&m->c_stack,   MPDM_A(0));

    mpdm_push(mpdm_set(&m->symtbl, MPDM_A(0)), MPDM_H(0));

    m->pc = m->sp = m->cs = m->tt = 0;
    m->mode = VM_IDLE;
}


static mpdm_t PUSH(struct mpsl_vm *m, mpdm_t v) { return mpdm_aset(m->stack, v, m->sp++); }
static mpdm_t POP(struct mpsl_vm *m) { return mpdm_aget(m->stack, --m->sp); }
static mpdm_t TOS(struct mpsl_vm *m) { return mpdm_aget(m->stack, m->sp - 1); }
static mpdm_t PC(struct mpsl_vm *m) { return mpdm_aget(m->prg, m->pc++); }

#define IPOP(m) mpdm_ival(POP(m))
#define RPOP(m) mpdm_rval(POP(m))
#define RF(v) mpdm_ref(v)
#define UF(v) mpdm_unref(v)

#include <time.h>

int mpsl_exec_vm(struct mpsl_vm *m, int msecs)
{
    clock_t max;
    mpdm_t v, w;

    /* maximum running time */
    max = msecs ? (clock() + (msecs * CLOCKS_PER_SEC) / 1000) : 0x7fffffff;

    /* start running if there is no error */
    if (m->mode != VM_ERROR)
        m->mode = VM_RUNNING;

    while (m->mode == VM_RUNNING) {

        /* get the opcode */
        mpsl_op_t opcode = (int) mpdm_ival(PC(m));
    
        switch (opcode) {
        case OP_EOP: m->mode = VM_IDLE; break;
        case OP_LIT: PUSH(m, mpdm_clone(PC(m))); break;
        case OP_NUL: PUSH(m, NULL); break;
        case OP_ARR: PUSH(m, MPDM_A(0)); break;
        case OP_HSH: PUSH(m, MPDM_H(0)); break;
        case OP_ROT: PUSH(m, mpdm_root()); break;
        case OP_POP: --m->sp; break;
        case OP_SWP: v = POP(m); w = RF(POP(m)); PUSH(m, v); UF(PUSH(m, w)); break;
        case OP_HGT: PUSH(m, mpdm_hget(TOS(m), POP(m))); break;
        case OP_HST: PUSH(m, mpdm_hset(TOS(m), POP(m), POP(m))); break;
        case OP_TPU: mpdm_aset(m->symtbl, POP(m), m->tt++); break;
        case OP_TPO: --m->tt; break;
        case OP_CAL: mpdm_aset(m->c_stack, MPDM_I(m->pc), m->cs++); m->pc = IPOP(m); break;
        case OP_RET: m->pc = mpdm_ival(mpdm_aget(m->c_stack, --m->cs)); break;
        case OP_JMP: m->pc = mpdm_ival(PC(m)); break;
        case OP_JT:  if (mpsl_is_true(POP(m))) m->pc = mpdm_ival(PC(m)); else m->pc++; break;
        case OP_JF:  if (!mpsl_is_true(POP(m))) m->pc = mpdm_ival(PC(m)); else m->pc++; break;
        case OP_ADD: PUSH(m, MPDM_R(RPOP(m) + RPOP(m))); break;
        case OP_SUB: PUSH(m, MPDM_R(RPOP(m) - RPOP(m))); break;
        case OP_MUL: PUSH(m, MPDM_R(RPOP(m) * RPOP(m))); break;
        case OP_DIV: PUSH(m, MPDM_R(RPOP(m) / RPOP(m))); break;
        case OP_MOD: PUSH(m, MPDM_I(IPOP(m) % IPOP(m))); break;
        case OP_EQ:  PUSH(m, MPDM_I(RPOP(m) == RPOP(m))); break;
        case OP_NE:  PUSH(m, MPDM_I(RPOP(m) != RPOP(m))); break;
        case OP_LT:  PUSH(m, MPDM_I(RPOP(m) <  RPOP(m))); break;
        case OP_LE:  PUSH(m, MPDM_I(RPOP(m) <= RPOP(m))); break;
        case OP_GT:  PUSH(m, MPDM_I(RPOP(m) >  RPOP(m))); break;
        case OP_GE:  PUSH(m, MPDM_I(RPOP(m) >= RPOP(m))); break;
        case OP_DMP: mpdm_dump(POP(m)); break;
        }

        /* if out of slice time, break */        
        if (clock() > max)
            m->mode = VM_TIMEOUT;
    }

    return m->mode;
}


static mpdm_t add_arg(mpdm_t prg, mpdm_t arg)
{
    return mpdm_push(prg, arg);
}

static mpdm_t add_ins(mpdm_t prg, int opcode)
{
    return mpdm_push(prg, MPDM_I(opcode));
}

#include <string.h>

int main(int argc, char *argv[])
{
    mpdm_t prg;
    struct mpsl_c c;
    struct mpsl_vm m;

    mpdm_startup();

    memset(&m, '\0', sizeof(m));
    memset(&c, '\0', sizeof(c));

/*    lp.ptr = L"a = 1000; b = NULL; while (c) { d; e; }";
    next_c(&lp);
    while (token(&lp) != EOP && lp.token != ERROR)
        printf("%d\n", lp.token);
    printf("%d.\n", lp.token);
*/
    prg = MPDM_A(0);
    mpsl_reset_vm(&m, prg);

    add_ins(prg, OP_HSH);
    add_ins(prg, OP_LIT); add_arg(prg, MPDM_LS(L"number_of_the_beast"));
    add_ins(prg, OP_LIT); add_arg(prg, MPDM_I(666));
    add_ins(prg, OP_HST);
    add_ins(prg, OP_DMP);
    add_ins(prg, OP_LIT); add_arg(prg, MPDM_I(2));
    add_ins(prg, OP_LIT); add_arg(prg, MPDM_I(20));
    add_ins(prg, OP_DIV);
    add_ins(prg, OP_LIT); add_arg(prg, MPDM_R(10));
    add_ins(prg, OP_EQ);
    add_ins(prg, OP_DMP);

    mpsl_exec_vm(&m, 0);

    c.ptr = L"a.c.d = 1000; hash = { 'uno': 1, 'dos': 2 * 6 }; b = 3;";
    parse(&c);

    return 0;
}
