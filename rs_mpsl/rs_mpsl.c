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


/** compiler **/

enum {
    EOP, ERROR,

    IF, ELSE, WHILE, BREAK,
    LOCAL, GLOBAL, SUB, RETURN, NULLT,

    LBRACE, RBRACE, LPAREN, RPAREN, LBRACK, RBRACK,
    COLON, SEMI, EQUAL, DOT,

    SYMBOL, LITERAL
};

/* should match token enum */
static wchar_t *tokens_c = L"{}()[]:;=.";

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

struct mpsl_lp {
    int token;          /* token found */
    struct ds token_s;  /* token as string */
    mpdm_t prg;         /* generated program */
    int x;              /* x source position */
    int y;              /* y source position */
    wchar_t c;          /* last char read from input */
    wchar_t *ptr;       /* program source */
    FILE *f;            /* program file */
    int error;          /* non-zero if syntax error */
};


static void next_c(struct mpsl_lp *l)
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


static int tok_eop(struct mpsl_lp *l)
{
    if (l->c == L'\0' || l->c == WEOF) {
        l->token = EOP;
        return 0;
    }

    return 1;
}


static int tok_1c(struct mpsl_lp *l)
{
    wchar_t *ptr;

    if ((ptr = wcschr(tokens_c, l->c)) != NULL) {
        next_c(l);
        l->token = (ptr - tokens_c) + LBRACE;
        return 0;
    }

    return 1;
}


static int tok_str(struct mpsl_lp *l)
{
    if (l->c == L'"') {
    }

    return 1;
}


static int tok_vstr(struct mpsl_lp *l)
{
    if (l->c == L'\'') {
        next_c(l);
        STORE(l->c != L'\'');
        l->token = LITERAL;

        return 0;
    }

    return 1;
}


static int tok_sym(struct mpsl_lp *l)
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
            l->token = SYMBOL;
        else
            l->token = n + IF;

        return 0;
    }

    return 1;
}

static int tok_specialnum(struct mpsl_lp *l)
{
    if (l->c == L'0') {
    }

    return 1;
}


static int tok_num(struct mpsl_lp *l)
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

        l->token = LITERAL;
        return 0;
    }

    return 1;
}


static int token(struct mpsl_lp *l)
{
    ds_rewind(l->token_s);

    if (tok_eop(l) && tok_1c(l) && tok_str(l) && tok_vstr(l) &&
        tok_sym(l) && tok_specialnum(l) && tok_num(l)) {
        l->error = 1;
        l->token = ERROR;
    }

    return l->token;
}


/** basic MPSL runtime **/

#define mpsl_is_true(v) mpdm_ival(v)

/** virtual machine **/

enum {
    OP_EOP,
    OP_LITERAL, OP_NULL, OP_POP,
    OP_SYMVAL, OP_ASSIGN,
    OP_LOCAL, OP_GLOBAL,
    OP_JMP, OP_JT, OP_JF,
    OP_TPUSH, OP_TPOP,
    OP_EXECSYM, OP_RETURN,
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


static void PUSH(struct mpsl_vm *m, mpdm_t v)
{
    mpdm_aset(m->stack, v, m->sp++);
}


static mpdm_t POP(struct mpsl_vm *m)
{
    return mpdm_aget(m->stack, --m->sp);
}

#include <time.h>

int mpsl_exec_vm(struct mpsl_vm *m, int msecs)
{
    clock_t max;
    mpdm_t v;
    double v1, v2, r;

    /* maximum running time */
    max = msecs ? (clock() + (msecs * CLOCKS_PER_SEC) / 1000) : 0x7fffffff;

    /* start running if there is no error */
    if (m->mode != VM_ERROR)
        m->mode = VM_RUNNING;

    while (m->mode == VM_RUNNING) {

        /* get the opcode */
        int opcode = mpdm_ival(mpdm_aget(m->prg, m->pc++));
    
        switch (opcode) {
        case OP_EOP:
            m->mode = VM_IDLE;
            break;

        case OP_LITERAL:
            /* literal: next thing in pc is the literal */
            PUSH(m, mpdm_clone(mpdm_aget(m->prg, m->pc++)));
            break;

        case OP_NULL:
            /* pushes a NULL value */
            PUSH(m, NULL);
            break;

        case OP_POP:
            /* discards the TOS */
            --m->sp;
            break;

        case OP_SYMVAL:
            /* get symbol value */
            PUSH(m, mpsl_get_symbol(m, POP(m)));
            break;

        case OP_ASSIGN:
            /* assign a value to a symbol */
            PUSH(m, mpsl_set_symbol(m, POP(m), POP(m)));
            break;

        case OP_LOCAL:
            /* creates a local symbol */
            v = POP(m);
            mpdm_hset(mpdm_aget(m->symtbl, m->tt - 1), v, NULL);
            PUSH(m, v);
            break;
    
        case OP_GLOBAL:
            /* creates a global symbol */
            v = POP(m);
            mpdm_hset(mpdm_root(), v, NULL);
            PUSH(m, v);
            break;

        case OP_JMP:
            /* non-conditional jump */
            m->pc = mpdm_ival(mpdm_aget(m->prg, m->pc));
            break;

        case OP_JT:
            /* jump if true */
            if (mpsl_is_true(POP(m)))
                m->pc = mpdm_ival(mpdm_aget(m->prg, m->pc));
            else
                m->pc++;
            break;

        case OP_JF:
            /* jump if false */
            if (!mpsl_is_true(POP(m)))
                m->pc = mpdm_ival(mpdm_aget(m->prg, m->pc));
            else
                m->pc++;
            break;

        case OP_TPUSH:
            /* pushes the TOS as a new symtbl */
            mpdm_aset(m->symtbl, POP(m), m->tt++);
            break;

        case OP_TPOP:
            /* discards the last symtbl */
            --m->tt;
            break;

        case OP_EXECSYM:
            /* calls a subroutine */
            /* args...? */
            mpdm_aset(m->c_stack, MPDM_I(m->pc), m->cs++);
            break;

        case OP_RETURN:
            /* returns from subroutine */
            m->pc = mpdm_ival(mpdm_aget(m->c_stack, --m->cs));
            break;

        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
            v2 = mpdm_rval(POP(m));
            v1 = mpdm_rval(POP(m));
    
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
            v2 = mpdm_rval(POP(m));
            v1 = mpdm_rval(POP(m));
    
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
    struct mpsl_lp lp;
    struct mpsl_vm m;

    mpdm_startup();

    memset(&m, '\0', sizeof(m));
    memset(&lp, '\0', sizeof(lp));

    lp.ptr = L"a = 1000; b = NULL; while (c) { d; e; }";
    next_c(&lp);
    while (token(&lp) != EOP && lp.token != ERROR)
        printf("%d\n", lp.token);
    printf("%d.\n", lp.token);

    prg = MPDM_A(0);
    mpsl_reset_vm(&m, prg);

    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_I(1));
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_I(2));
    add_ins(prg, OP_ADD);
    add_ins(prg, OP_DUMP);

    mpsl_exec_vm(&m, 0);

    prg = MPDM_A(0);
    mpsl_reset_vm(&m, prg);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"MPDM"));
    add_ins(prg, OP_SYMVAL);
    add_ins(prg, OP_DUMP);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"test"));
    add_ins(prg, OP_LOCAL);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_I(666));
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"test"));
    add_ins(prg, OP_ASSIGN);

    mpsl_exec_vm(&m, 0);

    return 0;
}
