/*

    MPSL - Minimum Profit Scripting Language
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
#include "mpdm.h"
#include "mpsl.h"


/** code **/

mpdm_t trap_func(mpdm_t args, mpdm_t ctxt)
{
    mpdm_t c = mpdm_aget(args, 0);
    mpdm_t r = mpdm_aget(args, 2);

    printf("-- Code ---------------\n");
    mpdm_dump(c);
    printf("-- Ret  ---------------\n");
    mpdm_dump(r);

    printf("Press ENTER\n");
    getchar();

    return NULL;
}


int mpsl_main(int argc, char *argv[])
{
    mpdm_t v = NULL;
    mpdm_t w = NULL;
    char *immscript = NULL;
    FILE *script = stdin;
    int ret = 0;
    int dump_only = 0;
    int install_trap = 0;

    /* skip the executable */
    argv++;
    argc--;

    while (argc > 0) {
        if (strcmp(argv[0], "-v") == 0 || strcmp(argv[0], "--help") == 0) {
            printf("MPSL %s - Minimum Profit Scripting Language\n",
                   VERSION);
            printf("Copyright (C) 2003-2010 Angel Ortega <angel@triptico.com>\n");
            printf("This software is covered by the GPL license. NO WARRANTY.\n\n");

            printf("Usage: mpsl [-d | -s] [-e 'script' | script.mpsl ]\n\n");

            return 0;
        }
        else
        if (strcmp(argv[0], "-d") == 0)
            dump_only = 1;
        else
        if (strcmp(argv[0], "-s") == 0)
            install_trap = 1;
        else
        if (strcmp(argv[0], "-e") == 0) {
            argv++;
            argc--;
            immscript = argv[0];
        }
        else {
            /* next argument is a script name; open it */
            if ((script = fopen(argv[0], "r")) == NULL) {
                fprintf(stderr, "Can't open '%s'\n", argv[0]);
                return 1;
            }
        }

        argv++;
        argc--;
    }

    mpsl_startup();

    /* set arguments */
    mpsl_argv(argc, argv);

    if (install_trap)
        mpsl_trap(MPDM_X(trap_func));

    /* compile */
    if (immscript != NULL) {
        w = mpdm_ref(MPDM_MBS(immscript));
        v = mpsl_compile(w);
        w = mpdm_unref(w);
    }
    else {
        int c;

        /* if line starts with #!, discard it */
        if ((c = getc(script)) == '#' && (c = getc(script)) == '!')
            while ((c = getc(script)) != EOF && c != '\n');
        else
            ungetc(c, script);

        if (c != EOF) {
            w = mpdm_ref(MPDM_F(script));
            v = mpsl_compile_file(w, NULL);
            mpdm_close(w);
            w = mpdm_unref(w);
        }
    }

    if (v != NULL) {
        mpdm_ref(v);

        if (dump_only)
            mpdm_dump(v);
        else
            mpdm_void(mpdm_exec(v, NULL, NULL));

        mpdm_unref(v);
    }

    /* prints the error, if any */
    if ((w = mpdm_hget_s(mpdm_root(), L"ERROR")) != NULL) {
        mpdm_write_wcs(stderr, mpdm_string(w));
        fprintf(stderr, "\n");

        ret = 1;
    }

    mpsl_shutdown();

    return ret;
}


int main(int argc, char *argv[])
{
    return mpsl_main(argc, argv);
}
