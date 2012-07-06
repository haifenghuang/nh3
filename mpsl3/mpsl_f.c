/*

    MPSL - Minimum Profit Scripting Language 3.x
    Copyright (C) 2003/2012 Angel Ortega <angel@triptico.com>

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

    http://triptico.com

*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <time.h>

#include "mpsl.h"

/** code **/

#define F_ARGS mpdm_t a, mpdm_t l

#define A(n) mpdm_aget(a, n)
#define A0 A(0)
#define A1 A(1)
#define A2 A(2)
#define IA(n) mpdm_ival(A(n))
#define IA0 IA(0)
#define IA1 IA(1)
#define IA2 IA(2)
#define IA3 IA(3)
#define RA(n) mpdm_rval(A(n))
#define RA0 RA(0)
#define RA1 RA(1)
#define RA2 RA(2)
#define RA3 RA(3)


/** library **/

/**
 * print - Writes values to stdout.
 * @arg1: first argument
 * @arg2: second argument
 * @argn: nth argument
 *
 * Writes the variable arguments to stdout.
 * [Input-Output]
 */
/** print(arg1 [,arg2 ... argn]); */
static mpdm_t F_print(F_ARGS)
{
    int n;

    for (n = 0; n < mpdm_size(a); n++)
        mpdm_write_wcs(stdout, mpdm_string(A(n)));
    return NULL;
}


/** init **/

void mpsl_library_init(mpdm_t r)
/* inits the library */
{
    mpdm_ref(r);

    mpdm_hset_s(r, L"print",    MPDM_X(F_print));

    mpdm_unref(r);
}
