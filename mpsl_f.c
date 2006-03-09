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
static mpdm_t F_write(mpdm_t a) { return(mpsl_boolean(mpdm_write(A0,A1))); }
static mpdm_t F_unlink(mpdm_t a) { return(mpsl_boolean(mpdm_unlink(A0))); }
static mpdm_t F_glob(mpdm_t a) { return(mpdm_glob(A0)); }
static mpdm_t F_encoding(mpdm_t a) { return(MPDM_I(mpdm_encoding(A0))); }

static mpdm_t F_regex(mpdm_t a) { return(mpdm_regex(A0,A1,IA2)); }
static mpdm_t F_sregex(mpdm_t a) { return(mpdm_sregex(A0,A1,A2,IA3)); }

static mpdm_t F_compile(mpdm_t a) { return(mpsl_compile(A0)); }
static mpdm_t F_load(mpdm_t a) { return(mpdm_exec(mpsl_compile_file(A0), NULL)); }

static mpdm_t F_print(mpdm_t a)
{
	int n;

	for(n = 0;n < mpdm_size(a);n++)
		mpdm_write_wcs(stdout, mpdm_string(A(n)));
	return(NULL);
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
	{ L"unlink",	F_unlink },
	{ L"glob",	F_glob },
	{ L"encoding",	F_encoding },
	{ L"regex",	F_regex },
	{ L"sregex",	F_sregex },
	{ L"compile",	F_compile },
	{ L"load",	F_load },
	{ L"print",	F_print },
	{ NULL,		NULL }
};

void mpsl_lib(void)
/* inits the MPSL library */
{
	mpdm_t r;
	int n;

	r = mpdm_root();

	/* already done? */
	if(mpdm_hget_s(r, L"MPSL") != NULL)
		return;

	mpdm_hset_s(r, L"MPSL", MPDM_MBS(VERSION));

	/* creates all the symbols */
	for(n = 0;mpsl_funcs[n].name != NULL;n++)
		mpdm_hset_s(r, mpsl_funcs[n].name,
			MPDM_X(mpsl_funcs[n].func));

	/* creates INC, unless already defined */
	if(mpdm_hget_s(r, L"INC") == NULL)
		mpdm_hset_s(r, L"INC", MPDM_A(0));

	/* standard file descriptors */
	mpdm_hset_s(r, L"STDIN", MPDM_F(stdin));
	mpdm_hset_s(r, L"STDOUT", MPDM_F(stdout));
	mpdm_hset_s(r, L"STDERR", MPDM_F(stderr));
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

	for(n = 0;n < argc;n++)
		mpdm_push(ARGV, MPDM_MBS(argv[n]));

	mpdm_hset_s(mpdm_root(), L"ARGV", ARGV);
}
