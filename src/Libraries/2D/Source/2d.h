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
#include "fix.h"
#ifndef __2D_H
#define __2D_H

#include "GR/grs.h"
#include "plytyp.h"
#include "tmaps.h"

#if defined(__cplusplus)
extern "C" {
#endif // !defined(__cplusplus)

#include "tlucdat.h"
#include "tluctab.h"

#pragma pack(push, 2)

typedef struct {
    grs_point3d normal;
    grs_point3d u_grad;
    grs_point3d v_grad;
} grs_per_context;

// grd.h
extern grs_sys_info grd_info;
extern grs_drvcap *grd_cap;
extern grs_drvcap grd_mode_cap;
extern int grd_mode;
#define grd_scr_canv grd_screen_canvas

extern grs_screen *grd_screen;
extern uchar grd_default_pal[];
extern uchar *grd_pal;
extern grs_rgb grd_default_bpal[];
extern grs_rgb *grd_bpal;
extern uchar *grd_ipal;
extern grs_canvas *grd_screen_canvas;
extern grs_canvas *grd_visible_canvas;
extern grs_canvas *grd_canvas;
#define grd_bm (grd_canvas->bm)
#define grd_gc (grd_canvas->gc)
#define grd_ytab (grd_canvas->ytab)
#define grd_int_clip (grd_gc.clip.i)
#define grd_fix_clip (grd_gc.clip.f)
#define grd_clip (grd_int_clip)
extern void (**grd_pixel_table)();
extern void (**grd_device_table)();
extern void (**grd_canvas_table)();
extern void (**grd_function_table)();
enum { BMT_DEVICE, BMT_MONO, BMT_FLAT8, BMT_FLAT24, BMT_RSD8, BMT_TLUC8, BMT_SPAN, BMT_GEN, BMT_TYPES };
#define REAL_BMT_TYPES BMT_SPAN
#define BMF_TRANS 1
#define BMF_TLUC8 2
extern void gr_init_bitmap(grs_bitmap *bm, uchar *p, uchar type, ushort flags, short w, short h);
extern void gr_init_sub_bitmap(grs_bitmap *sbm, grs_bitmap *dbm, short x, short y, short w, short h);
extern grs_bitmap *gr_alloc_bitmap(uchar type, ushort flags, short w, short h);
#define gr_init_bm gr_init_bitmap
#define gr_init_sub_bm gr_init_sub_bitmap
#define gr_alloc_bm gr_alloc_bitmap
enum {
    GRC_PIXEL,
#define GRC_LINE GRC_PIXEL
    GRC_WIRE_POLY_LINE,
    GRC_DEGEN_LINE,
    GRC_BITMAP,
    GRC_STENCIL_BITMAP,
    GRC_CLUT_BITMAP,
    GRC_HFLIP_BITMAP,
    GRC_CLUT_HFLIP_BITMAP,
    GRC_MASK_BITMAP,
    GRC_HDOUBLE_BITMAP,
    GRC_VDOUBLE_BITMAP,
    GRC_HVDOUBLE_BITMAP,
    GRC_HDOUBLE_BLEND_BITMAP,
    GRC_VDOUBLE_BLEND_BITMAP,
    GRC_HVDOUBLE_BLEND_BITMAP,
    GRC_SCALE,
    GRC_TRANS_SCALE,
    GRC_LIT_SCALE,
    GRC_TRANS_LIT_SCALE,
    GRC_CLUT_SCALE,
    GRC_TRANS_CLUT_SCALE,
    GRC_POLY,
    GRC_MORE_POLY,
    GRC_LIN,
    GRC_TRANS_LIN,
    GRC_LIT_LIN,
    GRC_TRANS_LIT_LIN,
    GRC_CLUT_LIN,
    GRC_TRANS_CLUT_LIN,
    GRC_BILIN,
    GRC_TRANS_BILIN,
    GRC_LIT_BILIN,
    GRC_TRANS_LIT_BILIN,
    GRC_CLUT_BILIN,
    GRC_TRANS_CLUT_BILIN,
    GRC_FLOOR,
    GRC_TRANS_FLOOR,
    GRC_LIT_FLOOR,
    GRC_TRANS_LIT_FLOOR,
    GRC_CLUT_FLOOR,
    GRC_TRANS_CLUT_FLOOR,
    GRC_WALL2D,
    GRC_TRANS_WALL2D,
    GRC_LIT_WALL2D,
    GRC_TRANS_LIT_WALL2D,
    GRC_CLUT_WALL2D,
    GRC_TRANS_CLUT_WALL2D,
    GRC_WALL1D,
    GRC_TRANS_WALL1D,
    GRC_LIT_WALL1D,
    GRC_TRANS_LIT_WALL1D,
    GRC_CLUT_WALL1D,
    GRC_TRANS_CLUT_WALL1D,
    GRC_PER,
    GRC_TRANS_PER,
    GRC_LIT_PER,
    GRC_TRANS_LIT_PER,
    GRC_CLUT_PER,
    GRC_TRANS_CLUT_PER,
    GRC_PER_VSCAN,
    GRC_TRANS_PER_VSCAN,
    GRC_LIT_PER_VSCAN,
    GRC_TRANS_LIT_PER_VSCAN,
    GRC_CLUT_PER_VSCAN,
    GRC_TRANS_CLUT_PER_VSCAN,
    GRD_FUNCS
};
enum { FILL_NORM, FILL_CLUT, FILL_XOR, FILL_BLEND, FILL_SOLID, GRD_FILL_TYPES };

typedef void (*grt_function_table[GRD_FILL_TYPES][GRD_FUNCS * REAL_BMT_TYPES])();
extern grt_function_table flat8_function_table;

enum {
    GR_LINE,
    GR_ILINE,
    GR_HLINE,
    GR_VLINE,
    GR_SLINE,
    GR_CLINE,
    GR_WIRE_POLY_LINE,
    GR_WIRE_POLY_SLINE,
    GR_WIRE_POLY_CLINE,
    GRD_LINE_TYPES
};
typedef void *grt_uline_fill;
typedef void (*grt_uline_fill_v)(long, long, grs_vertex *, grs_vertex *);
typedef void (*grt_uline_fill_xy)(short, short, short, long, long);
typedef void (*grt_wire_poly_uline)(long, long, grs_vertex *, grs_vertex *);
typedef void (*grt_wire_poly_ucline)(long, long, grs_vertex *, grs_vertex *);
typedef grt_uline_fill grt_uline_fill_table[GRD_FILL_TYPES][GRD_LINE_TYPES];
#define grt_wire_poly_usline grt_wire_poly_ucline;
extern grt_uline_fill *grd_uline_fill_vector;
extern grt_uline_fill_table *grd_uline_fill_table;
extern grt_uline_fill_table *grd_uline_fill_table_list[];
extern grt_uline_fill gen_uline_fill_table[][GRD_LINE_TYPES];
extern grt_uline_fill flat8_uline_fill_table[][GRD_LINE_TYPES];

extern grt_function_table *grd_function_table_list[];
extern grt_function_table *grd_function_fill_table;

#ifndef GRSTATE_H
#define GRSTATE_H
#define gr_push_state ((int (*)())grd_pixel_table[PUSH_STATE])
#define gr_pop_state ((int (*)())grd_pixel_table[POP_STATE])
#endif
extern int gr_init(void);
extern int gr_close(void);

extern grs_context grd_defgc;
extern void gr_set_canvas(grs_canvas *c);
extern int gr_push_canvas(grs_canvas *c);
extern grs_canvas *gr_pop_canvas(void);
extern void gr_make_canvas(grs_bitmap *bm, grs_canvas *c);
extern void gr_init_canvas(grs_canvas *c, uchar *p, int id, short w, short h);
extern void gr_init_sub_canvas(grs_canvas *sc, grs_canvas *dc, short x, short y, short w, short h);
extern grs_canvas *gr_alloc_canvas(int id, short w, short h);
extern void gr_free_canvas(grs_canvas *c);
extern grs_canvas *gr_alloc_sub_canvas(grs_canvas *c, short x, short y, short w, short h);
extern void gr_free_sub_canvas(grs_canvas *c);

#define gr_init_gc(c)                             \
    {                                             \
        (c)->gc = grd_defgc;                      \
        (c)->gc.clip.f.right = ((c)->bm.w) << 16; \
        (c)->gc.clip.f.bot = ((c)->bm.h) << 16;   \
    }
#define gr_set_cliprect(l, t, r, b) \
    grd_clip.sten = NULL, grd_clip.left = (l), grd_clip.top = (t), grd_clip.right = (r), grd_clip.bot = (b)
#define gr_safe_set_cliprect(l, t, r, b)                      \
    do {                                                      \
        grd_clip.sten = NULL;                                 \
        grd_clip.left = (((l) < 0) ? 0 : (l));                \
        grd_clip.right = (((r) > grd_bm.w) ? grd_bm.w : (r)); \
        grd_clip.top = (((t) < 0) ? 0 : (t));                 \
        grd_clip.bot = (((b) > grd_bm.h) ? grd_bm.h : (b));   \
    } while (0)
#define gr_set_fix_cliprect(l, t, r, b)                                                                  \
    grd_fix_clip.sten = NULL, grd_fix_clip.left = (l), grd_fix_clip.top = (t), grd_fix_clip.right = (r), \
    grd_fix_clip.bot = (b)
#define gr_set_clipmask(t, b, mask) \
    grd_clip.top = (t), grd_clip.bot = (b), grd_clip.sten = (mask), gr_set_canvas(grd_canvas)
#define gr_set_fcolor(color) (grd_canvas->gc.fcolor = color)
#define gr_get_fcolor() (grd_canvas->gc.fcolor)
#define gr_set_bcolor(color) (grd_canvas->gc.bcolor = color)
#define gr_get_bcolor() (grd_canvas->gc.bcolor)
#define gr_set_font(fnt) (grd_canvas->gc.font = fnt)
#define gr_get_font() (grd_canvas->gc.font)
#define gr_set_text_attr(attr) (grd_canvas->gc.text_attr = attr)
#define gr_get_text_attr() (grd_canvas->gc.text_attr)

// gri_set_fill_globals implementation is in PixFill.C
extern void gri_set_fill_globals(long *fill_type_ptr, long fill_type, void (***function_table_ptr)(),
                                 void (**function_table)(), grt_uline_fill **line_vector_ptr,
                                 grt_uline_fill *line_vector);

#ifdef OPTIMAL_BUT_BROKEN
#define gr_set_fill_type(__ft)                                                              \
    do {                                                                                    \
        long fill_type = __ft;                                                              \
        gri_set_fill_globals(&(grd_canvas->gc.fill_type), fill_type, &grd_function_table,   \
                             (*grd_function_fill_table)[fill_type], &grd_uline_fill_vector, \
                             (*grd_uline_fill_table)[fill_type]);                           \
    } while (0)
#else
#define gr_set_fill_type(type)             \
    do {                                   \
        grd_canvas->gc.fill_type = (type); \
        gr_set_canvas(grd_canvas);         \
    } while (0)
#endif
#define gr_get_fill_type() (grd_canvas->gc.fill_type)
#define gr_set_fill_parm(parm) (grd_canvas->gc.fill_parm = (intptr_t)(parm))
#define gr_get_fill_parm() (grd_canvas->gc.fill_parm)
#define gr_cset_cliprect(c, l, t, r, b)                                                                        \
    (c)->gc.clip.i.sten = NULL, (c)->gc.clip.i.left = (l), (c)->gc.clip.i.top = t, (c)->gc.clip.i.right = (r), \
    (c)->gc.clip.i.bot = (b)
#define gr_cset_fix_cliprect(c, l, t, r, b)                                                 \
    (c)->gc.clip.i.sten->flags = NULL, (c)->gc.clip.f.left = (l), (c)->gc.clip.f.top = (t), \
    (c)->gc.clip.f.right = (r), (c)->gc.clip.f.bot = (b)
#define gr_get_cliprect(l, t, r, b) \
    (*(l) = grd_clip.left, *(t) = grd_clip.top, *(r) = grd_clip.right, *(b) = grd_clip.bot)
#define gr_get_clip_l() (grd_clip.left)
#define gr_get_clip_t() (grd_clip.top)
#define gr_get_clip_r() (grd_clip.right)
#define gr_get_clip_b() (grd_clip.bot)
extern int gr_detect(grs_sys_info *info);

extern grs_mode_info grd_mode_info[];
extern int gr_set_mode(int mode, int clear);

extern grs_screen *gr_alloc_screen(short w, short h);
extern void gr_free_screen(grs_screen *s);
extern void gr_set_screen(grs_screen *s);
typedef void *grt_line_clip_fill;
typedef int (*grt_line_clip_fill_v)(long, long, grs_vertex *, grs_vertex *);
typedef int (*grt_line_clip_fill_xy)(short, short, short, long, long);
extern grt_line_clip_fill *grd_line_clip_fill_vector;
#define grd_uline_fill ((grt_uline_fill_v)(grd_uline_fill_vector[GR_LINE]))
#define grd_uiline_fill ((grt_uline_fill_v)(grd_uline_fill_vector[GR_ILINE]))
#define grd_uhline_fill ((grt_uline_fill_xy)(grd_uline_fill_vector[GR_HLINE]))
#define grd_uvline_fill ((grt_uline_fill_xy)(grd_uline_fill_vector[GR_VLINE]))
#define grd_usline_fill ((grt_uline_fill_v)(grd_uline_fill_vector[GR_SLINE]))
#define grd_ucline_fill ((grt_uline_fill_v)(grd_uline_fill_vector[GR_CLINE]))
#define grd_wire_poly_uline_fill ((grt_wire_poly_uline)(grd_uline_fill_vector[GR_WIRE_POLY_LINE]))
#define grd_wire_poly_usline_fill ((grt_wire_poly_usline)(grd_uline_fill_vector[GR_WIRE_POLY_SLINE]))
#define grd_wire_poly_ucline_fill ((grt_wire_poly_ucline)(grd_uline_fill_vector[GR_WIRE_POLY_CLINE]))
#define grd_line_clip_fill ((grt_line_clip_fill_v)(grd_line_clip_fill_vector[GR_LINE]))
#define grd_iline_clip_fill ((grt_line_clip_fill_v)(grd_line_clip_fill_vector[GR_ILINE]))
#define grd_hline_clip_fill ((grt_line_clip_fill_xy)(grd_line_clip_fill_vector[GR_HLINE]))
#define grd_vline_clip_fill ((grt_line_clip_fill_xy)(grd_line_clip_fill_vector[GR_VLINE]))
#define grd_sline_clip_fill ((grt_line_clip_fill_v)(grd_line_clip_fill_vector[GR_SLINE]))
#define grd_cline_clip_fill ((grt_line_clip_fill_v)(grd_line_clip_fill_vector[GR_CLINE]))
#define grd_wire_poly_line_clip_fill ((grt_wire_poly_uline)(grd_line_clip_fill_vector[GR_WIRE_POLY_LINE]))
#define grd_wire_poly_sline_clip_fill ((grt_wire_poly_usline)(grd_line_clip_fill_vector[GR_WIRE_POLY_SLINE]))
#define grd_wire_poly_cline_clip_fill ((grt_wire_poly_ucline)(grd_line_clip_fill_vector[GR_WIRE_POLY_CLINE]))
#define grd_pixel_fill(c, parm, x, y) gr_fill_upixel(c, x, y)

#define gr_clut_hflip_flat8_ubitmap \
    ((void (*)(grs_bitmap * bm, short x, short y, uchar *cl)) grd_canvas_table[CLUT_HFLIP_FLAT8_UBITMAP])

#define gr_int_circle ((int (*)(short x, short y, short r))grd_canvas_table[INT_CIRCLE])
#define gr_int_disk ((int (*)(short x, short y, short r))grd_canvas_table[INT_DISK])
#define gr_ubitmap(bm, x, y)                                                                                           \
    ((void (*)(grs_bitmap * _bm, short _x, short _y)) grd_canvas_table[DRAW_DEVICE_UBITMAP + 2 * ((bm)->type)])(bm, x, \
                                                                                                                y)
#define gr_bitmap(bm, x, y) \
    ((int (*)(grs_bitmap * _bm, short _x, short _y)) grd_canvas_table[DRAW_DEVICE_BITMAP + 2 * ((bm)->type)])(bm, x, y)
#define gr_mono_ubitmap ((void (*)(grs_bitmap * bm, short x, short y)) grd_canvas_table[DRAW_MONO_UBITMAP])
#define gr_mono_bitmap ((int (*)(grs_bitmap * bm, short x, short y)) grd_canvas_table[DRAW_MONO_BITMAP])
#define gr_flat8_ubitmap ((void (*)(grs_bitmap * bm, short x, short y)) grd_canvas_table[DRAW_FLAT8_UBITMAP])
#define gr_flat8_bitmap ((int (*)(grs_bitmap * bm, short x, short y)) grd_canvas_table[DRAW_FLAT8_BITMAP])
#define gr_flat24_ubitmap ((void (*)(grs_bitmap * bm, short x, short y)) grd_canvas_table[DRAW_FLAT24_UBITMAP])
#define gr_rsd8_bitmap ((int (*)(grs_bitmap * bm, short x, short y)) grd_canvas_table[DRAW_RSD8_BITMAP])
#define gr_tluc8_ubitmap ((void (*)(grs_bitmap * bm, short x, short y)) grd_canvas_table[DRAW_TLUC8_UBITMAP])
#define gr_get_ubitmap(bm, x, y) \
    ((void (*)(grs_bitmap * _bm, short _x, short _y)) grd_canvas_table[GET_DEVICE_UBITMAP + 2 * ((bm)->type)])(bm, x, y)
#define gr_get_bitmap(bm, x, y) \
    ((int (*)(grs_bitmap * _bm, short _x, short _y)) grd_canvas_table[GET_DEVICE_BITMAP + 2 * ((bm)->type)])(bm, x, y)

#define gr_hflip_bitmap(bm, x, y)                                                                                      \
    ((void (*)(grs_bitmap * _bm, short _x, short _y)) grd_canvas_table[HFLIP_DEVICE_BITMAP + 2 * ((bm)->type)])(bm, x, \
                                                                                                                y)
#define gr_hflip_flat8_ubitmap ((void (*)(grs_bitmap * bm, short x, short y)) grd_canvas_table[HFLIP_FLAT8_UBITMAP])

extern void gr_set_malloc(void *(*malloc_func)(int bytes));
extern void gr_set_free(void (*free_func)(void *mem));
extern void *(*gr_malloc)(int n);
extern void (*gr_free)(void *p);
extern int gr_int_line(short x0, short y0, short x1, short y1);

#define gr_ucpoly ((void (*)(long c, int n, grs_vertex **vpl))grd_canvas_table[FIX_UCPOLY])
#define gr_cpoly ((int (*)(long c, int n, grs_vertex **vpl))grd_canvas_table[FIX_CPOLY])
#define gr_init_device(info) \
    (grd_device_table[GRT_INIT_DEVICE] ? ((int (*)(grs_sys_info * _info)) grd_device_table[GRT_INIT_DEVICE])(info) : 0)
#define gr_close_device(info)                                                                                        \
    (grd_device_table[GRT_CLOSE_DEVICE] ? ((int (*)(grs_sys_info * _info)) grd_device_table[GRT_CLOSE_DEVICE])(info) \
                                        : 0)
#define gr_set_screen_mode ((int (*)(int mode, int clear))grd_device_table[GRT_SET_MODE])
#define gr_get_screen_mode ((int (*)(void))grd_device_table[GRT_GET_MODE])
#define gr_set_state ((int (*)(void *buf, int clear))grd_device_table[GRT_SET_STATE])
#define gr_get_state ((int (*)(void *buf, int flags))grd_device_table[GRT_GET_STATE])
#define gr_stat_htrace ((int (*)(void))grd_device_table[GRT_STAT_HTRACE])
#define gr_stat_vtrace ((int (*)(void))grd_device_table[GRT_STAT_VTRACE])
#define gr_set_screen_pal ((void (*)(int start, int n, uchar *pal_data))grd_device_table[GRT_SET_PAL])

#define gr_set_upixel24 ((void (*)(long color, short x, short y))grd_pixel_table[SET_UPIXEL24])

#define gr_set_upixel ((void (*)(long color, short x, short y))grd_pixel_table[SET_UPIXEL8])
#define gr_set_pixel ((int (*)(long color, short x, short y))grd_pixel_table[SET_PIXEL8])
extern int gen_fill_pixel(long color, short x, short y);
#define gr_set_upixel_interrupt ((void (*)(long color, short x, short y))grd_pixel_table[SET_UPIXEL8_INTERRUPT])
#define gr_set_pixel_interrupt ((int (*)(long color, short x, short y))grd_pixel_table[SET_PIXEL8_INTERRUPT])
#define gr_fill_upixel ((void (*)(long color, short x, short y))grd_function_table[GRC_PIXEL])
#define gr_fill_pixel gen_fill_pixel
#define gr_get_upixel ((long (*)(short x, short y))grd_pixel_table[GET_UPIXEL8])
#define gr_get_pixel ((long (*)(short x, short y))grd_pixel_table[GET_PIXEL8])
#define gr_poly ((int (*)(long c, int n, grs_vertex **vpl))grd_canvas_table[FIX_POLY])
#define gr_per_map(bm, n, vpl)                              \
    ((int (*)(grs_bitmap * _bm, int _n, grs_vertex **_vpl)) \
         grd_canvas_table[DEVICE_PER_MAP + 2 * ((bm)->type)])(bm, n, vpl)

#define gr_clut_ubitmap(bm, x, y, cl)                             \
    ((void (*)(grs_bitmap * _bm, short _x, short _y, uchar *_cl)) \
         grd_canvas_table[CLUT_DRAW_DEVICE_UBITMAP + 2 * ((bm)->type)])(bm, x, y, cl)
#define gr_clut_bitmap(bm, x, y, cl)                             \
    ((int (*)(grs_bitmap * _bm, short _x, short _y, uchar *_cl)) \
         grd_canvas_table[CLUT_DRAW_DEVICE_BITMAP + 2 * ((bm)->type)])(bm, x, y, cl)
#define gr_flat8_clut_ubitmap \
    ((void (*)(grs_bitmap * bm, short x, short y, uchar *cl)) grd_canvas_table[CLUT_DRAW_FLAT8_UBITMAP])
#define gr_clear ((void (*)(long color))grd_canvas_table[DRAW_CLEAR])
#define gr_upoint ((void (*)(short x, short y))grd_canvas_table[DRAW_UPOINT])
#define gr_point ((int (*)(short x, short y))grd_canvas_table[DRAW_POINT])
#define gr_uhline(x0, y0, x1)                                                   \
    do {                                                                        \
        grd_uhline_fill((x0), (y0), (x1), gr_get_fcolor(), gr_get_fill_parm()); \
    } while (0)
extern int gen_hline(short x0, short y0, short x1);
#define gr_hline gen_hline
#define gr_uvline(x0, y0, y1)                                                   \
    do {                                                                        \
        grd_uvline_fill((x0), (y0), (y1), gr_get_fcolor(), gr_get_fill_parm()); \
    } while (0)
extern int gen_vline(short x0, short y0, short y1);
#define gr_vline gen_vline
#define gr_urect ((void (*)(short x0, short y0, short x1, short y1))grd_canvas_table[DRAW_URECT])
#define gr_rect ((int (*)(short x0, short y0, short x1, short y1))grd_canvas_table[DRAW_RECT])
#define gr_ubox ((void (*)(short x0, short y0, short x1, short y1))grd_canvas_table[DRAW_UBOX])
#define gr_box ((int (*)(short x0, short y0, short x1, short y1))grd_canvas_table[DRAW_BOX])
#define gr_fix_line gen_fix_line
extern int gen_fix_line(fix x0, fix y0, fix x1, fix y1);
extern int gen_fix_cline(fix x0, fix y0, grs_rgb c0, fix x1, fix y1, grs_rgb c1);
extern int gen_fix_sline(fix x0, fix y0, fix i0, fix x1, fix y1, fix i1);

#define gr_scale_ubitmap(bm, x, y, w, h)                                  \
    ((void (*)(grs_bitmap * _bm, short _x, short _y, short _w, short _h)) \
         grd_canvas_table[SCALE_DEVICE_UBITMAP + 2 * ((bm)->type)])(bm, x, y, w, h)
#define gr_scale_bitmap(bm, x, y, w, h)                                  \
    ((int (*)(grs_bitmap * _bm, short _x, short _y, short _w, short _h)) \
         grd_canvas_table[SCALE_DEVICE_BITMAP + 2 * ((bm)->type)])(bm, x, y, w, h)
#define gr_clut_scale_ubitmap(bm, x, y, w, h, cl)                                    \
    ((int (*)(grs_bitmap * _bm, short _x, short _y, short _w, short _h, uchar *_cl)) \
         grd_canvas_table[CLUT_SCALE_DEVICE_UBITMAP + 2 * ((bm)->type)])(bm, x, y, w, h, cl)
#define gr_clut_scale_bitmap(bm, x, y, w, h, cl)                                     \
    ((int (*)(grs_bitmap * _bm, short _x, short _y, short _w, short _h, uchar *_cl)) \
         grd_canvas_table[CLUT_SCALE_DEVICE_BITMAP + 2 * ((bm)->type)])(bm, x, y, w, h, cl)

#define gr_string(s, x, y) \
    (((int (*)(grs_font *, char *, short, short))grd_canvas_table[DRAW_STRING]))((grs_font *)gr_get_font(), s, x, y)
#define gr_scale_string(s, x, y, w, h)                                                                \
    (((int (*)(grs_font *, char *, short, short, short, short))grd_canvas_table[DRAW_SCALE_STRING]))( \
        (grs_font *)gr_get_font(), s, x, y, w, h)
#define gr_char(s, x, y) \
    (((int (*)(grs_font *, char, short, short))grd_canvas_table[DRAW_CHAR]))((grs_font *)gr_get_font(), s, x, y)

enum {
    SET_UPIXEL8,
    SET_PIXEL8,
    GET_UPIXEL8,
    GET_PIXEL8,
    SET_UPIXEL24,
    SET_PIXEL24,
    GET_UPIXEL24,
    GET_PIXEL24,
    DRAW_CLEAR,
    DRAW_UPOINT,
    DRAW_POINT,
    SET_UPIXEL8_INTERRUPT,
    SET_PIXEL8_INTERRUPT,
    DRAW_UVLINE,
    DRAW_VLINE,
    DRAW_URECT,
    DRAW_RECT,
    DRAW_UBOX,
    DRAW_BOX,
    PUSH_STATE,
    POP_STATE,
    FIX_USLINE,
    FIX_SLINE,
    FIX_UCLINE,
    FIX_CLINE,
    FIX_UPOLY,
    FIX_POLY,
    FIX_USPOLY,
    FIX_SPOLY,
    FIX_UCPOLY,
    FIX_CPOLY,
    FIX_TLUC8_UPOLY,
    FIX_TLUC8_POLY,
    FIX_TLUC8_USPOLY,
    FIX_TLUC8_SPOLY,
    VOX_RECT,
    VOX_POLY,
    VOX_CPOLY,
    INTERP2_UBITMAP,
    FILTER2_UBITMAP,
    ROLL_UBITMAP,
    ROLL_BITMAP,
    FLAT8_WALL_UMAP,
    FLAT8_WALL_MAP,
    FLAT8_LIT_WALL_UMAP,
    FLAT8_LIT_WALL_MAP,
    FLAT8_CLUT_WALL_UMAP,
    FLAT8_CLUT_WALL_MAP,
    FLAT8_FLOOR_UMAP,
    FLAT8_FLOOR_MAP,
    FLAT8_LIT_FLOOR_UMAP,
    FLAT8_LIT_FLOOR_MAP,
    FLAT8_CLUT_FLOOR_UMAP,
    FLAT8_CLUT_FLOOR_MAP,
    DEVICE_ULMAP,
    DEVICE_LMAP,
    MONO_ULMAP,
    MONO_LMAP,
    FLAT8_ULMAP,
    FLAT8_LMAP,
    FLAT24_ULMAP,
    FLAT24_LMAP,
    RSD_ULMAP,
    RSD_LMAP,
    TLUC8_ULMAP,
    TLUC8_LMAP,
    DEVICE_LIT_LIN_UMAP,
    DEVICE_LIT_LIN_MAP,
    MONO_LIT_LIN_UMAP,
    MONO_LIT_LIN_MAP,
    FLAT8_LIT_LIN_UMAP,
    FLAT8_LIT_LIN_MAP,
    FLAT24_LIT_LIN_UMAP,
    FLAT24_LIT_LIN_MAP,
    RSD_LIT_LIN_UMAP,
    RSD_LIT_LIN_MAP,
    TLUC8_LIT_LIN_UMAP,
    TLUC8_LIT_LIN_MAP,
    DEVICE_CLUT_LIN_UMAP,
    DEVICE_CLUT_LIN_MAP,
    MONO_CLUT_LIN_UMAP,
    MONO_CLUT_LIN_MAP,
    FLAT8_CLUT_LIN_UMAP,
    FLAT8_CLUT_LIN_MAP,
    FLAT24_CLUT_LIN_UMAP,
    FLAT24_CLUT_LIN_MAP,
    RSD_CLUT_LIN_UMAP,
    RSD_CLUT_LIN_MAP,
    TLUC8_CLUT_LIN_UMAP,
    TLUC8_CLUT_LIN_MAP,
    FLAT8_SOLID_LIN_UMAP,
    FLAT8_SOLID_LIN_MAP,
    DEVICE_PER_UMAP,
    DEVICE_PER_MAP,
    MONO_PER_UMAP,
    MONO_PER_MAP,
    FLAT8_PER_UMAP,
    FLAT8_PER_MAP,
    FLAT24_PER_UMAP,
    FLAT24_PER_MAP,
    RSD_PER_UMAP,
    RSD_PER_MAP,
    TLUC8_PER_UMAP,
    TLUC8_PER_MAP,
    DEVICE_LIT_PER_UMAP,
    DEVICE_LIT_PER_MAP,
    MONO_LIT_PER_UMAP,
    MONO_LIT_PER_MAP,
    FLAT8_LIT_PER_UMAP,
    FLAT8_LIT_PER_MAP,
    FLAT24_LIT_PER_UMAP,
    FLAT24_LIT_PER_MAP,
    RSD_LIT_PER_UMAP,
    RSD_LIT_PER_MAP,
    TLUC8_LIT_PER_UMAP,
    TLUC8_LIT_PER_MAP,
    DEVICE_CLUT_PER_UMAP,
    DEVICE_CLUT_PER_MAP,
    MONO_CLUT_PER_UMAP,
    MONO_CLUT_PER_MAP,
    FLAT8_CLUT_PER_UMAP,
    FLAT8_CLUT_PER_MAP,
    FLAT24_CLUT_PER_UMAP,
    FLAT24_CLUT_PER_MAP,
    RSD_CLUT_PER_UMAP,
    RSD_CLUT_PER_MAP,
    TLUC8_CLUT_PER_UMAP,
    TLUC8_CLUT_PER_MAP,
    FLAT8_SOLID_PER_UMAP,
    FLAT8_SOLID_PER_MAP,
    INT_UCIRCLE,
    INT_CIRCLE,
    FIX_UCIRCLE,
    FIX_CIRCLE,
    INT_UDISK,
    INT_DISK,
    FIX_UDISK,
    FIX_DISK,
    INT_UROD,
    INT_ROD,
    FIX_UROD,
    FIX_ROD,
    DRAW_DEVICE_UBITMAP,
    DRAW_DEVICE_BITMAP,
    DRAW_MONO_UBITMAP,
    DRAW_MONO_BITMAP,
    DRAW_FLAT8_UBITMAP,
    DRAW_FLAT8_BITMAP,
    DRAW_FLAT24_UBITMAP,
    DRAW_FLAT24_BITMAP,
    DRAW_RSD8_UBITMAP,
    DRAW_RSD8_BITMAP,
    DRAW_TLUC8_UBITMAP,
    DRAW_TLUC8_BITMAP,
    CLUT_DRAW_DEVICE_UBITMAP,
    CLUT_DRAW_DEVICE_BITMAP,
    CLUT_DRAW_MONO_UBITMAP,
    CLUT_DRAW_MONO_BITMAP,
    CLUT_DRAW_FLAT8_UBITMAP,
    CLUT_DRAW_FLAT8_BITMAP,
    CLUT_DRAW_FLAT24_UBITMAP,
    CLUT_DRAW_FLAT24_BITMAP,
    CLUT_DRAW_RSD8_UBITMAP,
    CLUT_DRAW_RSD8_BITMAP,
    CLUT_DRAW_TLUC8_UBITMAP,
    CLUT_DRAW_TLUC8_BITMAP,
    SOLID_RSD8_UBITMAP,
    SOLID_RSD8_BITMAP,
    SCALE_DEVICE_UBITMAP,
    SCALE_DEVICE_BITMAP,
    SCALE_MONO_UBITMAP,
    SCALE_MONO_BITMAP,
    SCALE_FLAT8_UBITMAP,
    SCALE_FLAT8_BITMAP,
    SCALE_FLAT24_UBITMAP,
    SCALE_FLAT24_BITMAP,
    SCALE_RSD8_UBITMAP,
    SCALE_RSD8_BITMAP,
    SCALE_TLUC8_UBITMAP,
    SCALE_TLUC8_BITMAP,
    SOLID_SCALE_RSD8_UBITMAP,
    SOLID_SCALE_RSD8_BITMAP,
    CLUT_SCALE_DEVICE_UBITMAP,
    CLUT_SCALE_DEVICE_BITMAP,
    CLUT_SCALE_MONO_UBITMAP,
    CLUT_SCALE_MONO_BITMAP,
    CLUT_SCALE_FLAT8_UBITMAP,
    CLUT_SCALE_FLAT8_BITMAP,
    CLUT_SCALE_FLAT24_UBITMAP,
    CLUT_SCALE_FLAT24_BITMAP,
    CLUT_SCALE_RSD8_UBITMAP,
    CLUT_SCALE_RSD8_BITMAP,
    CLUT_SCALE_TLUC8_UBITMAP,
    CLUT_SCALE_TLUC8_BITMAP,
    MASK_DEVICE_UBITMAP,
    MASK_DEVICE_BITMAP,
    MASK_MONO_UBITMAP,
    MASK_MONO_BITMAP,
    MASK_FLAT8_UBITMAP,
    MASK_FLAT8_BITMAP,
    MASK_FLAT24_UBITMAP,
    MASK_FLAT24_BITMAP,
    MASK_RSD8_UBITMAP,
    MASK_RSD8_BITMAP,
    MASK_TLUC8_UBITMAP,
    MASK_TLUC8_BITMAP,
    GET_DEVICE_UBITMAP,
    GET_DEVICE_BITMAP,
    GET_MONO_UBITMAP,
    GET_MONO_BITMAP,
    GET_FLAT8_UBITMAP,
    GET_FLAT8_BITMAP,
    GET_FLAT24_UBITMAP,
    GET_FLAT24_BITMAP,
    GET_RSD8_UBITMAP,
    GET_RSD8_BITMAP,
    GET_TLUC8_UBITMAP,
    GET_TLUC8_BITMAP,
    HFLIP_DEVICE_UBITMAP,
    HFLIP_DEVICE_BITMAP,
    HFLIP_MONO_UBITMAP,
    HFLIP_MONO_BITMAP,
    HFLIP_FLAT8_UBITMAP,
    HFLIP_FLAT8_BITMAP,
    HFLIP_FLAT24_UBITMAP,
    HFLIP_FLAT24_BITMAP,
    HFLIP_RSD8_UBITMAP,
    HFLIP_RSD8_BITMAP,
    HFLIP_TLUC8_UBITMAP,
    HFLIP_TLUC8_BITMAP,
    CLUT_HFLIP_DEVICE_UBITMAP,
    CLUT_HFLIP_DEVICE_BITMAP,
    CLUT_HFLIP_MONO_UBITMAP,
    CLUT_HFLIP_MONO_BITMAP,
    CLUT_HFLIP_FLAT8_UBITMAP,
    CLUT_HFLIP_FLAT8_BITMAP,
    CLUT_HFLIP_FLAT24_UBITMAP,
    CLUT_HFLIP_FLAT24_BITMAP,
    CLUT_HFLIP_RSD8_UBITMAP,
    CLUT_HFLIP_RSD8_BITMAP,
    CLUT_HFLIP_TLUC8_UBITMAP,
    CLUT_HFLIP_TLUC8_BITMAP,
    DOUBLE_H_DEVICE_UBITMAP,
    DOUBLE_H_DEVICE_BITMAP,
    DOUBLE_H_MONO_UBITMAP,
    DOUBLE_H_MONO_BITMAP,
    DOUBLE_H_FLAT8_UBITMAP,
    DOUBLE_H_FLAT8_BITMAP,
    DOUBLE_H_FLAT24_UBITMAP,
    DOUBLE_H_FLAT24_BITMAP,
    DOUBLE_H_RSD8_UBITMAP,
    DOUBLE_H_RSD8_BITMAP,
    DOUBLE_H_TLUC8_UBITMAP,
    DOUBLE_H_TLUC8_BITMAP,
    DOUBLE_V_DEVICE_UBITMAP,
    DOUBLE_V_DEVICE_BITMAP,
    DOUBLE_V_MONO_UBITMAP,
    DOUBLE_V_MONO_BITMAP,
    DOUBLE_V_FLAT8_UBITMAP,
    DOUBLE_V_FLAT8_BITMAP,
    DOUBLE_V_FLAT24_UBITMAP,
    DOUBLE_V_FLAT24_BITMAP,
    DOUBLE_V_RSD8_UBITMAP,
    DOUBLE_V_RSD8_BITMAP,
    DOUBLE_V_TLUC8_UBITMAP,
    DOUBLE_V_TLUC8_BITMAP,
    DOUBLE_HV_DEVICE_UBITMAP,
    DOUBLE_HV_DEVICE_BITMAP,
    DOUBLE_HV_MONO_UBITMAP,
    DOUBLE_HV_MONO_BITMAP,
    DOUBLE_HV_FLAT8_UBITMAP,
    DOUBLE_HV_FLAT8_BITMAP,
    DOUBLE_HV_FLAT24_UBITMAP,
    DOUBLE_HV_FLAT24_BITMAP,
    DOUBLE_HV_RSD8_UBITMAP,
    DOUBLE_HV_RSD8_BITMAP,
    DOUBLE_HV_TLUC8_UBITMAP,
    DOUBLE_HV_TLUC8_BITMAP,
    SMOOTH_DOUBLE_H_DEVICE_UBITMAP,
    SMOOTH_DOUBLE_H_DEVICE_BITMAP,
    SMOOTH_DOUBLE_H_MONO_UBITMAP,
    SMOOTH_DOUBLE_H_MONO_BITMAP,
    SMOOTH_DOUBLE_H_FLAT8_UBITMAP,
    SMOOTH_DOUBLE_H_FLAT8_BITMAP,
    SMOOTH_DOUBLE_H_FLAT24_UBITMAP,
    SMOOTH_DOUBLE_H_FLAT24_BITMAP,
    SMOOTH_DOUBLE_H_RSD8_UBITMAP,
    SMOOTH_DOUBLE_H_RSD8_BITMAP,
    SMOOTH_DOUBLE_H_TLUC8_UBITMAP,
    SMOOTH_DOUBLE_H_TLUC8_BITMAP,
    SMOOTH_DOUBLE_V_DEVICE_UBITMAP,
    SMOOTH_DOUBLE_V_DEVICE_BITMAP,
    SMOOTH_DOUBLE_V_MONO_UBITMAP,
    SMOOTH_DOUBLE_V_MONO_BITMAP,
    SMOOTH_DOUBLE_V_FLAT8_UBITMAP,
    SMOOTH_DOUBLE_V_FLAT8_BITMAP,
    SMOOTH_DOUBLE_V_FLAT24_UBITMAP,
    SMOOTH_DOUBLE_V_FLAT24_BITMAP,
    SMOOTH_DOUBLE_V_RSD8_UBITMAP,
    SMOOTH_DOUBLE_V_RSD8_BITMAP,
    SMOOTH_DOUBLE_V_TLUC8_UBITMAP,
    SMOOTH_DOUBLE_V_TLUC8_BITMAP,
    SMOOTH_DOUBLE_HV_DEVICE_UBITMAP,
    SMOOTH_DOUBLE_HV_DEVICE_BITMAP,
    SMOOTH_DOUBLE_HV_MONO_UBITMAP,
    SMOOTH_DOUBLE_HV_MONO_BITMAP,
    SMOOTH_DOUBLE_HV_FLAT8_UBITMAP,
    SMOOTH_DOUBLE_HV_FLAT8_BITMAP,
    SMOOTH_DOUBLE_HV_FLAT24_UBITMAP,
    SMOOTH_DOUBLE_HV_FLAT24_BITMAP,
    SMOOTH_DOUBLE_HV_RSD8_UBITMAP,
    SMOOTH_DOUBLE_HV_RSD8_BITMAP,
    SMOOTH_DOUBLE_HV_TLUC8_UBITMAP,
    SMOOTH_DOUBLE_HV_TLUC8_BITMAP,
    DRAW_USTRING,
    DRAW_STRING,
    DRAW_SCALE_USTRING,
    DRAW_SCALE_STRING,
    DRAW_UCHAR,
    DRAW_CHAR,
    CALC_ROW,
    SUB_BITMAP,
    START_FRAME,
    END_FRAME,
    GRD_CANVAS_FUNCS
};
enum {
    GRT_INIT_DEVICE,
    GRT_CLOSE_DEVICE,
    GRT_SET_MODE,
    GRT_GET_MODE,
    GRT_SET_STATE,
    GRT_GET_STATE,
    GRT_STAT_HTRACE,
    GRT_STAT_VTRACE,
    GRT_SET_PAL,
    GRT_GET_PAL,
    GRT_SET_WIDTH,
    GRT_GET_WIDTH,
    GRT_SET_FOCUS,
    GRT_GET_FOCUS,
    GRT_CANVAS_TABLE,
    GRT_SPAN_TABLE,
    GRD_DEVICE_FUNCS
};

extern void gr_set_pal(int start, int n, uchar *pal_data);
extern void gr_set_gamma_pal(int start, int n, fix gamma);
extern void gr_get_pal(int start, int n, uchar *pal_data);
typedef struct {
    ubyte ltol, wftol;
    fix cltol;
} gr_per_detail_level;
enum { GR_LOW_PER_DETAIL, GR_MEDIUM_PER_DETAIL, GR_HIGH_PER_DETAIL, GR_NUM_PER_DETAIL_LEVELS };
extern void gr_set_per_tol(ubyte linear_tol, ubyte wall_floor_tol);
extern void gr_set_clut_lit_tol(fix clut_lit_tol);
extern void gr_set_per_detail_level(int detail_level);
extern void gr_set_per_detail_level_param(int linear_tol, int wall_floor_tol, fix clut_lit_tol, int detail_level);
#define RGB_OK (0)
#define RGB_OUT_OF_MEMORY (-1)
#define RGB_CANT_DEALLOCATE (-2)
#define RGB_IPAL_NOT_ALLOCATED (-3)
#define gr_index_rgb(r, g, b) (((r) >> 19) & 0x1f) | (((g) >> 14) & 0x3e0) | (((b) >> 9) & 0x7c00)
#define gr_index_lrgb(t) ((((t) >> 3) & 0x1f) | (((t) >> 6) & 0x3e0) | (((t) >> 9) & 0x7c00))
#define gr_bind_rgb(r, g, b) (((r) << 2) | ((g) << 13) | ((b) << 24))
#define gr_index_brgb(c) ((((c) >> 5) & 0x1f) | (((c) >> 11) & 0x3e0) | (((c) >> 17) & 0x7c00))
extern void gr_split_rgb(grs_rgb c, uchar *r, uchar *g, uchar *b);
int gr_alloc_ipal(void);
int gr_init_ipal(void);
int gr_free_ipal(void);
#define gr_get_light_tab() (grd_screen->ltab)
#define gr_set_light_tab(p) (grd_screen->ltab = (p))
#define gr_get_clut() (grd_screen->clut)

extern void gr_font_string_size(grs_font *font, char *string, short *width, short *height);
extern short gr_font_string_width(grs_font *font, char *string);
extern short gr_font_char_width(grs_font *font, char c);
extern void gr_font_char_size(grs_font *font, char c, short *width, short *height);
extern int gr_font_string_wrap(grs_font *pfont, char *ps, short width);
extern void gr_font_string_unwrap(char *s);
#define gr_string_size(s, w, h) gr_font_string_size((grs_font *)gr_get_font(), s, w, h)
#define gr_string_width(s) gr_font_string_width((grs_font *)gr_get_font(), s)
#define gr_char_width(c) gr_font_char_width((grs_font *)gr_get_font(), c)
#define gr_char_size(c, w, h) gr_font_char_size((grs_font *)gr_get_font(), c, w, h)
#define gr_string_wrap(string, width) gr_font_string_wrap((grs_font *)gr_get_font(), string, width)

typedef struct {
    grs_vertex val;
    grs_vertex d;
} grs_span_vertex;

typedef struct _span {
    short l, r;
    struct _span *n;
    union {
        struct {
            grs_span_vertex *lvert, *rvert;
        } pgon;
        struct {
            uchar *pp;
            fix scale;
            fix start;
        } bitmap;
    };
} grs_span;

#ifndef _RSDCVT_C
extern uchar *grd_unpack_buf;
extern int gr_rsd8_convert(grs_bitmap *sbm, grs_bitmap *dbm);
#endif

uchar *gr_rsd8_unpack(uchar *src, uchar *dst);

#define gr_set_unpack_buf(buf) grd_unpack_buf = buf
#define gr_get_unpack_buf() grd_unpack_buf
#define GR_UNPACK_RSD8_OK 0
#define GR_UNPACK_RSD8_NOBUF 1
#define GR_UNPACK_RSD8_NOTRSD 2
uchar gr_free_blend(void);
uchar gr_init_blend(int log_blend_levels);

#define gr_start_frame ((void (*)())grd_canvas_table[START_FRAME])

#pragma pack(pop)

#if defined(__cplusplus)
}
#endif // !defined(__cplusplus)

#endif /* __2D_H */
