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
 * $Source: r:/prj/lib/src/2d/RCS/ctxmac.h $
 * $Revision: 1.7 $
 * $Author: kevin $
 * $Date: 1994/11/16 12:24:37 $
 *
 * Macros for handling elements of the grs_context structure
 *
 * This file is part of the 2d library.
 */

#ifndef __CTXMAC_H
#define __CTXMAC_H
#include "cnvdat.h"
#include "lintab.h"
#include "fcntab.h"
#include "tabdat.h"

#define gr_init_gc(c) { (c)->gc=grd_defgc; \
      (c)->gc.clip.f.right=((c)->bm.w)<<16; \
      (c)->gc.clip.f.bot=((c)->bm.h)<<16; }

/* macros for setting the clipping region of the current canvas. */
#define gr_set_cliprect(l, t, r, b) \
   grd_clip.sten=NULL, \
   grd_clip.left=(l), grd_clip.top=(t), \
   grd_clip.right=(r), grd_clip.bot=(b)

#define gr_safe_set_cliprect(l, t, r, b)              \
   do {                                               \
      grd_clip.sten=NULL;                             \
      grd_clip.left=(((l)<0)?0:(l));                  \
      grd_clip.right=(((r)>grd_bm.w)?grd_bm.w:(r));   \
      grd_clip.top=(((t)<0)?0:(t));                   \
      grd_clip.bot=(((b)>grd_bm.h)?grd_bm.h:(b));     \
   } while (0)

#define gr_set_fix_cliprect(l, t, r, b) \
   grd_fix_clip.sten=NULL, \
   grd_fix_clip.left=(l), grd_fix_clip.top=(t), \
   grd_fix_clip.right=(r), grd_fix_clip.bot=(b)

/* macros for getting parts of the graphic context of the current canvas. */
#define gr_set_fcolor(color) (grd_canvas->gc.fcolor=color)
#define gr_get_fcolor() (grd_canvas->gc.fcolor)
#define gr_set_font(fnt) (grd_canvas->gc.font=fnt)
#define gr_get_font() (grd_canvas->gc.font)

/* this horrifying mess is necessary to ensure that grd_gc.fill_type is set
   _before_ the function tables.  Otherwise if an interrupt that uses the 2d
   occurs before grd_gc.fill_type is set, the function table ptrs may get out
   of sync when the interrupt tries to restore the canvas. */
// implementation of gri_set_fill_globals is in PixFill.c
extern void gri_set_fill_globals(long *fill_type_ptr, long fill_type,
                          void (***function_table_ptr)(), void (**function_table)(),
                          grt_uline_fill **line_vector_ptr, grt_uline_fill *line_vector);

#define gr_set_fill_type(__ft)  \
do {                            \
   long fill_type=__ft;         \
   gri_set_fill_globals(&(grd_canvas->gc.fill_type),fill_type,                      \
                        &grd_function_table,(*grd_function_fill_table)[fill_type],  \
                        &grd_uline_fill_vector,(*grd_uline_fill_table)[fill_type]); \
} while (0)
#define gr_get_fill_type() (grd_canvas->gc.fill_type)

#define gr_set_fill_parm(parm) \
   (grd_canvas->gc.fill_parm=(intptr_t)(parm))
#define gr_get_fill_parm() (grd_canvas->gc.fill_parm)

/* macros for setting the clipping region of a specified canvas. */
#define gr_cset_cliprect(c, l, t, r, b) \
   (c)->gc.clip.i.sten=NULL, \
   (c)->gc.clip.i.left=(l), (c)->gc.clip.i.top=t, \
   (c)->gc.clip.i.right=(r), (c)->gc.clip.i.bot=(b)
//KLC - changed (c)->gc.clip.i.sten->flags=NULL
#define gr_cset_fix_cliprect(c, l, t, r, b) \
   (c)->gc.clip.i.sten=NULL, \
   (c)->gc.clip.f.left=(l), (c)->gc.clip.f.top=(t), \
   (c)->gc.clip.f.right=(r), (c)->gc.clip.f.bot=(b)

/* macros for getting part of the clipping region of the current canvas. */
#define gr_get_cliprect(l,t,r,b) (*(l)=grd_clip.left,*(t)=grd_clip.top, \
   *(r)=grd_clip.right,*(b)=grd_clip.bot)
#define gr_get_clip_l() (grd_clip.left)
#define gr_get_clip_t() (grd_clip.top)
#define gr_get_clip_r() (grd_clip.right)
#define gr_get_clip_b() (grd_clip.bot)

#endif /* __CTXMAC_H */
