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

#include "2d.h"

// externs for functions in FrUtils.C

extern void Fast_Slot_Copy(grs_bitmap *bm);
extern void Fast_FullScreen_Copy(grs_bitmap *bm);
// extern void Fast_Slot_Double(grs_bitmap *bm, long w, long h);
// extern void Fast_FullScreen_Double(grs_bitmap *bm, long w, long h);

void FastSlotDouble2Canvas(grs_bitmap *bm, grs_canvas *destCanvas, long w, long h);
void FastFullscreenDouble2Canvas(grs_bitmap *bm, grs_canvas *destCanvas, long w, long h);

// Stuff for the low-res temporary offscreen buffer.
extern grs_canvas gDoubleSizeOffCanvas;

// int AllocDoubleBuffer(int w, int h);
// void FreeDoubleBuffer(void);
