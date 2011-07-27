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

typedef enum {
    OP_LITERAL,
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
    OP_PC,
    OP_PATH,
    OP_RETURN,
    OP_IF,
    OP_IFELSE,
    OP_WHILE,
    OP_ASSIGN,
    OP_SYMVAL,
    OP_PRINT,
    OP_DUMP,
    OP_APUSH,
    OP_HSET
} mpsl_op_t;

#define mpsl_is_true(v) mpdm_ival(v)

void rs_mpsl_reset_machine(mpdm_t machine)
{
    mpdm_hset_s(machine, L"stack",      MPDM_A(0));
    mpdm_hset_s(machine, L"c_stack",    MPDM_A(0));
    mpdm_hset_s(machine, L"pc",         MPDM_I(0));
}


int rs_mpsl_exec1(mpdm_t prg, mpdm_t stack, mpdm_t c_stack, int *ppc)
{
    mpsl_op_t opcode;
    int pc;
    int ret = 1;

    pc = *ppc;

    /* get the opcode */
    opcode = mpdm_ival(mpdm_aget(prg, pc++));

    switch (opcode) {
    case OP_LITERAL:
        /* literal: next thing in pc is the literal */
        mpdm_push(stack, mpdm_clone(mpdm_aget(prg, pc++)));
        break;

    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
        {
            double v2   = mpdm_rval(mpdm_pop(stack));
            double v1   = mpdm_rval(mpdm_pop(stack));
            double r;

            switch (opcode) {
            case OP_ADD:    r = v1 + v2; break;
            case OP_SUB:    r = v1 - v2; break;
            case OP_MUL:    r = v1 * v2; break;
            case OP_DIV:    r = v1 / v2; break;
            }

            mpdm_push(stack, MPDM_R(r));
        }

        break;

    case OP_EQ:
    case OP_NEQ:
    case OP_LT:
    case OP_LE:
    case OP_GT:
    case OP_GE:
        {
            double v2   = mpdm_rval(mpdm_pop(stack));
            double v1   = mpdm_rval(mpdm_pop(stack));
            int r;

            switch (opcode) {
            case OP_EQ:     r = v1 == v2; break;
            case OP_NEQ:    r = v1 != v2; break;
            case OP_LT:     r = v1 < v2; break;
            case OP_LE:     r = v1 <= v2; break;
            case OP_GT:     r = v1 > v2; break;
            case OP_GE:     r = v1 >= v2; break;
            }

            mpdm_push(stack, MPDM_I(r));
        }

        break;

    case OP_PC:
        /* push the program counter */
        mpdm_push(stack, MPDM_I(pc));
        break;

    case OP_PATH:
        /* new code path */
        mpdm_push(stack, MPDM_I(pc));

        {
            /* move beyond the appropriate return */
            int l = 1;

            while (l && pc < mpdm_size(prg)) {
                opcode = mpdm_ival(mpdm_aget(prg, pc++));

                if (opcode == OP_PATH)
                    l++;
                if (opcode == OP_RETURN)
                    l--;
            }

            /* return not found? error */
            if (l)
                ret = -2;
        }

        break;

    case OP_RETURN:
        /* if there is a return address, use it;
           otherwise, trigger error */
        if (mpdm_size(c_stack))
            pc = mpdm_ival(mpdm_pop(c_stack));
        else
            ret = -1;

        break;

    case OP_IF:
        if (mpsl_is_true(mpdm_pop(stack))) {
            /* push return address and call */
            mpdm_push(c_stack, MPDM_I(pc));
            pc = mpdm_ival(mpdm_pop(stack));
        }
        else
            mpdm_adel(stack, -1);

        break;

    case OP_IFELSE:
        /* always push return address */
        mpdm_push(c_stack, MPDM_I(pc));

        if (mpsl_is_true(mpdm_pop(stack))) {
            /* drop false pc */
            mpdm_adel(stack, -1);

            /* jump to true pc */
            pc = mpdm_ival(mpdm_pop(stack));
        }
        else {
            /* jump to false pc */
            pc = mpdm_ival(mpdm_pop(stack));

            /* drop true pc */
            mpdm_adel(stack, -1);
        }

        break;

    case OP_WHILE:
        /* in the stack:
            <body pc> <condition pc> <condition>
        */
        if (mpdm_ival(mpdm_pop(stack))) {
            /* set the return address to the cond. pc */
            mpdm_push(c_stack, mpdm_aget(stack, -1));

            /* jump to the body */
            pc = mpdm_ival(mpdm_aget(stack, -2));
        }
        else {
            /* drop both pcs */
            mpdm_adel(stack, -1);
            mpdm_adel(stack, -1);
        }

        break;

    case OP_PRINT:
        /* prints the value in the stack */
        mpdm_write_wcs(stdout, mpdm_string(mpdm_pop(stack)));
        break;

    case OP_DUMP:
        {
            mpdm_t v = mpdm_pop(stack);
            mpdm_dump(v);
            mpdm_void(v);
        }
        break;

    case OP_ASSIGN:
        /* assign a value to a symbol */
        {
            mpdm_t s = mpdm_pop(stack);
            mpdm_t v = mpdm_pop(stack);
            mpdm_hset(mpdm_root(), s, v);
        }
        break;

    case OP_SYMVAL:
        /* get symbol value */
        mpdm_push(stack, mpdm_hget(mpdm_root(), mpdm_pop(stack)));
        break;

    case OP_APUSH:
        /* pushes a value into an array on the stack */
        {
            mpdm_t v = mpdm_pop(stack);
            mpdm_push(mpdm_aget(stack, -1), v);
        }
        break;

    case OP_HSET:
        /* sets a hash's key/value pair */
        {
            mpdm_t v = mpdm_pop(stack);
            mpdm_t k = mpdm_pop(stack);
            mpdm_hset(mpdm_aget(stack, -1), k, v);
        }
    }

    *ppc = pc;

    /* got to the end? trigger exit */
    if (pc == mpdm_size(prg))
        ret = 0;

    return ret;
}

#include <time.h>

int rs_mpsl_exec(mpdm_t machine, int msecs)
{
    int ret;
    mpdm_t prg      = mpdm_hget_s(machine, L"prg");
    mpdm_t stack    = mpdm_hget_s(machine, L"stack");
    mpdm_t c_stack  = mpdm_hget_s(machine, L"c_stack");
    int pc          = mpdm_ival(mpdm_hget_s(machine, L"pc"));
    clock_t max;

    /* maximum running time */
    max = msecs ? (clock() + (msecs * CLOCKS_PER_SEC) / 1000) : 0x7fffffff;

    while (
        pc < mpdm_size(prg) &&
        (ret = rs_mpsl_exec1(prg, stack, c_stack, &pc)) > 0 &&
        clock() < max
    );

    mpdm_hset_s(machine, L"pc", MPDM_I(pc));

    return ret;
}


static mpdm_t add_arg(mpdm_t prg, mpdm_t arg)
{
    return mpdm_push(prg, arg);
}

static mpdm_t add_ins(mpdm_t prg, mpsl_op_t opcode)
{
    return mpdm_push(prg, MPDM_I((int) opcode));
}


int main(int argc, char *argv[])
{
    mpdm_t v;
    mpdm_t machine, prg;
    int ret;

    mpdm_startup();

    machine = mpdm_ref(MPDM_H(0));
    prg = mpdm_hset_s(machine, L"prg", MPDM_A(0));
    rs_mpsl_reset_machine(machine);

    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_I(1));
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_I(2));
    add_ins(prg, OP_LT);
    add_ins(prg, OP_PRINT);
    add_ins(prg, OP_PATH);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"true\n"));
    add_ins(prg, OP_PRINT);
    add_ins(prg, OP_RETURN);
    add_ins(prg, OP_PATH);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"false\n"));
    add_ins(prg, OP_PRINT);
    add_ins(prg, OP_RETURN);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_I(1));
    add_ins(prg, OP_IFELSE);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_I(2));
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_I(3));
    add_ins(prg, OP_ADD);
    add_ins(prg, OP_PRINT);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"!!!\n"));
    add_ins(prg, OP_PRINT);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"content\n"));
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"VAR1"));
    add_ins(prg, OP_ASSIGN);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"VAR1"));
    add_ins(prg, OP_SYMVAL);
    add_ins(prg, OP_PRINT);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_I(3));
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_I(4));
    add_ins(prg, OP_ADD);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"VAR1"));
    add_ins(prg, OP_ASSIGN);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"VAR1"));
    add_ins(prg, OP_SYMVAL);
    add_ins(prg, OP_PRINT);

    /* VAR1 = VAR1 + 4 ; print VAR1 */
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"VAR1"));
    add_ins(prg, OP_SYMVAL);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_I(4));
    add_ins(prg, OP_ADD);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"VAR1"));
    add_ins(prg, OP_ASSIGN);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"VAR1"));
    add_ins(prg, OP_SYMVAL);
    add_ins(prg, OP_PRINT);

    rs_mpsl_exec(machine, 0);

    prg = mpdm_hset_s(machine, L"prg", MPDM_A(0));
    rs_mpsl_reset_machine(machine);

    /* VAR1 = 0 */
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_I(0));
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"VAR1"));
    add_ins(prg, OP_ASSIGN);

    /* { print VAR1; print "\n"; VAR1 = VAR1 + 1; } */
    add_ins(prg, OP_PATH);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"VAR1"));
    add_ins(prg, OP_SYMVAL);
    add_ins(prg, OP_PRINT);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"\n"));
    add_ins(prg, OP_PRINT);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"VAR1"));
    add_ins(prg, OP_SYMVAL);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_I(1));
    add_ins(prg, OP_ADD);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"VAR1"));
    add_ins(prg, OP_ASSIGN);
    add_ins(prg, OP_RETURN);

    /* | VAR1 < 10 while */
    add_ins(prg, OP_PC);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"VAR1"));
    add_ins(prg, OP_SYMVAL);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_I(10));
    add_ins(prg, OP_LT);
    add_ins(prg, OP_WHILE);

    printf("*\n");
    rs_mpsl_exec(machine, 1);
    printf("*\n");
    rs_mpsl_exec(machine, 0);

    prg = mpdm_hset_s(machine, L"prg", MPDM_A(0));
    rs_mpsl_reset_machine(machine);

    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_A(0));
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_I(1));
    add_ins(prg, OP_APUSH);
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_I(2));
    add_ins(prg, OP_APUSH);
    add_ins(prg, OP_DUMP);

    rs_mpsl_exec(machine, 0);

    prg = mpdm_hset_s(machine, L"prg", MPDM_A(0));
    rs_mpsl_reset_machine(machine);

    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_H(0));
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"font_face"));
    add_ins(prg, OP_LITERAL);
    add_arg(prg, MPDM_LS(L"Courier"));
    add_ins(prg, OP_HSET);
    add_ins(prg, OP_DUMP);

    rs_mpsl_exec(machine, 0);

    return 0;
}
