/*

    Copyright (C) 2003/2006 Angel Ortega <angel@triptico.com>

    mpsl.h - Minimum Profit Scripting Language

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

extern int mpsl_trace;
extern int mpsl_abort;

int mpsl_is_true(mpdm_t v);
mpdm_t mpsl_boolean(int b);

mpdm_t mpsl_set_symbol(mpdm_t s, mpdm_t v);
mpdm_t mpsl_get_symbol(mpdm_t s);

mpdm_t mpsl_error(mpdm_t err);

mpdm_t mpsl_exec(mpdm_t c, mpdm_t args);

mpdm_t mpsl_compile(mpdm_t code);
mpdm_t mpsl_compile_file(mpdm_t filename);
mpdm_t mpsl_eval(mpdm_t code, mpdm_t args);

void mpsl_argv(int argc, char * argv[]);
