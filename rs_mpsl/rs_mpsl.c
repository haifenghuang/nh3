/*

    Reverse Stack MPSL

    Angel Ortega <angel@triptico.com>

    This is an experiment of a reverse-stack MPSL.

    Also, the executor includes a maximum number of
    milliseconds before yielding, with the capability
    of restarting where it left in the subsequent call.

*/

#include <stdio.h>
#include <mpdm.h>


/** basic MPSL runtime **/

#define mpsl_is_true(v) mpdm_ival(v)

static mpdm_t find_symtbl(mpdm_t s, mpdm_t symtbl, int tt)
/* finds the local symbol table that stores s */
{
    int n;
    mpdm_t l = NULL;

    for (n = tt - 1; n >= 0; n--) {
        if ((l = mpdm_aget(symtbl, n)) == NULL)
            break;

        if (mpdm_exists(l, s))
            break;
    }

    if (l == NULL || n < 0)
        l = mpdm_root();

    return l;
}


mpdm_t mpsl_get_symbol(mpdm_t s, mpdm_t symtbl, int tt)
{
    mpdm_t r;

    mpdm_ref(s);
    r = mpdm_hget(find_symtbl(s, symtbl, tt), s);
    mpdm_unref(s);

    return r;
}


mpdm_t mpsl_set_symbol(mpdm_t s, mpdm_t v, mpdm_t symtbl, int tt)
{
    mpdm_t r;

    mpdm_ref(s);
    r = mpdm_hset(find_symtbl(s, symtbl, tt), s, v);
    mpdm_unref(s);

    return r;
}



/** virtual machine **/

enum {
    OP_POP,
    OP_LITERAL,
    OP_SYMVAL,
    OP_ASSIGN,
    OP_LOCAL,
    OP_GLOBAL,
    OP_JMP,
    OP_JT,
    OP_JF,
    OP_TPUSH,
    OP_TPOP,
    OP_EXECSYM,
    OP_RETURN,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,
    OP_DUMP
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
};


void mpsl_reset_vm(struct mpsl_vm *m, mpdm_t prg)
{
    if (prg) {
        mpdm_unref(m->prg);
        m->prg = mpdm_ref(prg);
    }

    mpdm_unref(m->stack);
    m->stack = mpdm_ref(MPDM_A(0));
    mpdm_unref(m->c_stack);
    m->c_stack = mpdm_ref(MPDM_A(0));
    mpdm_unref(m->symtbl);
    m->symtbl = mpdm_ref(MPDM_A(0));
    mpdm_push(m->symtbl, MPDM_H(0));

    m->pc = m->sp = m->cs = m->tt = 0;
}


#include <time.h>

#define PUSH(v) mpdm_aset(m->stack, v, m->sp++)
#define POP()   mpdm_aget(m->stack, --m->sp)

int mpsl_exec_vm(struct mpsl_vm *m, int msecs)
{
    int ret = 0;
    clock_t max;
    mpdm_t v, w;
    double v1, v2, r;

    /* maximum running time */
    max = msecs ? (clock() + (msecs * CLOCKS_PER_SEC) / 1000) : 0x7fffffff;

    while (ret == 0 && m->pc < mpdm_size(m->prg)) {

        /* get the opcode */
        int opcode = mpdm_ival(mpdm_aget(m->prg, m->pc++));
    
        switch (opcode) {
        case OP_POP:
            /* discards the TOS */
            --m->sp;
            break;

        case OP_LITERAL:
            /* literal: next thing in pc is the literal */
            PUSH(mpdm_clone(mpdm_aget(m->prg, m->pc++)));
            break;

        case OP_SYMVAL:
            /* get symbol value */
            v = POP();
            PUSH(mpsl_get_symbol(v, m->symtbl, m->tt));
            break;

        case OP_ASSIGN:
            /* assign a value to a symbol */
            v = POP();
            w = POP();
            PUSH(mpsl_set_symbol(v, w, m->symtbl, m->tt));
            break;

        case OP_LOCAL:
            /* creates a local symbol */
            v = POP();
            mpdm_hset(mpdm_aget(m->symtbl, m->tt - 1), v, NULL);
            PUSH(v);
            break;
    
        case OP_GLOBAL:
            /* creates a global symbol */
            v = POP();
            mpdm_hset(mpdm_root(), v, NULL);
            PUSH(v);
            break;

        case OP_JMP:
            /* non-conditional jump */
            m->pc = mpdm_ival(mpdm_aget(m->prg, m->pc));
            break;

        case OP_JT:
            /* jump if true */
            if (mpsl_is_true(POP()))
                m->pc = mpdm_ival(mpdm_aget(m->prg, m->pc));
            else
                m->pc++;
            break;

        case OP_JF:
            /* jump if false */
            if (!mpsl_is_true(POP()))
                m->pc = mpdm_ival(mpdm_aget(m->prg, m->pc));
            else
                m->pc++;
            break;

        case OP_TPUSH:
            /* pushes the TOS as a new symtbl */
            mpdm_aset(m->symtbl, POP(), m->tt++);
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
            v2 = mpdm_rval(POP());
            v1 = mpdm_rval(POP());
    
            switch (opcode) {
            case OP_ADD:    r = v1 + v2; break;
            case OP_SUB:    r = v1 - v2; break;
            case OP_MUL:    r = v1 * v2; break;
            case OP_DIV:    r = v1 / v2; break;
            }
    
            PUSH(MPDM_R(r));
    
            break;
    
        case OP_EQ:
        case OP_NEQ:
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:
            v2 = mpdm_rval(POP());
            v1 = mpdm_rval(POP());
    
            switch (opcode) {
            case OP_EQ:     r = v1 == v2; break;
            case OP_NEQ:    r = v1 != v2; break;
            case OP_LT:     r = v1 <  v2; break;
            case OP_LE:     r = v1 <= v2; break;
            case OP_GT:     r = v1 >  v2; break;
            case OP_GE:     r = v1 >= v2; break;
            }
    
            PUSH(MPDM_I(r));
    
            break;
    
        case OP_DUMP:
            v = POP();
            mpdm_dump(v);
            mpdm_void(v);

            break;
        }

        /* if out of slice time, break */        
        if (clock() > max)
            ret = 1;
    }

    return ret;
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
    mpdm_t v;
    mpdm_t prg;
    int ret;
    int n;
    struct mpsl_vm m;

    mpdm_startup();

    memset(&m, '\0', sizeof(m));

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
