/*

    mpdm - Minimum Profit Data Manager
    Copyright (C) 2003/2005 Angel Ortega <angel@triptico.com>

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
mpdm_v _mpsl_ops=NULL;

/* local symbol table */
mpdm_v _mpsl_local=NULL;

/* instruction execution tracing flag */
int _mpsl_trace=0;

/*******************
	Code
********************/

/**
 * mpsl_is_true - Tests if a value is true
 * @v: the value
 *
 * If @v is a valid mpsl 'false' value (NULL, "" or the "0" string),
 * returns zero, or nonzero otherwise.
 */
int mpsl_is_true(mpdm_v v)
{
	/* if value or its data is NULL, it's false */
	if(v == NULL || v->data == NULL)
		return(0);

	/* if it's a printable string... */
	if(v->flags & MPDM_STRING)
	{
		wchar_t * ptr=(wchar_t *)v->data;

		/* ... and it's "" or the "0" string, it's false */
		if(*ptr == L'\0' || (*ptr == L'0' && *(ptr + 1) == L'\0'))
			return(0);
	}

	/* any other case is true */
	return(1);
}


/**
 * mpsl_boolean - Returns mpsl values true or false
 * @b: boolean selector
 *
 * Returns mpsl's 'false' or 'true' values depending on the value in @b.
 */
mpdm_v mpsl_boolean(int b)
{
	static mpdm_v _true=NULL;

	if(_true == NULL)
		_true=mpdm_ref(MPDM_I(1));

	return(b ? _true : NULL);
}


/**
 * mpsl_local_add_blkframe - Creates a block frame
 *
 * Creates a block frame in the local variable symbol table.
 */
mpdm_v mpsl_local_add_blkframe(void)
{
	/* pushes a new hash onto the last subframe */
	return(mpdm_apush(mpdm_aget(_mpsl_local, -1), MPDM_H(0)));
}


/**
 * mpsl_local_del_blkframe - Deletes a block frame
 *
 * Deletes a block frame previously created by
 * mpsl_local_add_blkframe().
 */
void mpsl_local_del_blkframe(void)
{
	/* simply pops the blkframe */
	mpdm_apop(mpdm_aget(_mpsl_local, -1));
}


/**
 * mpsl_local_add_subframe - Creates a subroutine frame
 *
 * Creates a subroutine frame in the local variable symbol table.
 */
void mpsl_local_add_subframe(void)
{
	/* if local symbol table doesn't exist, create */
	if(_mpsl_local == NULL)
		_mpsl_local=mpdm_ref(MPDM_A(0));

	/* creates a new array for holding the hashes */
	mpdm_apush(_mpsl_local, MPDM_A(0));

	mpsl_local_add_blkframe();
}


/**
 * mpsl_local_del_subframe - Deletes a subroutine frame
 *
 * Deletes a subroutine frame previously created by
 * mpsl_local_add_subframe().
 */
void mpsl_local_del_subframe(void)
{
	mpsl_local_del_blkframe();

	/* simply pops the subframe */
	mpdm_apop(_mpsl_local);
}


mpdm_v mpsl_local_find_symbol(mpdm_v s)
{
	int n;
	mpdm_v l;
	mpdm_v v = NULL;

	/* if s is multiple, take just the first element */
	if(s->flags & MPDM_MULTIPLE)
		s=mpdm_aget(s, 0);

	l=mpdm_aget(_mpsl_local, -1);

	/* travel the local symbol table trying to find it */
	for(n=mpdm_size(l) - 1;n >=0;n--)
	{
		mpdm_v h = mpdm_aget(l, n);

		if(mpdm_hexists(h, s))
		{
			v=h;
			break;
		}
	}

	return(v);
}


void mpsl_local_set_symbols(mpdm_v s, mpdm_v v)
{
	mpdm_v l;

	if(s == NULL)
		return;

	/* gets the top local variable frame */
	l=mpdm_aget(mpdm_aget(_mpsl_local, -1), -1);

	if(s->flags & MPDM_MULTIPLE)
	{
		int n;

		for(n=0;n < mpdm_size(s);n++)
			mpdm_hset(l, mpdm_aget(s, n), mpdm_aget(v, n));
	}
	else
		mpdm_hset(l, s, v);
}


/**
 * mpsl_set_symbol - Sets value to a symbol
 * @s: symbol name
 * @v: value
 *
 * Assigns the value @v to the @s symbol. If the value exists as
 * a local symbol, it's assigned to it; otherwise, it's set as a global
 * symbol (and created if it does not exist).
 */
mpdm_v mpsl_set_symbol(mpdm_v s, mpdm_v v)
{
	mpdm_sset(mpsl_local_find_symbol(s), s, v);
	return(v);
}


/**
 * mpsl_get_symbol - Gets the value of a symbol
 * @s: symbol name
 *
 * Gets the value of a symbol. The symbol can be local or global
 * (if the symbol exists in both tables, the local value will be returned).
 */
mpdm_v mpsl_get_symbol(mpdm_v s)
{
	return(mpdm_sget(mpsl_local_find_symbol(s), s));
}


/** opcodes **/

#define _O_TYPE static mpdm_v
#define _O_ARGS mpdm_v c, mpdm_v a, int * f

_O_TYPE _mpsl_exec_i(_O_ARGS);

#define C(n) mpdm_aget(c, n)
#define C0 C(0)
#define C1 C(1)

#define M(n) _mpsl_exec_i(C(n), a, f)
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

_O_TYPE _O_multi(_O_ARGS) { mpdm_v v=RF(M1); if(!*f) v=M2; else UF(v); return(v); }
_O_TYPE _O_literal(_O_ARGS) { return(mpdm_clone(C1)); }
_O_TYPE _O_symval(_O_ARGS) { return(GET(M1)); }
_O_TYPE _O_assign(_O_ARGS) { mpdm_v v=RF(M1); return(SET(UF(v), M2)); }
_O_TYPE _O_exec(_O_ARGS) { mpdm_v v=RF(M1); return(mpdm_exec(UF(v), M2)); }
_O_TYPE _O_if(_O_ARGS) { return(ISTRU(M1) ? M2 : M3); }
_O_TYPE _O_while(_O_ARGS) { while(! *f && ISTRU(M1)) M2; if(*f == 1) *f=0; return(NULL); }
_O_TYPE _O_local(_O_ARGS) { mpsl_local_set_symbols(M1, NULL); return(NULL); }
_O_TYPE _O_uminus(_O_ARGS) { return(MPDM_R(-RM1)); }
_O_TYPE _O_add(_O_ARGS) { return(MPDM_R(RM1 + RM2)); }
_O_TYPE _O_sub(_O_ARGS) { return(MPDM_R(RM1 - RM2)); }
_O_TYPE _O_mul(_O_ARGS) { return(MPDM_R(RM1 * RM2)); }
_O_TYPE _O_div(_O_ARGS) { return(MPDM_R(RM1 / RM2)); }
_O_TYPE _O_mod(_O_ARGS) { return(MPDM_I(IM1 % IM2)); }
_O_TYPE _O_not(_O_ARGS) { return(BOOL(! ISTRU(M1))); }
_O_TYPE _O_and(_O_ARGS) { mpdm_v r=M1; return(ISTRU(r) ? M2 : r); }
_O_TYPE _O_or(_O_ARGS) { mpdm_v r=M1; return(ISTRU(r) ? r : M2); }
_O_TYPE _O_numlt(_O_ARGS) { return(BOOL(RM1 < RM2)); }
_O_TYPE _O_numle(_O_ARGS) { return(BOOL(RM1 <= RM2)); }
_O_TYPE _O_numgt(_O_ARGS) { return(BOOL(RM1 > RM2)); }
_O_TYPE _O_numge(_O_ARGS) { return(BOOL(RM1 >= RM2)); }
_O_TYPE _O_strcat(_O_ARGS) { return(mpdm_strcat(M1, M2)); }
_O_TYPE _O_streq(_O_ARGS) { return(BOOL(mpdm_cmp(M1, M2) == 0)); }
_O_TYPE _O_immpinc(_O_ARGS) { mpdm_v s=M1; return(SET(s, MPDM_R(R(GET(s)) + 1))); }
_O_TYPE _O_immpdec(_O_ARGS) { mpdm_v s=M1; return(SET(s, MPDM_R(R(GET(s)) - 1))); }
_O_TYPE _O_immadd(_O_ARGS) { mpdm_v s=RF(M1); return(SET(UF(s), MPDM_R(R(GET(s)) + RM2))); }
_O_TYPE _O_immsub(_O_ARGS) { mpdm_v s=RF(M1); return(SET(UF(s), MPDM_R(R(GET(s)) - RM2))); }
_O_TYPE _O_immmul(_O_ARGS) { mpdm_v s=RF(M1); return(SET(UF(s), MPDM_R(R(GET(s)) * RM2))); }
_O_TYPE _O_immdiv(_O_ARGS) { mpdm_v s=RF(M1); return(SET(UF(s), MPDM_R(R(GET(s)) / RM2))); }
_O_TYPE _O_immmod(_O_ARGS) { mpdm_v s=RF(M1); return(SET(UF(s), MPDM_R(I(GET(s)) % IM2))); }
_O_TYPE _O_immsinc(_O_ARGS) { mpdm_v s=RF(M1); mpdm_v v=GET(s); SET(UF(s), MPDM_R(R(v) + 1)); return(v); }
_O_TYPE _O_immsdec(_O_ARGS) { mpdm_v s=RF(M1); mpdm_v v=GET(s); SET(UF(s), MPDM_R(R(v) - 1)); return(v); }
_O_TYPE _O_numeq(_O_ARGS) { mpdm_v v1=M1; mpdm_v v2=M2; return(BOOL((v1 == NULL || v2 == NULL) ? (v1 == v2) : (R(v1) == R(v2)))); }
_O_TYPE _O_break(_O_ARGS) { *f=1; return(NULL); }
_O_TYPE _O_return(_O_ARGS) { mpdm_v v=M1; *f=-1; return(v); }

_O_TYPE _O_foreach(_O_ARGS)
/* foreach loop */
{
	mpdm_v s=RF(M1);
	mpdm_v v=RF(M2);
	int n;

	for(n=0;n < mpdm_size(v) && ! *f;n++)
	{
		SET(s, mpdm_aget(v, n));
		M3;
	}

	if(*f == 1) *f=0;

	UF(s); UF(v);

	return(NULL);
}


_O_TYPE _O_range(_O_ARGS)
/* build list from range of two numeric values */
{
	double n;
	double v1=RM1;
	double v2=RM2;
	mpdm_v ret=MPDM_A(0);

	if(v1 < v2)
		for(n=v1;n <= v2;n++)
			mpdm_apush(ret, MPDM_R(n));
	else
		for(n=v1;n >= v2;n--)
			mpdm_apush(ret, MPDM_R(n));

	return(ret);
}


_O_TYPE _O_list(_O_ARGS)
/* build list from instructions */
{
	int n;
	mpdm_v ret=RF(MPDM_A(mpdm_size(c) - 1));

	for(n=1;n < mpdm_size(c);n++)
		mpdm_aset(ret, M(n), n - 1);

	return(UF(ret));
}


_O_TYPE _O_hash(_O_ARGS)
/* build hash from instructions */
{
	int n;
	mpdm_v ret=RF(MPDM_H(0));

	for(n=1;n < mpdm_size(c);n += 2)
		mpdm_hset(ret, M(n), M(n + 1));

	return(UF(ret));
}

_O_TYPE _O_subframe(_O_ARGS)
/* runs an instruction inside a subroutine frame */
{
	mpdm_v ret;

	/* creates a subroutine frame */
	mpsl_local_add_subframe();

	/* creates the arguments (if any) as local variables */
	mpsl_local_set_symbols(M2, a);

	/* execute instruction */
	ret=M1;

	/* destroys the frame */
	mpsl_local_del_subframe();

	return(ret);
}


_O_TYPE _O_blkframe(_O_ARGS)
/* runs an instruction under a block frame */
{
	mpdm_v ret;

	mpsl_local_add_blkframe();
	ret=M1;
	mpsl_local_del_blkframe();

	return(ret);
}


static struct _op
{
	wchar_t * name;
	mpdm_v (* func)(_O_ARGS);
} _op_table[]=
{
	{ L"MULTI",	_O_multi },
	{ L"LITERAL",	_O_literal },
	{ L"SYMVAL",	_O_symval },
	{ L"ASSIGN",	_O_assign },
	{ L"EXEC",	_O_exec },
	{ L"IF",	_O_if },
	{ L"WHILE",	_O_while },
	{ L"FOREACH",	_O_foreach },
	{ L"SUBFRAME",	_O_subframe },
	{ L"BLKFRAME",	_O_blkframe },
	{ L"BREAK",	_O_break },
	{ L"RETURN",	_O_return },
	{ L"LOCAL",	_O_local },
	{ L"LIST",	_O_list },
	{ L"HASH",	_O_hash },
	{ L"RANGE",	_O_range },
	{ L"UMINUS",	_O_uminus },
	{ L"ADD",	_O_add },
	{ L"SUB",	_O_sub },
	{ L"MUL",	_O_mul },
	{ L"DIV",	_O_div },
	{ L"MOD",	_O_mod },
	{ L"SINC",	_O_immsinc },
	{ L"SDEC",	_O_immsdec },
	{ L"PINC",	_O_immpinc },
	{ L"PDEC",	_O_immpdec },
	{ L"IADD",	_O_immadd },
	{ L"ISUB",	_O_immsub },
	{ L"IMUL",	_O_immmul },
	{ L"IDIV",	_O_immdiv },
	{ L"IMOD",	_O_immmod },
	{ L"NOT",	_O_not },
	{ L"AND",	_O_and },
	{ L"OR",	_O_or },
	{ L"NUMEQ",	_O_numeq },
	{ L"NUMLT",	_O_numlt },
	{ L"NUMLE",	_O_numle },
	{ L"NUMGT",	_O_numgt },
	{ L"NUMGE",	_O_numge },
	{ L"STRCAT",	_O_strcat },
	{ L"STREQ",	_O_streq },
	{ NULL,		NULL }
};


mpdm_v _mpsl_op(wchar_t * opcode)
{
	if(_mpsl_ops == NULL)
	{
		int n;

		/* first time; load opcodes */
		_mpsl_ops=mpdm_ref(MPDM_H(0));

		for(n=0;_op_table[n].name != NULL;n++)
		{
			mpdm_v v=MPDM_LS(_op_table[n].name);
			v->ival=n;
			v->flags |= MPDM_IVAL;

			/* keys and values are the same */
			mpdm_hset(_mpsl_ops, v, v);
		}
	}

	return(mpdm_hget_s(_mpsl_ops, opcode));
}


/**
 * _mpsl_exec_i - Executes one mpsl instruction
 * @c: Multiple value containing the bytecode
 * @args: Optional arguments for the bytecode
 * @f: Pointer to the flow control status
 *
 * Executes one mpsl instruction in the mpsl virtual machine.
 * Called from _mpsl_exec() (which holds the flow control
 * status variable).
 */
_O_TYPE _mpsl_exec_i(_O_ARGS)
{
	mpdm_v ret=NULL;

	if(c != NULL)
	{
		int op;

		/* gets the opcode */
		op=mpdm_ival(C0);

		/* if it's a valid opcode... */
		if(op >= 0 && op < sizeof(_op_table) / sizeof(struct _op))
		{
			struct _op * o=&_op_table[op];

			/* and call it if existent */
			if(o->func != NULL)
				ret=o->func(c, a, f);
		}
	}

	if(_mpsl_trace)
		printf("** %ls: %ls\n", mpdm_string(C0), mpdm_string(ret));

	/* sweep some values */
	mpdm_sweep(0);

	return(ret);
}


mpdm_v _mpsl_exec(mpdm_v c, mpdm_v a)
{
	int f=0;
	mpdm_v v;

	/* reference both code and arguments */
	mpdm_ref(c); mpdm_ref(a);

	/* execute first instruction */
	v=_mpsl_exec_i(c, a, &f);

	/* unreference */
	mpdm_unref(a); mpdm_unref(c);

	return(v);
}
