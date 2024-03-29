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
 * $Source: r:/prj/lib/src/2d/RCS/canvas.c $
 * $Revision: 1.21 $
 * $Author: kevin $
 * $Date: 1994/11/28 21:17:01 $
 *
 * Canvas handling routines.
 *
 */

#include "grs.h"
#include "bitmap.h"
#include "chain.h"
#include "cnvdat.h"
#include "cnvtab.h"
#include "context.h"
#include "ctxmac.h"
#include "fcntab.h"
#include "lintab.h"
#include "tabdat.h"

#define CANVAS_STACKSIZE 16
grs_canvas *grd_canvas_stack[CANVAS_STACKSIZE];
int grd_canvas_stackp = 0;

/* set current canvas to c. select driver_func from type of bitmap
   attached to canvas. */
void gr_set_canvas(grs_canvas *c) {
    int i;

    if (c == NULL)
        return;

    grd_canvas = c;
    if (gr_generic)
        i = BMT_GEN;
    else
        i = c->bm.type;

    grd_pixel_index = c->bm.type;
    grd_pixel_table = grd_canvas_table_list[grd_pixel_index];
    grd_canvas_table = grd_canvas_table_list[i];

    grd_uline_fill_table = grd_uline_fill_table_list[c->bm.type];
    grd_uline_fill_vector = (*grd_uline_fill_table)[c->gc.fill_type];
    grd_function_fill_table = grd_function_table_list[i];
    grd_function_table = (*grd_function_fill_table)[c->gc.fill_type];
}

/* push current canvas onto canvas stack and make passed in canvas active.
   returns 0 if stack is ok, -1 if there is an overflow. */
int gr_push_canvas(grs_canvas *c) {
    if (grd_canvas_stackp >= CANVAS_STACKSIZE)
        return -1;
    grd_canvas_stack[grd_canvas_stackp++] = grd_canvas;
    gr_set_canvas(c);
    return 0;
}

/* pop last canvas off of stack and make it active.  return it, or NULL if
   there is an underflow. */
grs_canvas *gr_pop_canvas(void) {
    grs_canvas *c;
    if (grd_canvas_stackp <= 0)
        return NULL;
    c = grd_canvas_stack[--grd_canvas_stackp];
    gr_set_canvas(c);
    return c;
}

#pragma scheduling off
#pragma global_optimizer off

void gr_init_canvas(grs_canvas *c, uchar *p, int type, short w, short h) {
#ifdef GR_DOUBLE_CANVAS
    if (type == BMT_FLAT8_DOUBLE) {
        gr_init_bitmap(&c->bm, p, BMT_FLAT8, 0, w, h);
        gr_init_gc(c);
        gr_cset_fix_cliprect(c, 0, 0, fix_make((w >> 1), 0), fix_make(h, 0));
        c->bm.type = BMT_FLAT8_DOUBLE;
    } else
#endif
    {
        gr_init_bitmap(&c->bm, p, type, 0, w, h);
        gr_init_gc(c);
        gr_cset_fix_cliprect(c, 0, 0, fix_make(w, 0), fix_make(h, 0));
    }
    c->ytab = NULL;
}

#pragma scheduling reset
#pragma global_optimizer reset

void gr_init_sub_canvas(grs_canvas *sc, grs_canvas *dc, short x, short y, short w, short h) {
    gr_init_sub_bitmap(&sc->bm, &dc->bm, x, y, w, h);
    gr_init_gc(dc);
    gr_cset_fix_cliprect(dc, 0, 0, fix_make(w, 0), fix_make(h, 0));
    dc->ytab = NULL;
}

void gr_make_canvas(grs_bitmap *bm, grs_canvas *c) { gr_init_canvas(c, bm->bits, bm->type, bm->w, bm->h); }
