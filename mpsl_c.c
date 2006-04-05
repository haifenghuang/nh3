/*

    MPSL - Minimum Profit Scripting Language
    Copyright (C) 2003/2006 Angel Ortega <angel@triptico.com>

    mpsl_c.c - Minimum Profit Scripting Language Core

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

    http://www.triptico.com

*/

#include "config.h"

#include <stdio.h>
#include <wchar.h>
#include "mpdm.h"
#include "mpsl.h"

/*******************
	Data
********************/

/* array containing the opcodes */
mpdm_t mpsl_ops = NULL;

/* local symbol table */
mpdm_t mpsl_local = NULL;

/* instruction execution tracing flag */
int mpsl_trace = 0;

/* global abort flag */
int mpsl_abort = 0;

/*******************
	Code
********************/

/**
 * mpsl_is_true - Tests if a value is true.
 * @v: the value
 *
 * If @v is a valid MPSL 'false' value (NULL, "" or the "0" string),
 * returns zero, or nonzero otherwise.
 */
int mpsl_is_true(mpdm_t v)
{
	/* if value or its data is NULL, it's false */
	if(v == NULL || v->data == NULL)
		return(0);

	/* if it's a printable string... */
	if(v->flags & MPDM_STRING)
	{
		wchar_t * ptr = (wchar_t *)v->data;

		/* ... and it's "" or the "0" string, it's false */
		if(*ptr == L'\0' || (*ptr == L'0' && *(ptr + 1) == L'\0'))
			return(0);
	}

	/* any other case is true */
	return(1);
}


/**
 * mpsl_boolean - Returns 'true' or 'false' MPSL stock values.
 * @b: boolean selector
 *
 * Returns MPSL's 'false' or 'true' values depending on the value in @b.
 */
mpdm_t mpsl_boolean(int b)
{
	mpdm_t v = NULL;

	if(b)
	{
		if((v = mpdm_hget_s(mpdm_root(), L"TRUE")) == NULL)
		{
			v = MPDM_I(1);
			mpdm_hset_s(mpdm_root(), L"TRUE", v);
		}
	}

	return(v);
}


/**
 * mpsl_local_add_blkframe - Creates a block frame.
 *
 * Creates a block frame in the local variable symbol table.
 */
mpdm_t mpsl_local_add_blkframe(void)
{
	/* pushes a new hash onto the last subframe */
	return(mpdm_push(mpdm_aget(mpsl_local, -1), MPDM_H(0)));
}


/**
 * mpsl_local_del_blkframe - Deletes a block frame.
 *
 * Deletes a block frame previously created by
 * mpsl_local_add_blkframe().
 */
void mpsl_local_del_blkframe(void)
{
	/* simply pops the blkframe */
	mpdm_pop(mpdm_aget(mpsl_local, -1));
}


/**
 * mpsl_local_add_subframe - Creates a subroutine frame.
 *
 * Creates a subroutine frame in the local variable symbol table.
 */
void mpsl_local_add_subframe(void)
{
	/* if local symbol table doesn't exist, create */
	if(mpsl_local == NULL)
		mpsl_local = mpdm_ref(MPDM_A(0));

	/* creates a new array for holding the hashes */
	mpdm_push(mpsl_local, MPDM_A(0));

	mpsl_local_add_blkframe();
}


/**
 * mpsl_local_del_subframe - Deletes a subroutine frame.
 *
 * Deletes a subroutine frame previously created by
 * mpsl_local_add_subframe().
 */
void mpsl_local_del_subframe(void)
{
	mpsl_local_del_blkframe();

	/* simply pops the subframe */
	mpdm_pop(mpsl_local);
}


static mpdm_t mpsl_local_find_symbol(mpdm_t s)
/* finds a symbol in the local symbol table */
{
	int n;
	mpdm_t l;
	mpdm_t v = NULL;

	/* if s is multiple, take just the first element */
	if(MPDM_IS_ARRAY(s))
		s = mpdm_aget(s, 0);

	l = mpdm_aget(mpsl_local, -1);

	/* travel the local symbol table trying to find it */
	for(n = mpdm_size(l) - 1;n >= 0;n--)
	{
		mpdm_t h = mpdm_aget(l, n);

		if(mpdm_exists(h, s))
		{
			v = h;
			break;
		}
	}

	return(v);
}


static void mpsl_local_set_symbols(mpdm_t s, mpdm_t v)
/* sets (or creates) a list of local symbols with a list of values */
{
	mpdm_t l;

	if(s == NULL)
		return;

	/* gets the top local variable frame */
	l = mpdm_aget(mpdm_aget(mpsl_local, -1), -1);

	if(MPDM_IS_ARRAY(s))
	{
		int n;

		for(n = 0;n < mpdm_size(s);n++)
			mpdm_hset(l, mpdm_aget(s, n), mpdm_aget(v, n));
	}
	else
		mpdm_hset(l, s, v);
}


/**
 * mpsl_set_symbol - Sets value to a symbol.
 * @s: symbol name
 * @v: value
 *
 * Assigns the value @v to the @s symbol. If the value exists as
 * a local symbol, it's assigned to it; otherwise, it's set as a global
 * symbol (and created if it does not exist).
 */
mpdm_t mpsl_set_symbol(mpdm_t s, mpdm_t v)
{
	mpdm_sset(mpsl_local_find_symbol(s), s, v);
	return(v);
}


/**
 * mpsl_get_symbol - Gets the value of a symbol.
 * @s: symbol name
 *
 * Gets the value of a symbol. The symbol can be local or global
 * (if the symbol exists in both tables, the local value will be returned).
 */
mpdm_t mpsl_get_symbol(mpdm_t s)
{
	return(mpdm_sget(mpsl_local_find_symbol(s), s));
}


/** opcodes **/

#define O_TYPE static mpdm_t
#define O_ARGS mpdm_t c, mpdm_t a, int * f

O_TYPE mpsl_exec_i(O_ARGS);

#define C(n) mpdm_aget(c, n)
#define C0 C(0)
#define C1 C(1)

#define M(n) mpsl_exec_i(C(n), a, f)
#define M1 M(1)
#define M2 M(2)
#define M3 M(3)

#define R(x) mpdm_rval(x)
#define I(x) mpdm_ival(x)

#define RM1 mpdm_rval(M(1))
#define RM2 mpdm_rval(M(2))
#define IM1 mpdm_ival(M(1))
#define IM2 mpdm_ival(M(2))

#define GET mpsl_get_symbol
#define SET mpsl_set_symbol
#define BOOL mpsl_boolean
#define ISTRU mpsl_is_true

#define RF(v) mpdm_ref(v)
#define UF(v) mpdm_unref(v)

O_TYPE O_multi(O_ARGS) { mpdm_t v = RF(M1); if(!*f) v = M2; else UF(v); return(v); }
O_TYPE O_literal(O_ARGS) { return(mpdm_clone(C1)); }
O_TYPE O_symval(O_ARGS) { return(GET(M1)); }
O_TYPE O_assign(O_ARGS) { mpdm_t v = RF(M1); mpdm_t r = SET(v, M2); UF(v); return(r); }
O_TYPE O_if(O_ARGS) { return(ISTRU(M1) ? M2 : M3); }
O_TYPE O_while(O_ARGS) { while(! *f && ISTRU(M1)) M2; if(*f == 1) *f = 0; return(NULL); }
O_TYPE O_local(O_ARGS) { mpsl_local_set_symbols(M1, NULL); return(NULL); }
O_TYPE O_uminus(O_ARGS) { return(MPDM_R(-RM1)); }
O_TYPE O_add(O_ARGS) { return(MPDM_R(RM1 + RM2)); }
O_TYPE O_sub(O_ARGS) { return(MPDM_R(RM1 - RM2)); }
O_TYPE O_mul(O_ARGS) { return(MPDM_R(RM1 * RM2)); }
O_TYPE O_div(O_ARGS) { return(MPDM_R(RM1 / RM2)); }
O_TYPE O_mod(O_ARGS) { return(MPDM_I(IM1 % IM2)); }
O_TYPE O_not(O_ARGS) { return(BOOL(! ISTRU(M1))); }
O_TYPE O_and(O_ARGS) { mpdm_t r = M1; return(ISTRU(r) ? M2 : r); }
O_TYPE O_or(O_ARGS) { mpdm_t r = M1; return(ISTRU(r) ? r : M2); }
O_TYPE O_numlt(O_ARGS) { return(BOOL(RM1 < RM2)); }
O_TYPE O_numle(O_ARGS) { return(BOOL(RM1 <= RM2)); }
O_TYPE O_numgt(O_ARGS) { return(BOOL(RM1 > RM2)); }
O_TYPE O_numge(O_ARGS) { return(BOOL(RM1 >= RM2)); }
O_TYPE O_strcat(O_ARGS) { mpdm_t v = RF(M1); mpdm_t r = mpdm_strcat(v, M2); UF(v); return(r); }
O_TYPE O_streq(O_ARGS) { mpdm_t v = RF(M1); mpdm_t r = BOOL(mpdm_cmp(v, M2) == 0); UF(v); return(r); }
O_TYPE O_immpinc(O_ARGS) { mpdm_t s=M1; return(SET(s, MPDM_R(R(GET(s)) + 1))); }
O_TYPE O_immpdec(O_ARGS) { mpdm_t s=M1; return(SET(s, MPDM_R(R(GET(s)) - 1))); }
O_TYPE O_immadd(O_ARGS) { mpdm_t s = RF(M1); mpdm_t r = SET(s, MPDM_R(R(GET(s)) + RM2)); UF(s); return(r); }
O_TYPE O_immsub(O_ARGS) { mpdm_t s = RF(M1); mpdm_t r = SET(s, MPDM_R(R(GET(s)) - RM2)); UF(s); return(r); }
O_TYPE O_immmul(O_ARGS) { mpdm_t s = RF(M1); mpdm_t r = SET(s, MPDM_R(R(GET(s)) * RM2)); UF(s); return(r); }
O_TYPE O_immdiv(O_ARGS) { mpdm_t s = RF(M1); mpdm_t r = SET(s, MPDM_R(R(GET(s)) / RM2)); UF(s); return(r); }
O_TYPE O_immmod(O_ARGS) { mpdm_t s = RF(M1); mpdm_t r = SET(s, MPDM_R(I(GET(s)) % IM2)); UF(s); return(r); }
O_TYPE O_immsinc(O_ARGS) { mpdm_t s = M1; mpdm_t v = GET(s); SET(s, MPDM_R(R(v) + 1)); return(v); }
O_TYPE O_immsdec(O_ARGS) { mpdm_t s = M1; mpdm_t v = GET(s); SET(s, MPDM_R(R(v) - 1)); return(v); }
O_TYPE O_numeq(O_ARGS) { mpdm_t v1 = RF(M1); mpdm_t v2 = M2; UF(v1); return(BOOL((v1 == NULL || v2 == NULL) ? (v1 == v2) : (R(v1) == R(v2)))); }
O_TYPE O_break(O_ARGS) { *f = 1; return(NULL); }
O_TYPE O_return(O_ARGS) { mpdm_t v = M1; *f = -1; return(v); }

O_TYPE O_exec(O_ARGS)
/* executes the value of a symbol */
{
	mpdm_t s, v, r;

	/* gets the symbol name */
	s = RF(M1);

	/* gets the symbol value */
	if ((v = GET(s)) == NULL) {
		/* not found or NULL value? warn */
		/* FIXME: This is a hack */
		mpdm_t t;

		t = mpdm_join(MPDM_LS(L"."), s);
		t = MPDM_2MBS((wchar_t *) t->data);
		fprintf(stderr, "WARNING: Undefined function %s()\n",
			(char *)t->data);
	}

	/* executes */
	r = mpdm_exec(v, M2);

	UF(s);

	return(r);
}

O_TYPE O_foreach(O_ARGS)
/* foreach loop */
{
	mpdm_t s = RF(M1);
	mpdm_t v = RF(M2);
	int n;

	for(n = 0;n < mpdm_size(v) && ! *f;n++)
	{
		SET(s, mpdm_aget(v, n));
		M3;
	}

	if(*f == 1) *f = 0;

	UF(s); UF(v);

	return(NULL);
}


O_TYPE O_range(O_ARGS)
/* build list from range of two numeric values */
{
	double n;
	double v1 = RM1;
	double v2 = RM2;
	mpdm_t ret = MPDM_A(0);

	if(v1 < v2)
		for(n = v1;n <= v2;n++)
			mpdm_push(ret, MPDM_R(n));
	else
		for(n = v1;n >= v2;n--)
			mpdm_push(ret, MPDM_R(n));

	return(ret);
}


O_TYPE O_list(O_ARGS)
/* build list from instructions */
{
	int n;
	mpdm_t ret = RF(MPDM_A(mpdm_size(c) - 1));

	for(n = 1;n < mpdm_size(c);n++)
		mpdm_aset(ret, M(n), n - 1);

	return(UF(ret));
}


O_TYPE O_hash(O_ARGS)
/* build hash from instructions */
{
	int n;
	mpdm_t ret = RF(MPDM_H(0));

	for(n = 1;n < mpdm_size(c);n += 2)
	{
		mpdm_t v = RF(M(n));

		mpdm_hset(ret, v, M(n + 1));

		UF(v);
	}

	return(UF(ret));
}


O_TYPE O_subframe(O_ARGS)
/* runs an instruction inside a subroutine frame */
{
	mpdm_t ret;

	/* creates a subroutine frame */
	mpsl_local_add_subframe();

	/* creates the arguments (if any) as local variables */
	mpsl_local_set_symbols(M2, a);

	/* execute instruction */
	ret = M1;

	/* destroys the frame */
	mpsl_local_del_subframe();

	return(ret);
}


O_TYPE O_blkframe(O_ARGS)
/* runs an instruction under a block frame */
{
	mpdm_t ret;

	mpsl_local_add_blkframe();
	ret = M1;
	mpsl_local_del_blkframe();

	return(ret);
}


static struct mpsl_op_s
{
	wchar_t * name;
	mpdm_t (* func)(O_ARGS);
} op_table[]=
{
	{ L"MULTI",	O_multi },
	{ L"LITERAL",	O_literal },
	{ L"SYMVAL",	O_symval },
	{ L"ASSIGN",	O_assign },
	{ L"EXEC",	O_exec },
	{ L"IF",	O_if },
	{ L"WHILE",	O_while },
	{ L"FOREACH",	O_foreach },
	{ L"SUBFRAME",	O_subframe },
	{ L"BLKFRAME",	O_blkframe },
	{ L"BREAK",	O_break },
	{ L"RETURN",	O_return },
	{ L"LOCAL",	O_local },
	{ L"LIST",	O_list },
	{ L"HASH",	O_hash },
	{ L"RANGE",	O_range },
	{ L"UMINUS",	O_uminus },
	{ L"ADD",	O_add },
	{ L"SUB",	O_sub },
	{ L"MUL",	O_mul },
	{ L"DIV",	O_div },
	{ L"MOD",	O_mod },
	{ L"SINC",	O_immsinc },
	{ L"SDEC",	O_immsdec },
	{ L"PINC",	O_immpinc },
	{ L"PDEC",	O_immpdec },
	{ L"IADD",	O_immadd },
	{ L"ISUB",	O_immsub },
	{ L"IMUL",	O_immmul },
	{ L"IDIV",	O_immdiv },
	{ L"IMOD",	O_immmod },
	{ L"NOT",	O_not },
	{ L"AND",	O_and },
	{ L"OR",	O_or },
	{ L"NUMEQ",	O_numeq },
	{ L"NUMLT",	O_numlt },
	{ L"NUMLE",	O_numle },
	{ L"NUMGT",	O_numgt },
	{ L"NUMGE",	O_numge },
	{ L"STRCAT",	O_strcat },
	{ L"STREQ",	O_streq },
	{ NULL,		NULL }
};


mpdm_t mpsl_op(wchar_t * opcode)
{
	if(mpsl_ops == NULL)
	{
		int n;

		/* first time; load opcodes */
		mpsl_ops = mpdm_ref(MPDM_H(0));

		for(n = 0;op_table[n].name != NULL;n++)
		{
			mpdm_t v = MPDM_LS(op_table[n].name);
			v->ival = n;
			v->flags |= MPDM_IVAL;

			/* keys and values are the same */
			mpdm_hset(mpsl_ops, v, v);
		}
	}

	return(mpdm_hget_s(mpsl_ops, opcode));
}


O_TYPE mpsl_exec_i(O_ARGS)
/* Executes one MPSL instruction in the MPSL virtual machine. Called
   from mpsl_exec() (which holds the flow control status variable) */
{
	mpdm_t ret = NULL;

	/* if aborted, do nothing */
	if(mpsl_abort) return(NULL);

	/* reference both code and arguments */
	mpdm_ref(c); mpdm_ref(a);

	if(c != NULL)
	{
		int op;

		/* gets the opcode */
		op = mpdm_ival(C0);

		/* if it's a valid opcode... */
		if(op >= 0 && op < sizeof(op_table) / sizeof(struct mpsl_op_s))
		{
			struct mpsl_op_s * o = &op_table[op];

			/* and call it if existent */
			if(o->func != NULL)
				ret = mpdm_ref(o->func(c, a, f));
		}
	}

	if(mpsl_trace)
		printf("** %ls: %ls\n", mpdm_string(C0), mpdm_string(ret));

	/* unreference */
	mpdm_unref(a); mpdm_unref(c);

	/* sweep some values */
	mpdm_sweep(0);

	return(mpdm_unref(ret));
}


mpdm_t mpsl_exec(mpdm_t c, mpdm_t a)
/* executes an MPSL instruction stream */
{
	int f = 0;
	mpdm_t v;

	/* execute first instruction */
	v = mpsl_exec_i(c, a, &f);

	return(v);
}
