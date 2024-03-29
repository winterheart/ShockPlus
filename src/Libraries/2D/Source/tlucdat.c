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
 * $Source: n:/project/lib/src/2d/RCS/tlucdat.c $
 * $Revision: 1.3 $
 * $Author: baf $
 * $Date: 1994/01/17 22:13:14 $
 *
 * Globals for translucency.
 *
 * This file is part of the 2d library.
 *
 * $Log: tlucdat.c $
 * Revision 1.3  1994/01/17  22:13:14  baf
 * Redid tluc8 spolys (again).
 * 
 * Revision 1.2  1994/01/14  12:41:07  baf
 * Lit translucency reform.
 * 
 * Revision 1.1  1993/12/01  21:20:05  baf
 * Initial revision
 * 
 *
 */

#include "lg.h"

uchar *tluc8tab[256];
uchar *tluc8stab;
int tluc8nstab = 0;
