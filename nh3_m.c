/*

    nh3 - A Programming Language
    Copyright (C) 2003/2013 Angel Ortega <angel@triptico.com>

    nh3_m.c - main()

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

#include "nh3.h"


void nh3_disasm(mpdm_t);
mpdm_t nh3_asm(mpdm_t);


/** code **/

int nh3_main(int argc, char *argv[])
{
    mpdm_t v = NULL;
    mpdm_t w = NULL;
    char *immscript = NULL;
    FILE *script = stdin;
    int ret = 0;
    int ok = 0;
    int disasm = 0;
    int enasm = 0;

    /* skip the executable */
    argv++;
    argc--;

    while (!ok && argc > 0) {
        if (strcmp(argv[0], "-v") == 0 || strcmp(argv[0], "--help") == 0) {
            printf("nh3 %s - A Programming Language\n",
                   VERSION);
            printf("Copyright (C) 2003-2013 Angel Ortega <angel@triptico.com>\n");
            printf("This software is covered by the GPL license. NO WARRANTY.\n\n");

            printf("Usage: nh3 [-d] [-a] [-e 'script' | script.nh3 ]\n\n");

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
        else
        if (strcmp(argv[0], "-a") == 0)
            enasm = 1;
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

    nh3_startup(argc, argv);

    /* compile */
    if (immscript != NULL) {
        w = mpdm_ref(MPDM_MBS(immscript));
        v = nh3_compile(w);
        mpdm_unref(w);
    }
    else {
        if (enasm) {
            v = nh3_asm(MPDM_F(script));
        }
        else {
            w = mpdm_ref(MPDM_F(script));
            v = nh3_compile(w);
            mpdm_close(w);
            mpdm_unref(w);
        }
    }

    if (v != NULL) {
        mpdm_ref(v);

        if (disasm)
            nh3_disasm(mpdm_aget(v, 1));
        else {
            int r = mpdm_ival(mpdm_exec(v, NULL, NULL));

/*            if (r == VM_ERROR)
                printf("ERROR:\n");*/
        }

        mpdm_unref(v);
    }

    /* prints the error, if any */
    if ((w = mpdm_hget_s(mpdm_root(), L"ERROR")) != NULL) {
        FILE *f = stderr;

        /* if it's a CGI, dump error to stdout instead of stderr */
        if (mpdm_hget_s(
                mpdm_hget_s(mpdm_root(), L"ENV"),
                L"GATEWAY_INTERFACE"
            ) != NULL) {
            printf("Content-type: text/plain\r\n\r\n");
            f = stdout;
        }

        mpdm_write_wcs(f, mpdm_string(w));
        fprintf(f, "\n");

        ret = 1;
    }

    nh3_shutdown();

    return ret;
}


int main(int argc, char *argv[])
{
    return nh3_main(argc, argv);
}
