/*

    MPSL - Minimum Profit Scripting Language
    Copyright (C) 2003/2006 Angel Ortega <angel@triptico.com>

    mpsl_m.c - Minimum Profit Scripting Language main()

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
#include "mpdm.h"
#include "mpsl.h"

/*******************
	Code
********************/

int mpsl_main(int argc, char * argv[])
{
	mpdm_t v;
	char * script = NULL;
	char * immscript = NULL;
	int ret = 0;

	if(argc == 1)
	{
		printf("MPSL %s - Minimum Profit Scripting Language\n", VERSION);
		printf("Copyright (C) 2003-2006 Angel Ortega <angel@triptico.com>\n");
		printf("This software is covered by the GPL license. NO WARRANTY.\n\n");

		printf("Usage: mpsl [-e 'script' | script.mpsl ]\n\n");

		return(0);
	}

	mpdm_startup();

	/* skip the executable */
	argv++;	argc--;

	if(strcmp(argv[0], "-e") == 0)
	{
		argv++; argc--;

		immscript = argv[0];
	}
	else
	{
		/* get the script name */
		script = argv[0];
	}

	/* set arguments */
	mpsl_argv(argc, argv);

	/* add '.' to INC */
	v = MPDM_A(1);
	mpdm_aset(v, MPDM_LS(L"."), 0);
	mpdm_hset_s(mpdm_root(), L"INC", v);

	/* compile */
	if(immscript != NULL)
		v = mpsl_compile(MPDM_MBS(immscript));
	else
		v = mpsl_compile_file(MPDM_MBS(script));

	/* execute, if possible */
	if(v != NULL)
		mpdm_exec(v, NULL);

	/* prints the error, if any */
	if((v = mpsl_error(NULL)) != NULL)
	{
		mpdm_write_wcs(stderr, mpdm_string(v));
		fprintf(stderr, "\n");

		ret = 1;
	}

	mpdm_shutdown();

	return(ret);
}


int main(int argc, char * argv[])
{
	return(mpsl_main(argc, argv));
}
