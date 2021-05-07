/*

Copyright (C) 2021 ShockPlus Project

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

#ifndef __CURDRW
#define __CURDRW

#include "cursors.h"

extern void cursor_draw_callback(ss_mouse_event *e, void *data);
extern void bitmap_cursor_drawfunc(int cmd, LGRegion *r, LGCursor *c, LGPoint pos);

#endif
