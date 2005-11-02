/*

    mpsl - Minimum Profit Scripting Language
    Copyright (C) 2003/2005 Angel Ortega <angel@triptico.com>

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
#include <wchar.h>
#include "mpdm.h"
#include "mpsl.h"

/*******************
	Code
********************/

int mpsl_main(int argc, char * argv[])
{
	if(argc == 1)
	{
		printf("mpsl %s - Minimum Profit Scripting Language\n", VERSION);
		printf("Copyright (C) 2003-2005 Angel Ortega <angel@triptico.com>\n");
		printf("This software is covered by the GPL license. NO WARRANTY.\n\n");

		printf("Usage: mpsl {script.mpsl}\n\n");

		return(0);
	}

	mpdm_startup();

	mpsl_argv(argc, argv);

	mpdm_shutdown();

	return(0);
}


int main(int argc, char * argv[])
{
	return(mpsl_main(argc, argv));
}
