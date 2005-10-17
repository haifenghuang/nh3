/*

    mpsl - Minimum Profit Scripting Language
    Copyright (C) 2003/2005 Angel Ortega <angel@triptico.com>

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

static mpdm_t F_is_array(mpdm_t a) { return(mpsl_boolean(A0->flags & MPDM_MULTIPLE)); }
static mpdm_t F_is_hash(mpdm_t a) { return(mpsl_boolean(A0->flags & MPDM_HASH)); }
static mpdm_t F_is_exec(mpdm_t a) { return(mpsl_boolean(A0->flags & MPDM_EXEC)); }

static mpdm_t F_splice(mpdm_t a) { return(mpdm_splice(A0,A1,IA2,IA3)); }
static mpdm_t F_aexpand(mpdm_t a) { return(mpdm_aexpand(A0,IA1,IA2)); }
static mpdm_t F_acollapse(mpdm_t a) { return(mpdm_acollapse(A0,IA1,IA2)); }
static mpdm_t F_ains(mpdm_t a) { return(mpdm_ains(A0,A1,IA2)); }
static mpdm_t F_adel(mpdm_t a) { return(mpdm_adel(A0,IA1)); }
static mpdm_t F_apush(mpdm_t a) { return(mpdm_apush(A0,A1)); }
static mpdm_t F_apop(mpdm_t a) { return(mpdm_apop(A0)); }
static mpdm_t F_aqueue(mpdm_t a) { return(mpdm_aqueue(A0,A1,IA2)); }
static mpdm_t F_aseek(mpdm_t a) { return(MPDM_I(mpdm_aseek(A0,A1,IA2))); }
static mpdm_t F_asort(mpdm_t a) { return(mpdm_asort_cb(A0,1,A1)); }
static mpdm_t F_asplit(mpdm_t a) { return(mpdm_asplit(A0,A1)); }
static mpdm_t F_ajoin(mpdm_t a) { return(mpdm_ajoin(A0,A1)); }

static mpdm_t F_hsize(mpdm_t a) { return(MPDM_I(mpdm_hsize(A0))); }
static mpdm_t F_hexists(mpdm_t a) { return(mpsl_boolean(mpdm_hexists(A0, A1))); }
static mpdm_t F_hdel(mpdm_t a) { return(mpdm_hdel(A0, A1)); }
static mpdm_t F_hkeys(mpdm_t a) { return(mpdm_hkeys(A0)); }

static mpdm_t F_open(mpdm_t a) { return(mpdm_open(A0, A1)); }
static mpdm_t F_close(mpdm_t a) { return(mpdm_close(A0)); }
static mpdm_t F_read(mpdm_t a) { return(mpdm_read(A0)); }
static mpdm_t F_write(mpdm_t a) { return(mpsl_boolean(mpdm_write(A0,A1))); }
static mpdm_t F_unlink(mpdm_t a) { return(mpsl_boolean(mpdm_unlink(A0))); }
static mpdm_t F_glob(mpdm_t a) { return(mpdm_glob(A0)); }
static mpdm_t F_encoding(mpdm_t a) { return(MPDM_I(mpdm_encoding(A0))); }

static mpdm_t F_regex(mpdm_t a) { return(mpdm_regex(A0,A1,IA2)); }
static mpdm_t F_sregex(mpdm_t a) { return(mpdm_sregex(A0,A1,A2,IA3)); }

static mpdm_t F_print(mpdm_t a)
{
	int n;

	for(n=0;n < mpdm_size(a);n++)
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
	{ L"aexpand",	F_aexpand },
	{ L"acollapse",	F_acollapse },
	{ L"ains",	F_ains },
	{ L"adel",	F_adel },
	{ L"apush",	F_apush },
	{ L"apop",	F_apop },
	{ L"aqueue",	F_aqueue },
	{ L"aseek",	F_aseek },
	{ L"asort",	F_asort },
	{ L"asplit",	F_asplit },
	{ L"ajoin",	F_ajoin },
	{ L"hsize",	F_hsize },
	{ L"hexists",	F_hexists },
	{ L"hdel",	F_hdel },
	{ L"hkeys",	F_hkeys },
	{ L"open",	F_open },
	{ L"close",	F_close },
	{ L"read",	F_read },
	{ L"write",	F_write },
	{ L"unlink",	F_unlink },
	{ L"glob",	F_glob },
	{ L"encoding",	F_encoding },
	{ L"regex",	F_regex },
	{ L"sregex",	F_sregex },
	{ L"print",	F_print },
	{ NULL,		NULL }
};

void mpsl_lib(void)
/* inits the mpsl library */
{
	mpdm_t r;
	int n;

	r=mpdm_root();

	/* already done? */
	if(mpdm_hget_s(r, L"MPSL_LIB") != NULL)
		return;

	mpdm_hset_s(r, L"MPSL_LIB", mpsl_boolean(1));

	/* creates all the symbols */
	for(n=0;mpsl_funcs[n].name != NULL;n++)
		mpdm_hset_s(r, mpsl_funcs[n].name,
			MPDM_X(mpsl_funcs[n].func));
}
