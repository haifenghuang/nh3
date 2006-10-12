/*

    MPSL - Minimum Profit Scripting Language
    Copyright (C) 2003/2006 Angel Ortega <angel@triptico.com>

    mpsl_f.c - Minimum Profit Scripting Language Function Library

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

/*******************
	Code
********************/

#define A(n) mpdm_aget(a, n)
#define A0 A(0)
#define A1 A(1)
#define A2 A(2)
#define IA(n) mpdm_ival(A(n))
#define IA0 IA(0)
#define IA1 IA(1)
#define IA2 IA(2)
#define IA3 IA(3)

static mpdm_t F_size(mpdm_t a) { return(MPDM_I(mpdm_size(A0))); }
static mpdm_t F_clone(mpdm_t a) { return(mpdm_clone(A0)); }
static mpdm_t F_dump(mpdm_t a) { mpdm_dump(A0); return(NULL); }
static mpdm_t F_cmp(mpdm_t a) { return(MPDM_I(mpdm_cmp(A0, A1))); }

static mpdm_t F_is_array(mpdm_t a) { return(mpsl_boolean(MPDM_IS_ARRAY(A0))); }
static mpdm_t F_is_hash(mpdm_t a) { return(mpsl_boolean(MPDM_IS_HASH(A0))); }
static mpdm_t F_is_exec(mpdm_t a) { return(mpsl_boolean(MPDM_IS_EXEC(A0))); }

static mpdm_t F_splice(mpdm_t a) { return(mpdm_splice(A0,A1,IA2,IA3)); }
static mpdm_t F_expand(mpdm_t a) { return(mpdm_expand(A0,IA1,IA2)); }
static mpdm_t F_collapse(mpdm_t a) { return(mpdm_collapse(A0,IA1,IA2)); }
static mpdm_t F_ins(mpdm_t a) { return(mpdm_ins(A0,A1,IA2)); }
static mpdm_t F_adel(mpdm_t a) { return(mpdm_adel(A0,IA1)); }
static mpdm_t F_shift(mpdm_t a) { return(mpdm_shift(A0)); }
static mpdm_t F_push(mpdm_t a) { return(mpdm_push(A0,A1)); }
static mpdm_t F_pop(mpdm_t a) { return(mpdm_pop(A0)); }
static mpdm_t F_queue(mpdm_t a) { return(mpdm_queue(A0,A1,IA2)); }
static mpdm_t F_seek(mpdm_t a) { return(MPDM_I(mpdm_seek(A0,A1,IA2))); }
static mpdm_t F_sort(mpdm_t a) { return(mpdm_sort_cb(A0,1,A1)); }
static mpdm_t F_split(mpdm_t a) { return(mpdm_split(A0,A1)); }
static mpdm_t F_join(mpdm_t a) { return(mpdm_join(A0,A1)); }

static mpdm_t F_hsize(mpdm_t a) { return(MPDM_I(mpdm_hsize(A0))); }
static mpdm_t F_exists(mpdm_t a) { return(mpsl_boolean(mpdm_exists(A0, A1))); }
static mpdm_t F_hdel(mpdm_t a) { return(mpdm_hdel(A0, A1)); }
static mpdm_t F_keys(mpdm_t a) { return(mpdm_keys(A0)); }

static mpdm_t F_open(mpdm_t a) { return(mpdm_open(A0, A1)); }
static mpdm_t F_close(mpdm_t a) { return(mpdm_close(A0)); }
static mpdm_t F_read(mpdm_t a) { return(mpdm_read(A0)); }
static mpdm_t F_write(mpdm_t a) { return(MPDM_I(mpdm_write(A0,A1))); }
static mpdm_t F_getchar(mpdm_t a) { return(mpdm_getchar(A0)); }
static mpdm_t F_putchar(mpdm_t a) { return(mpdm_putchar(A0, A1)); }
static mpdm_t F_fseek(mpdm_t a) { return(MPDM_I(mpdm_fseek(A0, IA1, IA2))); }
static mpdm_t F_ftell(mpdm_t a) { return(MPDM_I(mpdm_ftell(A0))); }

static mpdm_t F_unlink(mpdm_t a) { return(mpsl_boolean(mpdm_unlink(A0))); }
static mpdm_t F_stat(mpdm_t a) { return(mpdm_stat(A0)); }
static mpdm_t F_chmod(mpdm_t a) { return(MPDM_I(mpdm_chmod(A0,A1))); }
static mpdm_t F_chown(mpdm_t a) { return(MPDM_I(mpdm_chown(A0,A1,A2))); }
static mpdm_t F_glob(mpdm_t a) { return(mpdm_glob(A0)); }
static mpdm_t F_encoding(mpdm_t a) { return(MPDM_I(mpdm_encoding(A0))); }
static mpdm_t F_popen(mpdm_t a) { return(mpdm_popen(A0, A1)); }
static mpdm_t F_pclose(mpdm_t a) { return(mpdm_pclose(A0)); }

static mpdm_t F_regex(mpdm_t a) { return(mpdm_regex(A0,A1,IA2)); }
static mpdm_t F_sregex(mpdm_t a) { return(mpdm_sregex(A0,A1,A2,IA3)); }

static mpdm_t F_gettext(mpdm_t a) { return(mpdm_gettext(A0)); }
static mpdm_t F_gettext_domain(mpdm_t a) { return(MPDM_I(mpdm_gettext_domain(A0, A1))); }

static mpdm_t F_load(mpdm_t a) { return(mpdm_exec(mpsl_compile_file(A0), NULL)); }
static mpdm_t F_error(mpdm_t a) { return(mpsl_error(A0)); }
static mpdm_t F_sweep(mpdm_t a) { mpdm_sweep(IA0); return(NULL); }

static mpdm_t F_eval(mpdm_t a)
{
	mpdm_t c;

	a = mpdm_clone(a);
	c = mpdm_adel(a, 0);

	return(mpsl_eval(c, a));
}

static mpdm_t F_sprintf(mpdm_t a)
{
	mpdm_t f;
	mpdm_t v;

	a = mpdm_clone(a);
	f = mpdm_adel(a, 0);

	/* if the first argument is an array, take it as the arguments */
	if((v = mpdm_aget(a, 0)) != NULL && MPDM_IS_ARRAY(v))
		a = v;

	return(mpdm_sprintf(f, a));
}


static mpdm_t F_print(mpdm_t a)
{
	int n;

	for(n = 0;n < mpdm_size(a);n++)
		mpdm_write_wcs(stdout, mpdm_string(A(n)));
	return(NULL);
}


static mpdm_t F_chr(mpdm_t a)
{
	wchar_t tmp[2];

	tmp[0] = (wchar_t) mpdm_ival(mpdm_aget(a, 0));
	tmp[1] = L'\0';

	return(MPDM_S(tmp));
}


static mpdm_t F_ord(mpdm_t a)
{
	int ret = 0;
	mpdm_t v = mpdm_aget(a, 0);

	if(v != NULL)
	{
		wchar_t * ptr = mpdm_string(v);
		ret = (int) *ptr;
	}

	return(MPDM_I(ret));
}


static mpdm_t F_map(mpdm_t a)
{
	mpdm_t key = mpdm_aget(a, 0);
	mpdm_t set = mpdm_aget(a, 1);
	mpdm_t out;

	/* map NULL to NULL */
	if(set == NULL) return(NULL);

	out = mpdm_ref(MPDM_A(mpdm_size(set)));

	if(MPDM_IS_EXEC(key))
	{
		int n;

		/* executes the code using the element as argument
		   and stores the result in the output array */
		for(n = 0;n < mpdm_size(set);n++)
			mpdm_aset(out, mpdm_exec_1(key, mpdm_aget(set, n)), n);
	}
	else
	if(MPDM_IS_HASH(key))
	{
		int n;

		/* maps each value using the element as key */
		for(n = 0;n < mpdm_size(set);n++)
			mpdm_aset(out, mpdm_hget(key, mpdm_aget(set, n)), n);
	}

	return(mpdm_unref(out));
}


static mpdm_t F_grep(mpdm_t a)
{
	mpdm_t key = mpdm_aget(a, 0);
	mpdm_t set = mpdm_aget(a, 1);
	mpdm_t out = mpdm_ref(MPDM_A(0));

	if(MPDM_IS_EXEC(key))
	{
		int n;

		/* it's executable */
		for(n = 0;n < mpdm_size(set);n++)
		{
			mpdm_t v = mpdm_aget(set, n);

			if(mpsl_is_true(mpdm_exec_1(key, v)))
				mpdm_push(out, v);
		}
	}
	else
	if(key->flags & MPDM_STRING)
	{
		int n;

		/* it's a string; use it as a regular expression */
		for(n = 0;n < mpdm_size(set);n++)
		{
			mpdm_t v = mpdm_aget(set, n);

			if(mpdm_regex(key, v, 0))
				mpdm_push(out, v);
		}
	}

	return(mpdm_size(mpdm_unref(out)) == 0 ? NULL : out);
}

static struct
{
	wchar_t * name;
	mpdm_t (* func)(mpdm_t);
} mpsl_funcs[]=
{
	{ L"size",	F_size },
	{ L"clone",	F_clone },
	{ L"dump",	F_dump },
	{ L"cmp",	F_cmp },
	{ L"is_array",	F_is_array },
	{ L"is_hash",	F_is_hash },
	{ L"is_exec",	F_is_exec },
	{ L"splice",	F_splice },
	{ L"expand",	F_expand },
	{ L"collapse",	F_collapse },
	{ L"ins",	F_ins },
	{ L"adel",	F_adel },
	{ L"shift",	F_shift },
	{ L"push",	F_push },
	{ L"pop",	F_pop },
	{ L"queue",	F_queue },
	{ L"seek",	F_seek },
	{ L"sort",	F_sort },
	{ L"split",	F_split },
	{ L"join",	F_join },
	{ L"hsize",	F_hsize },
	{ L"exists",	F_exists },
	{ L"hdel",	F_hdel },
	{ L"keys",	F_keys },
	{ L"open",	F_open },
	{ L"close",	F_close },
	{ L"read",	F_read },
	{ L"write",	F_write },
	{ L"getchar",	F_getchar },
	{ L"putchar",	F_putchar },
	{ L"fseek",	F_fseek },
	{ L"ftell",	F_ftell },
	{ L"unlink",	F_unlink },
	{ L"stat",	F_stat },
	{ L"chmod",	F_chmod },
	{ L"chown",	F_chown },
	{ L"glob",	F_glob },
	{ L"encoding",	F_encoding },
	{ L"popen",	F_popen },
	{ L"pclose",	F_pclose },
	{ L"regex",	F_regex },
	{ L"sregex",	F_sregex },
	{ L"load",	F_load },
	{ L"error",	F_error },
	{ L"eval",	F_eval },
	{ L"print",	F_print },
	{ L"gettext",	F_gettext },
	{ L"gettext_domain", F_gettext_domain },
	{ L"sprintf",	F_sprintf },
	{ L"sweep",	F_sweep },
	{ L"chr",	F_chr },
	{ L"ord",	F_ord },
	{ L"map",	F_map },
	{ L"grep",	F_grep },
	{ NULL,		NULL }
};


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

	for(n = 0;n < argc;n++)
		mpdm_push(ARGV, MPDM_MBS(argv[n]));

	mpdm_hset_s(mpdm_root(), L"ARGV", ARGV);
}


/* in mpsl_c.c */
mpdm_t mpsl_build_opcodes(void);


/**
 * mpsl_startup - Initializes MPSL.
 *
 * Initializes the Minimum Profit Scripting Language. Returns 0 if
 * everything went OK.
 */
int mpsl_startup(void)
{
	int n;
	mpdm_t r;
	mpdm_t m;
	mpdm_t c;

	/* startup MPDM */
	mpdm_startup();

	r = mpdm_root();

	/* creates all the symbols in the CORE library */
	c = MPDM_H(0);
	for(n = 0;mpsl_funcs[n].name != NULL;n++)
	{
		mpdm_t f = MPDM_S(mpsl_funcs[n].name);
		mpdm_t x = MPDM_X(mpsl_funcs[n].func);

		mpdm_hset(r, f, x);
		mpdm_hset(c, f, x);
	}

	/* creates INC, unless already defined */
	if(mpdm_hget_s(r, L"INC") == NULL)
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
	mpdm_hset_s(m, L"CORE", c);

	return(0);
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
