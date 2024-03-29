/*

Copyright (C) 2015-2018 Night Dive Studios, LLC.
Copyright (C) 2018-2020 Shockolate Project

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
/* $Source: n:/project/lib/src/2d/RCS/strscl.c $
 * $Revision: 1.2 $
 * $Author: lmfeeney $
 * $Date: 1994/06/15 01:17:23 $
 */

/* scale each character as a separate bitmap: use std scaling calculation
   to set the ul of each (next) pixmap and set w/h for bitmap scaling
   routine

   this is hugely inefficient, like the old unscaled string routines
   it clips each constituent bitmap

 */

#include "bitmap.h"
#include "clip.h"
#include "grdbm.h"
#include "grrend.h"
#include "str.h"
#include "chr.h"

#include <stdio.h> // printf()

int gen_font_scale_string(grs_font *f, char *s, short x0, short y0, short w, short h) {
    grs_bitmap bm;             /* character bitmap */
    short *offset_tab;         /* table of character offsets */
    uchar *char_buf;           /* font pixel data */
    short offset;              /* offset of current character */
    short str_w, str_h;        /* width and height of src string */
    fix x, y;                  /* position of current character */
    fix x_scale, y_scale;      /* x and y scale factors */
    fix next_x, next_y, del_y; /* need to use next, del_y since it's const */
    int i;
    uchar c; /* current character */

    if (w == 0 || h == 0) {
        return CLIP_NONE;
    }

    char_buf = (uchar *)f + f->buf;
    offset_tab = (short *)f->off_tab;
    gr_init_bitmap(&bm, NULL, (f->id == 0xcccc) ? BMT_FLAT8 : BMT_MONO, BMF_TRANS, 0, f->h);
    bm.row = f->w;

    gr_font_string_size(f, s, &str_w, &str_h);

    x_scale = (w << 16) / str_w;
    y_scale = (h << 16) / str_h;

    x = x0 << 16;
    y = y0 << 16;

    for (i = 0, del_y = 0; i < f->h; del_y += y_scale, i++)
        ; /* multiply fix by int, faster ?? */
    next_y = y + del_y;

    while ((c = (uchar)(*s++)) != '\0') {
        if (c == '\n' || c == CHAR_SOFTCR) {
            x = x0 << 16;
            y = next_y;
            next_y = y + del_y;
            continue;
        }
        if (c > f->max || c < f->min || c == CHAR_SOFTSP)
            continue;
        offset = offset_tab[c - f->min];
        bm.w = offset_tab[c - f->min + 1] - offset;

        for (i = 0, next_x = x; i < bm.w; next_x += x_scale, i++)
            ; /* multiply fix by int, faster ?? */

        if (bm.type == BMT_MONO) {
            bm.bits = char_buf + (offset >> 3);
            bm.align = offset & 7;
            gr_scale_bitmap(&bm, fix_int(x), fix_int(y), fix_int(next_x) - fix_int(x), fix_int(next_y) - fix_int(y));

        } else {
            bm.bits = char_buf + offset;
            gr_scale_bitmap(&bm, fix_int(x), fix_int(y), fix_int(next_x) - fix_int(x), fix_int(next_y) - fix_int(y));
        }
        x = next_x;
    }
    return CLIP_NONE;
}
