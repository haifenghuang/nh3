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

enum {
    OP_POP,
    OP_LITERAL,
    OP_SYMVAL,
    OP_ASSIGN,
    OP_LOCAL,
    OP_GLOBAL,
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
    OP_PRINT,
    OP_DUMP
} mpsl_op_t;


#define mpsl_is_true(v) mpdm_ival(v)

void rs_mpsl_reset_machine(mpdm_t machine)
{
    mpdm_t v;

    mpdm_hset_s(machine, L"stack",      MPDM_A(0));
    mpdm_hset_s(machine, L"c_stack",    MPDM_A(0));

    v = mpdm_hset_s(machine, L"symtbl",     MPDM_A(0));
    mpdm_push(v, MPDM_H(0));

    mpdm_hset_s(machine, L"pc",         MPDM_I(0));
    mpdm_hset_s(machine, L"sp",         MPDM_I(0));
    mpdm_hset_s(machine, L"tt",         MPDM_I(1));
}


#include <time.h>

static mpdm_t find_symtbl(mpdm_t s, mpdm_t symtbl, int tt)
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


mpdm_t rs_mpsl_get_symbol(mpdm_t s, mpdm_t symtbl, int tt)
{
    return mpdm_hget(find_symtbl(s, symtbl, tt), s);
}


mpdm_t rs_mpsl_set_symbol(mpdm_t s, mpdm_t v, mpdm_t symtbl, int tt)
{
    return mpdm_hset(find_symtbl(s, symtbl, tt), s, v);
}


#define PUSH(v) mpdm_aset(stack, v, sp++)
#define POP()   mpdm_aget(stack, --sp)

int rs_mpsl_exec(mpdm_t machine, int msecs)
{
    int ret = 0;
    mpdm_t prg      = mpdm_hget_s(machine, L"prg");
    mpdm_t stack    = mpdm_hget_s(machine, L"stack");
    mpdm_t c_stack  = mpdm_hget_s(machine, L"c_stack");
    mpdm_t symtbl   = mpdm_hget_s(machine, L"symtbl");
    int pc          = mpdm_ival(mpdm_hget_s(machine, L"pc"));
    int sp          = mpdm_ival(mpdm_hget_s(machine, L"sp"));
    int tt          = mpdm_ival(mpdm_hget_s(machine, L"tt"));
    clock_t max;
    mpdm_t v, w;
    double v1, v2, r;

    /* maximum running time */
    max = msecs ? (clock() + (msecs * CLOCKS_PER_SEC) / 1000) : 0x7fffffff;

    while (ret == 0 && pc < mpdm_size(prg)) {

        /* get the opcode */
        int opcode = mpdm_ival(mpdm_aget(prg, pc++));
    
        switch (opcode) {
        case OP_POP:
            /* discards the TOS */
            --sp;
            break;

        case OP_LITERAL:
            /* literal: next thing in pc is the literal */
            PUSH(mpdm_clone(mpdm_aget(prg, pc++)));
            break;

        case OP_SYMVAL:
            /* get symbol value */
            v = POP();
            PUSH(rs_mpsl_get_symbol(v, symtbl, tt));
            break;

        case OP_ASSIGN:
            /* assign a value to a symbol */
            v = POP();
            w = POP();
            PUSH(rs_mpsl_set_symbol(v, w, symtbl, tt));
            break;

        case OP_LOCAL:
            /* creates a local symbol */
            v = POP();
            mpdm_hset(mpdm_aget(symtbl, tt - 1), v, NULL);
            PUSH(v);
            break;
    
        case OP_GLOBAL:
            /* creates a global symbol */
            v = POP();
            mpdm_hset(mpdm_root(), v, NULL);
            PUSH(v);
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
            case OP_LT:     r = v1 < v2;  break;
            case OP_LE:     r = v1 <= v2; break;
            case OP_GT:     r = v1 > v2;  break;
            case OP_GE:     r = v1 >= v2; break;
            }
    
            PUSH(MPDM_I(r));
    
            break;
    
        case OP_PRINT:
            /* prints the value in the stack */
            mpdm_write_wcs(stdout, mpdm_string(POP()));
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

    mpdm_hset_s(machine, L"pc", MPDM_I(pc));
    mpdm_hset_s(machine, L"sp", MPDM_I(sp));
    mpdm_hset_s(machine, L"tt", MPDM_I(tt));

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


int main(int argc, char *argv[])
{
    mpdm_t v;
    mpdm_t machine, prg;
    int ret;
    int n;

    mpdm_startup();

    machine = mpdm_ref(MPDM_H(0));
    prg = mpdm_hset_s(machine, L"prg", MPDM_A(0));
    rs_mpsl_reset_machine(machine);

    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_I(1));
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_I(2));
    add_ins(prg, OP_ADD);
    add_ins(prg, OP_DUMP);

    rs_mpsl_exec(machine, 0);

    prg = mpdm_hset_s(machine, L"prg", MPDM_A(0));
    rs_mpsl_reset_machine(machine);
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

    rs_mpsl_exec(machine, 0);

    return 0;
}
