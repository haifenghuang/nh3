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
        case OP_LITERAL:
            /* literal: next thing in pc is the literal */
            PUSH(m, mpdm_clone(mpdm_aget(m->prg, m->pc++)));
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
        case OP_NEQ:
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:
            v2 = mpdm_rval(POP(m));
            v1 = mpdm_rval(POP(m));
    
            switch (opcode) {
            case OP_EQ:     r = v1 == v2; break;
            case OP_NEQ:    r = v1 != v2; break;
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
