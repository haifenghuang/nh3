/*

    MPSL - Minimum Profit Scripting Language 3.x
    Copyright (C) 2003/2010 Angel Ortega <angel@triptico.com>

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

#include "mpsl.h"


void mpsl_disasm(mpdm_t);

/** code **/

int mpsl_main(int argc, char *argv[])
{
    mpdm_t v = NULL;
    mpdm_t w = NULL;
    char *immscript = NULL;
    FILE *script = stdin;
    int ret = 0;
    int ok = 0;
    int disasm = 0;

    /* skip the executable */
    argv++;
    argc--;

    while (!ok && argc > 0) {
        if (strcmp(argv[0], "-v") == 0 || strcmp(argv[0], "--help") == 0) {
            printf("MPSL %s - Minimum Profit Scripting Language\n",
                   VERSION);
            printf("Copyright (C) 2003-2012 Angel Ortega <angel@triptico.com>\n");
            printf("This software is covered by the GPL license. NO WARRANTY.\n\n");

            printf("Usage: mpsl [-d] [-e 'script' | script.mpsl ]\n\n");

            return 0;
        }
        else
        if (strcmp(argv[0], "-d") == 0)
            disasm = 1;
        else
        if (strcmp(argv[0], "-e") == 0) {
            argv++;
            argc--;
            immscript = argv[0];
            ok = 1;
        }
        else {
            /* next argument is a script name; open it */
            if ((script = fopen(argv[0], "r")) == NULL) {
                fprintf(stderr, "Can't open '%s'\n", argv[0]);
                return 1;
            }
            ok = 1;
        }

        argv++;
        argc--;
    }

    mpdm_startup();

    /* set arguments */
//    mpsl_argv(argc, argv);

    /* compile */
    if (immscript != NULL) {
        w = mpdm_ref(MPDM_MBS(immscript));
        v = mpsl_compile(w);
        mpdm_unref(w);
    }
    else {
        int c = 0;

        /* if line starts with #!, discard it */
/*        if ((c = getc(script)) == '#' && (c = getc(script)) == '!')
            while ((c = getc(script)) != EOF && c != '\n');
        else
            ungetc(c, script);
*/
        if (c != EOF) {
            w = mpdm_ref(MPDM_F(script));
            v = mpsl_compile(w);
            mpdm_close(w);
            mpdm_unref(w);
        }
    }

    if (v != NULL) {
        mpdm_ref(v);

        if (disasm)
            mpsl_disasm(mpdm_aget(v, 1));
        else {
            int r = mpdm_ival(mpdm_exec(v, NULL, NULL));

            if (r == VM_ERROR)
                printf("ERROR:\n");
        }

        mpdm_unref(v);
    }

    /* prints the error, if any */
    if ((w = mpdm_hget_s(mpdm_root(), L"ERROR")) != NULL) {
        mpdm_write_wcs(stderr, mpdm_string(w));
        fprintf(stderr, "\n");

        ret = 1;
    }

//    mpsl_shutdown();

    return ret;
}


int main(int argc, char *argv[])
{
    return mpsl_main(argc, argv);
}
