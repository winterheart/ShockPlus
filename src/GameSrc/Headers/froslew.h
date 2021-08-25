/*

Copyright (C) 2015-2018 Night Dive Studios, LLC.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
/*
 * FrOslew.h
 *
 * $Source: n:/project/cit/src/inc/RCS/froslew.h $
 * $Revision: 1.2 $
 * $Author: dc $
 * $Date: 1994/01/02 17:16:29 $
 *
 * Citadel Renderer
 *  object slew system controllers/prototypes/vars
 *
 * $Log: froslew.h $
 * Revision 1.2  1994/01/02  17:16:29  dc
 * Initial revision
 *
 * Revision 1.1  1993/09/05  20:59:07  dc
 * Initial revision
 *
 */

#ifndef __FROSLEW_H
#define __FROSLEW_H

#ifndef __RENDTEST__
#include "objects.h"
#else
//¥¥#include <rtestobj.h>
#endif

uchar fr_objslew_allowed(Obj *cobj, int32_t *eye);
uchar fr_objslew_moveone(Obj *objp, ObjID objnum, int which, int how, uchar conform);
uchar fr_objslew_go_real_height(Obj *cobj, int32_t *eye);

extern int32_t eye_mods[3];

#endif
