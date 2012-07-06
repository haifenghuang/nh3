/*

    MPSL - Minimum Profit Scripting Language 3.x
    Copyright (C) 2003/2012 Angel Ortega <angel@triptico.com>

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

#ifndef MPSL_H_
#define MPSL_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "mpdm.h"

enum {
    VM_IDLE, VM_RUNNING, VM_TIMEOUT, VM_ERROR
};

mpdm_t mpsl_compile(mpdm_t src);

void mpsl_startup(int argc, char *argv[]);
void mpsl_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* MPSL_H_ */
