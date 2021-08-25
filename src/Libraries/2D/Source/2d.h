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
#ifndef __2D_H
#define __2D_H

#if defined(__cplusplus)
extern "C" {
#endif // !defined(__cplusplus)

#include "bitmap.h"
#include "blend.h"
#include "canvas.h"
#include "chr.h"
#include "clpltyp.h"
#include "cnvdat.h"
#include "context.h"
#include "ctxmac.h"
#include "detect.h"
#include "fill.h"
#include "fix.h"
#include "grcbm.h"
#include "grcply.h"
#include "grd.h"
#include "grdbm.h"
#include "grgbm.h"
#include "grlin.h"
#include "grpix.h"
#include "grply.h"
#include "grrect.h"
#include "grrend.h"
#include "grs.h"
#include "icanvas.h"
#include "ifcn.h"
#include "init_2d.h"
#include "fcntab.h"
#include "line.h"
#include "lintab.h"
#include "lintyp.h"
#include "mode.h"
#include "pal.h"
#include "pertol.h"
#include "pertyp.h"
#include "pixfill.h"
#include "plytyp.h"
#include "rsdunpck.h"
#include "scrdat.h"
#include "screen.h"
#include "scrmac.h"
#include "str.h"
#include "tabdat.h"
#include "tabdrv.h"
#include "tlucdat.h"
#include "tluctab.h"
#include "tmaps.h"

#pragma pack(push, 2)

#define gr_int_circle ((int (*)(short x, short y, short r))grd_canvas_table[INT_CIRCLE])
#define gr_int_disk ((int (*)(short x, short y, short r))grd_canvas_table[INT_DISK])

extern int gr_int_line(short x0, short y0, short x1, short y1);

#pragma pack(pop)

#if defined(__cplusplus)
}
#endif // !defined(__cplusplus)

#endif /* __2D_H */
