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
#ifndef __OLHEXT_H
#define __OLHEXT_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/olhext.h $
 * $Revision: 1.2 $
 * $Author: mahk $
 * $Date: 1994/07/15 21:31:57 $
 *
 */

extern uchar olh_overlay_on;

void olh_do_hudobjs(short xl, short yl);
void olh_overlay();
void olh_scan_objects(void);
void olh_init(void);
void olh_closedown(void);
void olh_shutdown(void);
uchar toggle_olh_func(ushort, uint32_t, intptr_t);
uchar olh_overlay_func(ushort keycode, uint32_t context, intptr_t);

#endif // __OLHEXT_H
