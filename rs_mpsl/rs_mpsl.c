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

enum {
    T_EOP, T_ERROR,

    T_IF, T_ELSE, T_WHILE, T_BREAK,
    T_LOCAL, T_GLOBAL, T_SUB, T_RETURN, T_NULL,

    T_LBRACE, T_RBRACE,
    T_LPAREN, T_RPAREN,
    T_LBRACK, T_RBRACK,
    T_COLON, T_SEMI,
    T_EQUAL, T_DOT, T_BANG, T_COMMA,
    T_PLUS, T_MINUS, T_ASTERISK, T_SLASH,

    T_SYMBOL, T_LITERAL
};

/* should match token enum */
static wchar_t *tokens_c = L"{}()[]:;=.!+-*/,";

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
    int token;          /* token found */
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
    do {
        if (l->ptr != NULL)
            l->c = *(l->ptr++);
        else
            l->c = fgetwc(l->f);

        /* update position in source */
        if (l->c == L'\n') {
            l->y++;
            l->x = 0;
        }
        else
            l->x++;
    } while (l->c && wcschr(L" \t\r\n", l->c));
}

#define STORE(COND) while (COND) { \
    ds_poke(l->token_s, l->c); \
    next_c(l); \
    } \
    ds_poke(l->token_s, L'\0')


static int tok_eop(struct mpsl_c *l)
{
    if (l->c == L'\0' || l->c == WEOF) {
        l->token = T_EOP;
        return 0;
    }

    return 1;
}


static int tok_1c(struct mpsl_c *l)
{
    wchar_t *ptr;

    if ((ptr = wcschr(tokens_c, l->c)) != NULL) {
        next_c(l);
        l->token = (ptr - tokens_c) + T_LBRACE;
        return 0;
    }

    return 1;
}


static int tok_str(struct mpsl_c *l)
{
    if (l->c == L'"') {
    }

    return 1;
}


static int tok_vstr(struct mpsl_c *l)
{
    if (l->c == L'\'') {
        next_c(l);
        STORE(l->c != L'\'');
        l->token = T_LITERAL;

        return 0;
    }

    return 1;
}


static int tok_sym(struct mpsl_c *l)
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

static int tok_num(struct mpsl_c *l)
{
    if (iswdigit(l->c)) {
        /* numbers */
        ds_poke(l->token_s, l->c);

        /* store while digits */
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


static int tok_specialnum(struct mpsl_c *l)
{
    if (l->c == L'0') {
        ds_poke(l->token_s, l->c);
        next_c(l);

        if (l->c == L'.') {
            ds_poke(l->token_s, l->c);
            return tok_num(l);
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

    if (tok_eop(l) && tok_1c(l) && tok_str(l) && tok_vstr(l) &&
        tok_sym(l) && tok_specialnum(l) && tok_num(l)) {
        l->error = 1;
        l->token = T_ERROR;
    }

    return l->token;
}


/** parser **/

enum {
    N_LITERAL, N_NULL,
    N_IF, N_IFELSE, N_WHILE,
    N_NOP, N_SEQ,
    N_SYMID, N_SYMVAL, N_ASSIGN,
    N_UMINUS, N_NOT,
    N_PARTOF, N_EXECSYM,
    N_ADD, N_SUB, N_MUL, N_DIV,
    N_ARRAY, N_HASH,
    N_EXPR, N_PROG
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
    mpdm_aset(r, n1, 2);
    return mpdm_unrefnd(r);
}


static mpdm_t symid(struct mpsl_c *c)
{
    mpdm_t v = NULL;

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

static mpdm_t term(struct mpsl_c *c)
{
    mpdm_t v = NULL;

    if (c->token == T_LPAREN) {
        /* parenthesized expression */
        token(c);
        v = expr(c);

        if (c->token != T_RPAREN)
            c->error = 2;
    }
    else
    if (c->token == T_LBRACE) {
        /* inline hash */
    }
    else
    if (c->token == T_LBRACK) {
        /* inline array */
        token(c);

        v = mpdm_ref(node0(N_ARRAY));

        while (c->token != T_RBRACK) {
            mpdm_push(v, node1(N_EXPR, expr(c)));

            if (c->token == T_COMMA)
                token(c);
        }

        mpdm_unrefnd(v);
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
            case T_PLUS:      token(c); v = node2(N_ADD, v, expr(c)); break;
            case T_MINUS:     token(c); v = node2(N_SUB, v, expr(c)); break;
            case T_ASTERISK:  token(c); v = node2(N_MUL, v, expr(c)); break;
            case T_SLASH:     token(c); v = node2(N_DIV, v, expr(c)); break;
            }
        }
    }

    return v;
}


static mpdm_t statement(struct mpsl_c *p)
{
    mpdm_t v = NULL;

    if (p->token == T_IF) {
    }
    else
    if (p->token == T_WHILE) {
    }
    else
    if (p->token == T_LOCAL) {
    }
    else
    if (p->token == T_GLOBAL) {
    }
    else
    if (p->token == T_SUB) {
    }
    else
    if (p->token == T_RETURN) {
    }
    else
    if (p->token == T_SEMI) {
        /* compound statement */
        v = node0(N_NOP);
        token(p);
    }
    else
    if (p->token == T_LBRACE) {
        /* block */
    }
    else {
        /* expression */
        v = node1(N_EXPR, expr(p));

        if (p->token == T_SEMI)
            token(p);
        else
            p->error = 2;
    }

    return v;
}


static void parse(struct mpsl_c *l)
{
    next_c(l);
    token(l);

    mpdm_set(&l->node, node1(N_PROG, statement(l)));

    if (l->token != T_EOP)
        l->error = 2;
}


/** basic MPSL runtime **/

#define mpsl_is_true(v) mpdm_ival(v)

/** virtual machine **/

enum {
    OP_EOP,
    OP_LIT, OP_NUL, OP_ARR, OP_HSH, OP_ROT,
    OP_POP, OP_SWP,
    OP_HGT, OP_HST,
    OP_TPU, OP_TPO,
    OP_CAL, OP_RET,
    OP_JMP, OP_JT, OP_JF,

    OP_ADD, OP_SUB, OP_MUL, OP_DIV,
    OP_EQ, OP_NE, OP_LT, OP_LE, OP_GT, OP_GE,
    OP_DUMP
};

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


static mpdm_t PUSH(struct mpsl_vm *m, mpdm_t v)
{
    return mpdm_aset(m->stack, v, m->sp++);
}


static mpdm_t POP(struct mpsl_vm *m)
{
    return mpdm_aget(m->stack, --m->sp);
}


static mpdm_t TOS(struct mpsl_vm *m)
{
    return mpdm_aget(m->stack, m->sp - 1);
}


static mpdm_t PC(struct mpsl_vm *m)
{
    return mpdm_aget(m->prg, m->pc++);
}


#define IPOP(m) mpdm_ival(POP(m))
#define RPOP(m) mpdm_rval(POP(m))
#define RF(v) mpdm_ref(v)
#define UF(v) mpdm_unref(v)

#include <time.h>

int mpsl_exec_vm(struct mpsl_vm *m, int msecs)
{
    clock_t max;
    mpdm_t v, w;
    double v1, v2, r;

    /* maximum running time */
    max = msecs ? (clock() + (msecs * CLOCKS_PER_SEC) / 1000) : 0x7fffffff;

    /* start running if there is no error */
    if (m->mode != VM_ERROR)
        m->mode = VM_RUNNING;

    while (m->mode == VM_RUNNING) {

        /* get the opcode */
        int opcode = mpdm_ival(PC(m));
    
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


        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
            v2 = RPOP(m);
            v1 = RPOP(m);
    
            switch (opcode) {
            case OP_ADD:    r = v1 + v2; break;
            case OP_SUB:    r = v1 - v2; break;
            case OP_MUL:    r = v1 * v2; break;
            case OP_DIV:    r = v1 / v2; break;
            }
    
            PUSH(m, MPDM_R(r));
    
            break;
    
        case OP_EQ:
        case OP_NE:
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:
            v2 = RPOP(m);
            v1 = RPOP(m);
    
            switch (opcode) {
            case OP_EQ:     r = v1 == v2; break;
            case OP_NE:     r = v1 != v2; break;
            case OP_LT:     r = v1 <  v2; break;
            case OP_LE:     r = v1 <= v2; break;
            case OP_GT:     r = v1 >  v2; break;
            case OP_GE:     r = v1 >= v2; break;
            }
    
            PUSH(m, MPDM_I(r));
    
            break;
    
        case OP_DUMP:
            v = POP(m);
            mpdm_dump(v);
            mpdm_void(v);

            break;
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
    add_ins(prg, OP_DUMP);

    mpsl_exec_vm(&m, 0);

    c.ptr = L"a = 1000;";
    parse(&c);

    return 0;
}
