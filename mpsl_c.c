/*

    MPSL - Minimum Profit Scripting Language
    Copyright (C) 2003/2010 Angel Ortega <angel@triptico.com>

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
#include <math.h>
#include "mpdm.h"
#include "mpsl.h"


/** data **/

/* global abort flag */
int mpsl_abort = 0;

/* temporary storage for the local symbol table */
static mpdm_t local_symtbl = NULL;

/* temporary storage for the opcode table
   (only usable while compiling) */
mpdm_t mpsl_opcodes = NULL;

/* flag to mark if an execution is from inside constant folding */
static int in_constant_folding = 0;

/* pointer to a trap function */
static mpdm_t mpsl_trap_func = NULL;


/** code **/

/**
 * mpsl_is_true - Tests if a value is true.
 * @v: the value
 *
 * If @v is a valid MPSL 'false' value (NULL, "" or the "0" string),
 * returns zero, or nonzero otherwise.
 */
int mpsl_is_true(mpdm_t v)
{
	/* if value is NULL, it's false */
	if (v == NULL)
		return 0;

	/* if it's a printable string... */
	if (v->flags & MPDM_STRING) {
		wchar_t * ptr = (wchar_t *)v->data;

		/* ... and it's "" or the "0" string, it's false */
		if(*ptr == L'\0' || (*ptr == L'0' && *(ptr + 1) == L'\0'))
			return 0;
	}

	/* any other case is true */
	return 1;
}


/**
 * mpsl_boolean - Returns 'true' or 'false' MPSL stock values.
 * @b: boolean selector
 *
 * Returns MPSL's 'false' or 'true' values depending on the value in @b.
 */
mpdm_t mpsl_boolean(int b)
{
	return b ? mpdm_hget_s(mpdm_root(), L"TRUE") : NULL;
}


static mpdm_t find_local_symtbl(mpdm_t s, mpdm_t l)
/* finds the local symbol table hash that holds l */
{
	int n;
	mpdm_t v = NULL;

	/* no local symbol table? nothing to find */
	if(l == NULL)
		return NULL;

	/* if s is multiple, take just the first element */
	if(MPDM_IS_ARRAY(s))
		s = mpdm_aget(s, 0);

	/* travel the local symbol table trying to find it */
	for (n = mpdm_size(l) - 1; n >= 0; n--) {
		mpdm_t h = mpdm_aget(l, n);

		if (mpdm_exists(h, s)) {
			v = h;
			break;
		}
	}

	return v;
}


static void set_local_symbols(mpdm_t s, mpdm_t v, mpdm_t l)
/* sets (or creates) a list of local symbols with a list of values */
{
	mpdm_t h;

	if (l == NULL)
		return;

	/* gets the top local variable frame */
	h = mpdm_aget(l, -1);

	if (MPDM_IS_ARRAY(s) || MPDM_IS_ARRAY(v)) {
		int n;
		mpdm_t a;

		for (n = 0; n < mpdm_size(s); n++)
			mpdm_hset(h, mpdm_aget(s, n), mpdm_aget(v, n));

		/* store the rest of arguments into _ */
		a = MPDM_A(0);

		for (; n < mpdm_size(v); n++)
			mpdm_push(a, mpdm_aget(v, n));

		mpdm_hset_s(h, L"_", a);
	}
	else
		mpdm_hset(h, s, v);
}


static mpdm_t get_symbol(mpdm_t s, mpdm_t l)
/* gets a symbol from a local symbol table, or the global */
{
	return mpdm_sget(find_local_symtbl(s, l), s);
}


static mpdm_t set_symbol(mpdm_t s, mpdm_t v, mpdm_t l)
/* sets a symbol in a local symbol table, or the global */
{
	mpdm_sset(find_local_symtbl(s, l), s, v);
	return v;
}


/**
 * mpsl_set_symbol - Sets value to a symbol.
 * @s: symbol name
 * @v: value
 *
 * Assigns the value @v to the @s symbol. If the value exists as
 * a local symbol, it's assigned to it; otherwise, it's set as a global
 * symbol (and created if it does not exist).
 *
 * This function is only meant to be executed from inside an MPSL
 * program; from outside, it's exactly the same as calling mpdm_sset()
 * (as the local symbol table won't exist).
 */
mpdm_t mpsl_set_symbol(mpdm_t s, mpdm_t v)
{
	return set_symbol(s, v, local_symtbl);
}


/**
 * mpsl_get_symbol - Gets the value of a symbol.
 * @s: symbol name
 *
 * Gets the value of a symbol. The symbol can be local or global
 * (if the symbol exists in both tables, the local value will be returned).
 *
 * This function is only meant to be executed from inside an MPSL
 * program; from outside, it's exactly the same as calling mpdm_sget()
 * (as the local symbol table won't exist).
 */
mpdm_t mpsl_get_symbol(mpdm_t s)
{
	return get_symbol(s, local_symtbl);
}


/**
 * mpsl_error - Generates an error.
 * @err: the error message
 *
 * Generates an error. The @err error message is stored in the ERROR
 * mpsl variable and the mpsl_abort global flag is set, so no further
 * mpsl code can be executed until reset.
 */
mpdm_t mpsl_error(mpdm_t err)
{
	/* abort further execution */
	mpsl_abort = 1;

	/* set the error */
	return mpdm_hset_s(mpdm_root(), L"ERROR", err);
}


/** opcode macro helpers **/

#define O_TYPE static mpdm_t
#define O_ARGS mpdm_t c, mpdm_t a, mpdm_t l, int * f

O_TYPE mpsl_exec_i(O_ARGS);

#define C(n) mpdm_aget(c, n)
#define C0 C(0)
#define C1 C(1)

#define M(n) mpsl_exec_i(C(n), a, l, f)
#define M1 M(1)
#define M2 M(2)
#define M3 M(3)

#define R(x) mpdm_rval(x)
#define I(x) mpdm_ival(x)

#define RM1 mpdm_rval(M(1))
#define RM2 mpdm_rval(M(2))
#define IM1 mpdm_ival(M(1))
#define IM2 mpdm_ival(M(2))

#define GET(m) get_symbol(m, l)
#define SET(m, v) set_symbol(m, v, l)
#define BOOL mpsl_boolean

#define RF(v) mpdm_ref(v)
#define UF(v) v = mpdm_unref(v)
#define UFND(v) mpdm_unrefnd(v)

static int is_true_uf(mpdm_t v)
{
	int r;

	RF(v);
	r = mpsl_is_true(v);
	UF(v);

	return r;
}


/** opcodes **/

O_TYPE O_literal(O_ARGS) {
	return mpdm_clone(C1);
}

O_TYPE O_multi(O_ARGS) {
	mpdm_t v = M1;

	if (!*f) {
		mpdm_t t = v;

		RF(t);
		v = M2;
		UF(t);
	}

	return v;
}

O_TYPE O_imulti(O_ARGS) {
	mpdm_t v = RF(M1);

	if (!*f) {
		mpdm_t w = RF(M2);
		UF(w);
	}

	return UFND(v);
}

O_TYPE O_symval(O_ARGS) {
	mpdm_t v = RF(M1);
	mpdm_t r = GET(v);
	UF(v);

	return r;
}

O_TYPE O_assign(O_ARGS) {
	mpdm_t v = RF(M1);
	mpdm_t r = SET(v, M2);
	UF(v);

	return r;
}

O_TYPE O_if(O_ARGS) {
	mpdm_t r;

	if (is_true_uf(M1))
		r = M2;
	else
		r = M3;

	return r;
}

O_TYPE O_local(O_ARGS) {
	mpdm_t v = RF(M1);
	set_local_symbols(v, NULL, l);
	UF(v);

	return NULL;
}

O_TYPE O_uminus(O_ARGS) {
	mpdm_t v = RF(M1);
	mpdm_t r = MPDM_R(-mpdm_rval(v));
	UF(v);

	return r;
}

O_TYPE O_add(O_ARGS) { return MPDM_R(RM1 + RM2); }
O_TYPE O_sub(O_ARGS) { return MPDM_R(RM1 - RM2); }
O_TYPE O_mul(O_ARGS) { return MPDM_R(RM1 * RM2); }
O_TYPE O_div(O_ARGS) { return MPDM_R(RM1 / RM2); }
O_TYPE O_mod(O_ARGS) { return MPDM_I(IM1 % IM2); }

O_TYPE O_not(O_ARGS) {
	return BOOL(!is_true_uf(M1));
}

O_TYPE O_and(O_ARGS) {
	mpdm_t v = M1;
	mpdm_t r;

	if (mpsl_is_true(v)) {
		RF(v);
		r = M2;
		UF(v);
	}
	else
		r = v;

	return r;
}

O_TYPE O_or(O_ARGS) {
	mpdm_t v = M1;
	mpdm_t r;

	if (!mpsl_is_true(v)) {
		RF(v);
		r = M2;
		UF(v);
	}
	else
		r = v;

	return r;
}

O_TYPE O_bitand(O_ARGS) { return MPDM_I(IM1 & IM2); }
O_TYPE O_bitor(O_ARGS) { return MPDM_I(IM1 | IM2); }
O_TYPE O_bitxor(O_ARGS) { return MPDM_I(IM1 ^ IM2); }
O_TYPE O_shl(O_ARGS) { return MPDM_I(IM1 << IM2); }
O_TYPE O_shr(O_ARGS) { return MPDM_I(IM1 >> IM2); }
O_TYPE O_pow(O_ARGS) { return MPDM_R(pow(RM1, RM2)); }
O_TYPE O_numlt(O_ARGS) { return BOOL(RM1 < RM2); }
O_TYPE O_numle(O_ARGS) { return BOOL(RM1 <= RM2); }
O_TYPE O_numgt(O_ARGS) { return BOOL(RM1 > RM2); }
O_TYPE O_numge(O_ARGS) { return BOOL(RM1 >= RM2); }

O_TYPE O_strcat(O_ARGS) {
	mpdm_t v1 = RF(M1);
	mpdm_t v2 = RF(M2);

	mpdm_t r = mpdm_strcat(v1, v2);

	UF(v2);
	UF(v1);

	return r;
}

O_TYPE O_streq(O_ARGS) {
	mpdm_t v1 = RF(M1);
	mpdm_t v2 = RF(M2);

	mpdm_t r = BOOL(mpdm_cmp(v1, v2) == 0);

	UF(v2);
	UF(v1);

	return r;
}

O_TYPE O_numeq(O_ARGS) { mpdm_t v1 = RF(M1); mpdm_t v2 = M2; UF(v1); return BOOL((v1 == NULL || v2 == NULL) ? (v1 == v2) : (R(v1) == R(v2))); }

O_TYPE O_break(O_ARGS) {
	*f = 1;
	return NULL;
}

O_TYPE O_return(O_ARGS) {
	mpdm_t v = M1;
	*f = -1;
	return v;
}

O_TYPE O_execsym(O_ARGS)
/* executes the value of a symbol */
{
	mpdm_t s, v, r = NULL;

	/* gets the symbol name */
	s = RF(M1);

	/* gets the symbol value */
	v = GET(s);

	if (!MPDM_IS_EXEC(v)) {
		/* not found or NULL value? error */
		mpdm_t t, w;
		char tmp[128];

		w = RF(mpdm_join_s(L".", s));
		t = RF(MPDM_2MBS((wchar_t *) w->data));

		snprintf(tmp, sizeof(tmp), "Undefined function %s()",
			(char *)t->data);

		mpsl_error(MPDM_MBS(tmp));

		UF(w);
		UF(t);
	}
	else {
		/* save current local symbol table */
		mpdm_t t = local_symtbl;

		/* substitute with this one */
		local_symtbl = l;

		/* execute */
		r = mpdm_exec(v, M2);

		/* and get back to the original one */
		local_symtbl = t;
	}

	UF(s);

	return r;
}


O_TYPE O_while(O_ARGS)
/* while loop */
{
	mpdm_t r = NULL;

	while (!*f && is_true_uf(M1)) {
		UF(r);
		r = RF(M2);
	}

	if (*f == 1)
		*f = 0;

	return UFND(r);
}


O_TYPE O_foreach(O_ARGS)
/* foreach loop */
{
	mpdm_t s = RF(M1);
	mpdm_t v = RF(M2);
	mpdm_t r = NULL;
	int n;

	for (n = 0; n < mpdm_size(v) && ! *f; n++) {
		SET(s, mpdm_aget(v, n));
		UF(r);
		r = RF(M3);
	}

	if (*f == 1)
		*f = 0;

	UF(s); UF(v);

	return UFND(r);
}


O_TYPE O_range(O_ARGS)
/* build list from range of two numeric values */
{
	double n;
	double v1 = RM1;
	double v2 = RM2;
	mpdm_t ret = MPDM_A(0);

	if (v1 < v2)
		for (n = v1; n <= v2; n++)
			mpdm_push(ret, MPDM_R(n));
	else
		for (n = v1; n >= v2; n--)
			mpdm_push(ret, MPDM_R(n));

	return ret;
}


O_TYPE O_list(O_ARGS)
/* build list from instructions */
{
	mpdm_t ret = RF(mpdm_size(c) == 2 ? MPDM_A(0) : M(2));

	mpdm_push(ret, M(1));
	return UFND(ret);
}


O_TYPE O_ilist(O_ARGS)
/* build and inverse list from instructions */
{
	mpdm_t ret = RF(mpdm_size(c) == 2 ? MPDM_A(0) : M(2));

	mpdm_ins(ret, M(1), 0);
	return UFND(ret);
}


O_TYPE O_hash(O_ARGS)
/* build hash from instructions */
{	
	mpdm_t k, v;
	mpdm_t ret = RF(mpdm_size(c) == 3 ? MPDM_H(0) : M(3));

	k = RF(M(1)); v = RF(M(2));
	mpdm_hset(ret, UF(k), UF(v));
	return UFND(ret);
}


O_TYPE generic_frame(O_ARGS)
/* runs an instruction under a frame */
{
	mpdm_t ret;

	/* if l is NULL (usually for subroutine frames),
	   create a new array for holding local symbol tables */
	if (l == NULL)
		l = MPDM_A(0);

	RF(l);

	/* create a new local symbol table */
	mpdm_push(l, MPDM_H(0));

	/* creates the arguments (if any) as local variables */
	set_local_symbols(M2, a, l);

	/* execute instruction */
	ret = M1;

	/* destroy the local symbol table */
	mpdm_adel(l, -1);

	UF(l);

	return ret;
}


O_TYPE O_blkframe(O_ARGS)
/* runs an instruction under a block frame */
{
	return generic_frame(c, a, l, f);
}


O_TYPE O_subframe(O_ARGS)
/* runs an instruction inside a subroutine frame */
{
	/* don't propagate the local symbol table,
	   triggering a new subroutine frame */
	return generic_frame(c, a, NULL, f);
}


static struct mpsl_op_s {
	wchar_t * name;
	int foldable;
	mpdm_t (* func)(O_ARGS);
} op_table[] = {
	{ L"LITERAL",	0,	O_literal },	/* *must* be the zeroth */
	{ L"MULTI",	0,	O_multi },
	{ L"IMULTI",	0,	O_imulti },
	{ L"SYMVAL",	0,	O_symval },
	{ L"ASSIGN",	0,	O_assign },
	{ L"EXECSYM",	0,	O_execsym },
	{ L"IF",	0,	O_if },
	{ L"WHILE",	0,	O_while },
	{ L"FOREACH",	0,	O_foreach },
	{ L"SUBFRAME",	0,	O_subframe },
	{ L"BLKFRAME",	0,	O_blkframe },
	{ L"BREAK",	0,	O_break },
	{ L"RETURN",	0,	O_return },
	{ L"LOCAL",	0,	O_local },
	{ L"LIST",	1,	O_list },
	{ L"ILIST",	1,	O_ilist },
	{ L"HASH",	1,	O_hash },
	{ L"RANGE",	1,	O_range },
	{ L"UMINUS",	1,	O_uminus },
	{ L"ADD",	1,	O_add },
	{ L"SUB",	1,	O_sub },
	{ L"MUL",	1,	O_mul },
	{ L"DIV",	1,	O_div },
	{ L"MOD",	1,	O_mod },
	{ L"NOT",	1,	O_not },
	{ L"AND",	1,	O_and },
	{ L"OR",	1,	O_or },
	{ L"NUMEQ",	1,	O_numeq },
	{ L"NUMLT",	1,	O_numlt },
	{ L"NUMLE",	1,	O_numle },
	{ L"NUMGT",	1,	O_numgt },
	{ L"NUMGE",	1,	O_numge },
	{ L"STRCAT",	1,	O_strcat },
	{ L"STREQ",	1,	O_streq },
	{ L"BITAND",	1,	O_bitand },
	{ L"BITOR",	1,	O_bitor },
	{ L"BITXOR",	1,	O_bitxor },
	{ L"SHL",	1,	O_shl },
	{ L"SHR",	1,	O_shr },
	{ L"POW",	1,	O_pow },
	{ NULL,		0,	NULL }
};


O_TYPE mpsl_exec_i(O_ARGS)
/* Executes one MPSL instruction in the MPSL virtual machine. Called
   from mpsl_exec_p() (which holds the flow control status variable) */
{
	struct mpsl_op_s * o;
	mpdm_t ret = NULL;

	/* if aborted or NULL, do nothing */
	if (mpsl_abort || c == NULL)
		return NULL;

	/* sweep some values */
	if (!in_constant_folding)
		mpdm_sweep(0);

	/* gets the opcode */
	o = &op_table[mpdm_ival(C0)];

	/* blindly call it, or crash */
	ret = o->func(c, a, l, f);

	if (mpsl_trap_func != NULL && !in_constant_folding) {
		mpdm_t f = mpsl_trap_func;

		mpsl_trap_func = NULL;
		mpdm_exec_3(f, c, a, ret);
		mpsl_trap_func = f;
	}

	return ret;
}


mpdm_t mpsl_exec_p(mpdm_t c, mpdm_t a)
/* executes an MPSL instruction stream */
{
	int f = 0;

	/* execute first instruction with a new flow control variable */
	return mpsl_exec_i(c, a, local_symtbl, &f);
}


static mpdm_t constant_fold(mpdm_t i)
/* tries to fold complex but constant expressions into a literal */
{
	int n;
	mpdm_t v, w;

	/* get the number opcode */
	n = mpdm_ival(mpdm_aget(i, 0));

	if (op_table[n].foldable) {
		/* test if all arguments are literal (opcode 0) */
		for (n = 1; n < mpdm_size(i); n++) {
			mpdm_t t = mpdm_aget(i, n);

			/* if it's not LITERAL, abort immediately */
			if (mpdm_ival(mpdm_aget(t, 0)) != 0)
				return i;
		}

		in_constant_folding = 1;

		/* execute the instruction and convert to LITERAL */
		v = RF(i);
		w = RF(mpsl_exec_p(v, NULL));
		i = mpsl_mkins(L"LITERAL", 1, w, NULL, NULL);
		UF(w);
		UF(v);

		in_constant_folding = 0;
	}

	return i;
}


mpdm_t mpsl_mkins(wchar_t * opcode, int args, mpdm_t a1, mpdm_t a2, mpdm_t a3)
/* creates an instruction */
{
	mpdm_t o;
	mpdm_t v;

	v = MPDM_A(args + 1);

	/* inserts the opcode */
	o = mpdm_hget_s(mpsl_opcodes, opcode);
	mpdm_aset(v, o, 0);

	switch (args) {
	case 3:		mpdm_aset(v, a3, 3);
	case 2:		mpdm_aset(v, a2, 2);
	case 1:		mpdm_aset(v, a1, 1);
	}

	v = constant_fold(v);

	return v;
}


mpdm_t mpsl_build_opcodes(void)
/* builds the table of opcodes */
{
	int n;
	mpdm_t r = MPDM_H(0);

	for (n = 0; op_table[n].name != NULL; n++) {
		mpdm_t v = MPDM_LS(op_table[n].name);

		mpdm_set_ival(v, n);

		/* keys and values are the same */
		mpdm_hset(r, v, v);
	}

	return r;
}


/**
 * mpsl_trap - Install a trapping function.
 * @trap_func: The trapping MPSL code
 *
 * Installs a trapping function. The function is an MPSL
 * executable value receiving 3 arguments: the code stream,
 * the arguments and the return value of the executed code.
 *
 * Returns NULL (previous versions returned the previous
 * trapping function).
 */
mpdm_t mpsl_trap(mpdm_t trap_func)
{
	mpdm_ref(trap_func);
	mpdm_unref(mpsl_trap_func);
	mpsl_trap_func = trap_func;

	return NULL;
}


/**
 * mpsl_argv - Fills the ARGV global array.
 * @argc: number of arguments
 * @argv: array of string values
 *
 * Fills the ARGV global MPSL array with an array of arguments. These
 * are usually the ones sent to main().
 */
void mpsl_argv(int argc, char * argv[])
{
	int n;
	mpdm_t ARGV;

	/* create the ARGV array */
	ARGV = MPDM_A(0);

	for (n = 0; n < argc; n++)
		mpdm_push(ARGV, MPDM_MBS(argv[n]));

	mpdm_hset_s(mpdm_root(), L"ARGV", ARGV);
}


/* in mpsl_f.c */
mpdm_t mpsl_build_funcs(void);


/**
 * mpsl_startup - Initializes MPSL.
 *
 * Initializes the Minimum Profit Scripting Language. Returns 0 if
 * everything went OK.
 */
int mpsl_startup(void)
{
	mpdm_t r;
	mpdm_t m;

	/* startup MPDM */
	mpdm_startup();

	r = mpdm_root();

	/* creates INC, unless already defined */
	if (mpdm_hget_s(r, L"INC") == NULL)
		mpdm_hset_s(r, L"INC", MPDM_A(0));

	/* the TRUE value */
	mpdm_hset_s(r, L"TRUE", MPDM_I(1));

	/* standard file descriptors */
	mpdm_hset_s(r, L"STDIN", MPDM_F(stdin));
	mpdm_hset_s(r, L"STDOUT", MPDM_F(stdout));
	mpdm_hset_s(r, L"STDERR", MPDM_F(stderr));

	/* home and application directories */
	mpdm_hset_s(r, L"HOMEDIR", mpdm_home_dir());
	mpdm_hset_s(r, L"APPDIR", mpdm_app_dir());

	/* fill now the MPSL hash */
	m = MPDM_H(0);
	mpdm_hset_s(r, L"MPSL", m);

	/* store things there */
	mpdm_hset_s(m, L"VERSION", MPDM_MBS(VERSION));
	mpdm_hset_s(m, L"OPCODE", mpsl_build_opcodes());
	mpdm_hset_s(m, L"LC", MPDM_H(0));
	mpdm_hset_s(m, L"CORE", mpsl_build_funcs());

	mpdm_dump_1 = mpsl_dump_1;

	return 0;
}


/**
 * mpsl_shutdown - Shuts down MPSL.
 *
 * Shuts down MPSL. No MPSL functions should be used from now on.
 */
void mpsl_shutdown(void)
{
	mpdm_shutdown();
}
