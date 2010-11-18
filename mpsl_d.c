/*

    MPSL - Minimum Profit Scripting Language
    Copyright (C) 2003/2009 Angel Ortega <angel@triptico.com>

    mpsl_d.c - Minimum Profit Scripting Language debugging functions

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
#include <string.h>
#include <wchar.h>
#include <malloc.h>

#include "mpdm.h"
#include "mpsl.h"

/** code **/

static wchar_t *dump_string(const mpdm_t v, wchar_t *ptr, int *size)
/* dumps a string, escaping special chars */
{
	wchar_t *iptr = mpdm_string(v);

	ptr = mpdm_pokews(ptr, size, L"\"");

	while (*iptr != L'\0') {
		switch (*iptr) {
		case '"':
			ptr = mpdm_pokews(ptr, size, L"\\\"");
			break;

		case '\'':
			ptr = mpdm_pokews(ptr, size, L"\\'");
			break;

		case '\r':
			ptr = mpdm_pokews(ptr, size, L"\\r");
			break;

		case '\n':
			ptr = mpdm_pokews(ptr, size, L"\\n");
			break;

		case '\t':
			ptr = mpdm_pokews(ptr, size, L"\\t");
			break;

		case '\\':
			ptr = mpdm_pokews(ptr, size, L"\\\\");
			break;

		default:
			if (*iptr > 127) {
				char tmp[16];
				wchar_t *wptr;

				sprintf(tmp, "\\x{%04x}", *iptr);
				wptr = mpdm_mbstowcs(tmp, NULL, -1);
				ptr = mpdm_pokews(ptr, size, wptr);
				free(wptr);
			}
			else
				ptr = mpdm_poke(ptr, size, iptr, 1, sizeof(wchar_t));

			break;
		}
		iptr++;
	}

	ptr = mpdm_pokews(ptr, size, L"\"");

	return ptr;
}


wchar_t *mpsl_dump_1(const mpdm_t v, int l, wchar_t *ptr, int *size)
/* dump plugin for mpdm_dump() */
{
	int n;
	char tmp[256];
	wchar_t *wptr;


	/* indent (if negative, don't prepend indentation) */
	if (l < 0)
		l = -l;
	else
	for (n = 0; n < l; n++)
		ptr = mpdm_pokews(ptr, size, L"  ");

	if (v == NULL)
		ptr = mpdm_pokews(ptr, size, L"NULL");
	else
	if (MPDM_IS_HASH(v)) {
		mpdm_t k;
		mpdm_t w;
		int c;

		c = n = 0;

		ptr = mpdm_pokews(ptr, size, L"{\n");

		while (mpdm_iterator(v, &c, &k, &w)) {
			if (n++)
				ptr = mpdm_pokews(ptr, size, L",\n");

			ptr = mpsl_dump_1(k, l + 1, ptr, size);
			ptr = mpdm_pokews(ptr, size, L" => ");
			ptr = mpsl_dump_1(w, -(l + 1), ptr, size);
		}

		ptr = mpdm_pokews(ptr, size, L"\n");

		/* re-indent */
		for (n = 0; n < l; n++)
			ptr = mpdm_pokews(ptr, size, L"  ");

		ptr = mpdm_pokews(ptr, size, L"}");
	}
	else
	if (MPDM_IS_ARRAY(v)) {
		ptr = mpdm_pokews(ptr, size, L"[\n");

		for (n = 0; n < mpdm_size(v); n++) {
			ptr = mpsl_dump_1(mpdm_aget(v, n), l + 1, ptr, size);

			if (n < mpdm_size(v) - 1)
				ptr = mpdm_pokews(ptr, size, L",");

			ptr = mpdm_pokews(ptr, size, L"\n");
		}

		/* re-indent */
		for (n = 0; n < l; n++)
			ptr = mpdm_pokews(ptr, size, L"  ");

		ptr = mpdm_pokews(ptr, size, L"]");
	}
	else
	if (MPDM_IS_EXEC(v)) {
		snprintf(tmp, sizeof(tmp), "bincall(%p)", v->data);
		wptr = mpdm_mbstowcs(tmp, NULL, -1);
		ptr = mpdm_pokews(ptr, size, wptr);
		free(wptr);
	}
	else
	if (MPDM_IS_STRING(v))
		ptr = dump_string(v, ptr, size);
	else
	if (v->flags & MPDM_FILE) {
		FILE *f;

		f = mpdm_get_filehandle(v);

		if (f != NULL)
			snprintf(tmp, sizeof(tmp), "NULL /* file descriptor: %d */", fileno(f));
		else
			strcpy(tmp, "NULL /* file descriptor (unknown handle) */");

		wptr = mpdm_mbstowcs(tmp, NULL, -1);
		ptr = mpdm_pokews(ptr, size, wptr);
		free(wptr);
	}
	else
	if (v->flags & MPDM_REGEX)
		ptr = mpdm_pokews(ptr, size, L"NULL /* compiled regular expression */");
	else {
		snprintf(tmp, sizeof(tmp), "NULL /* non-printable value (flags: 0x%04x) */", v->flags);
		wptr = mpdm_mbstowcs(tmp, NULL, -1);
		ptr = mpdm_pokews(ptr, size, wptr);
		free(wptr);
	}

	if (l == 0)
		ptr = mpdm_pokews(ptr, size, L";\n");

	return ptr;
}
