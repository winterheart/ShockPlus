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
 * $Source: r:/prj/cit/src/RCS/rendtool.c $
 * $Revision: 1.37 $
 * $Author: buzzard $
 * $Date: 1994/11/25 09:16:45 $
 *
 *
 *  support for render functions and tools, such as mouse and so on
 */

#include "Engine/Options.h"
#include "3dinterp.h"
#include "map.h"
#include "frintern.h"
#include "fullscrn.h"
#include "gamerend.h"
#include "textmaps.h"
#include "gettmaps.h"

#include "frterr.h"
#include "frutils.h"
#include "player.h"
#include "rendtool.h"

#include "objects.h"
#include "objprop.h"
#include "objbit.h"
#include "objsim.h"
#include "view360.h"

#include "modtext.h"
#include "citmat.h"

// should probably wimp out and take frintern
// or have a separate fritrfce or something
// rather than externing everything a couple of 30 lines down

#include "frtypes.h" // so we get _fr, so we can get size + canvas and all
#include "frparams.h"
#include "frflags.h"
#include "fr3d.h"
#include "frquad.h"

#include "gamesort.h"
#include "citres.h"

#include "star.h"

#include "curdat.h"

// pain, stupidity, yes
char model_vtext_data[] = {
    29, -1,
    21, 15, 16, -1,
    37, 15, 21, -1,
    38, 3, -1,
    39, -1,
    21, -1,
    21, -1,
    1, 2, 3, 4, -1,
    23, 21, 16, -1,
    25, 47, -1,
    9, 48, -1,
    40, 49, -1,
    49, -1,
    22, 21, -1,
    5, 7, -1,
    50, -1,
    7, -1,
    5, 7, 8, -1,
    10, 11, 15, 16, 21, -1,
    0, -1,
    13, 15, 46, -1,
    18, 19, -1,
    0, 26, -1,
    8, 28, 7, -1,
    20, 15, -1,
    1, 21, 39, -1,
    18, 21, -1,
    12, -1,
    41, 39, 2, 15, 6, 21, -1,
    8, -1,
    10, 11, 12, 13, 21, -1,
    14, 15, 16, 17, -1,
    26, -1,
    0, -1,
    0, -1,
    0, -1,
    0, -1,
    0, -1,
    36, -1,
    24, -1,
    0, -1,
    0, -1,
    0, -1,
    0, -1,
    0, -1,
    0, -1,
    0, -1,
    0, -1,
    0, -1,
    17, 32, 21, -1,
    43, -1,
    -1,
};


LGRect *rendrect;
extern fauxrend_context *_fr;
ubyte mouselocked = 0;

#define NUM_STARS 1000

sts_vec star_vec[NUM_STARS];
uchar star_col[NUM_STARS];

//------------------------
//  Internal Prototypes
//------------------------
void rend_mouse_hide(void);
void rend_mouse_show(void);
uchar game_obj_block_home(void *vmptr, uchar *_sclip, int *loc);
uchar game_obj_block(void *vmptr, uchar *_sclip, int *loc);
int game_fr_idx(void);
grs_bitmap *game_fr_tmap_128(void);
grs_bitmap *game_fr_tmap_64(void);
grs_bitmap *game_fr_tmap_full(void);
void game_rend_start(void);
void game_fr_clip_start(uchar headnorth);

// Note that I have fixed this so that the cursor does not flicker.
// It just works.  Note its simplistic beauty.	I love this job.
void rend_mouse_hide(void) {

    MouseLock++;
    if (MouseLock == 1 && CurrentCursor != NULL) {
        int cmd = CURSOR_DRAW;
        LGPoint pos = LastCursorPos;
        grs_canvas *old_canvas = CursorCanvas;

        pos.x -= _fr->xtop;
        pos.y -= _fr->ytop;

        /*KLC - this is never true, now
                        if (_fr_curflags & FR_DOHFLIP_MASK)
                        {
                                pos.x = _fr->xwid - pos.x - 1;
                                cmd = CURSOR_DRAW_HFLIP;
                        }
        */
        mouselocked = 1;
        MouseLock++; // keep mouse locked while we blit

        if (ShockPlus::Options::halfResolution && !view360_is_rendering) // If double-sizing, then draw the cursor in the
        {                                        // temporary doubled canvas.
            CursorCanvas = &gDoubleSizeOffCanvas;
            if (!full_game_3d) // In slot view, adjust the cursor position.
            {
                pos.x -= 28;
                pos.y -= 29;
            }
        } else
            CursorCanvas = &_fr->draw_canvas;
        if (LastCursor)
            LastCursor->func(cmd, CursorRegion, LastCursor, pos);
        CursorCanvas = old_canvas;
    } else
        mouselocked = 0;
    MouseLock--; // decrement mouselock after frame buffer blit.
}

void rend_mouse_show(void) { MouseLock -= mouselocked; }

// extern grs_bitmap tmap_bm[32]; // this is dumb, yea yea

static MapElem *home_ptr;

uchar game_obj_block_home(void *vmptr, uchar *_sclip, int *loc) {
    MapElem *mptr = (MapElem *)vmptr;
    Obj *cobj;
    ObjID cobjid;
    ObjRefID curORef;

    curORef = mptr->objRef;
    while (curORef != OBJ_REF_NULL) {
        cobjid = objRefs[curORef].obj;
        if (ObjProps[OPNUM(cobjid)].flags & RENDER_BLOCK) { // if we are a block type object
            cobj = &objs[cobjid];
            if (((cobj->loc.p | cobj->loc.b) == 0) && ((cobj->loc.h & 0x3f) == 0) && (cobj->info.current_frame == 0))
                if ((objRefs[curORef].state.bin.sq.x == (cobj->loc.x >> 8)) &&
                    (objRefs[curORef].state.bin.sq.y == (cobj->loc.y >> 8))) { // is it at correct heading
                    if ((cobj->loc.h + 0x20) & 0x40) {
                        //		    mprintf("Thinking about X subclip from %x and %x, clip
                        //%x\n",cobj->loc.x,fr_camera_last[0],_sclip[1]);
                        if ((home_ptr == mptr) || (loc[0] >> 16 == _fr_x_cen)) {
                            if (_sclip[1] == FMK_INT_WW) {
                                if (cobj->loc.x < (fr_camera_last[0] >> 8)) {
                                    _me_subclip(mptr) |= _sclip[1];
                                    return TRUE;
                                }
                            } else if (cobj->loc.x > (fr_camera_last[0] >> 8)) {
                                _me_subclip(mptr) |= _sclip[1];
                                return TRUE;
                            }
                            //		    mprintf("Guess not\n");
                            return FALSE;
                        } else {
                            _me_subclip(mptr) |= _sclip[1]; /* mprintf("xsubclip standard..."); */
                            return TRUE;
                        }
                    } else {
                        if ((home_ptr == mptr) || (loc[1] >> 16 == _fr_y_cen)) {
                            if (_sclip[0] == FMK_INT_SW) {
                                if (cobj->loc.y < (fr_camera_last[1] >> 8)) {
                                    _me_subclip(mptr) |= _sclip[0];
                                } // mprintf("ysubclip yep south.."); }
                            } else if (cobj->loc.y > (fr_camera_last[1] >> 8)) {
                                _me_subclip(mptr) |= _sclip[0];
                            } // mprintf("ysubclip yep north.."); }
                              //		    mprintf(" Yo: Y subclip from %x and %x, clip
                              //%x\n",cobj->loc.y,fr_camera_last[1],_sclip[0]);
                        } else {
                            _me_subclip(mptr) |= _sclip[0];
                        }             // mprintf("ysubclip standard results.."); }
                        return FALSE; // y direction
                    }
                }
        }
        curORef = objRefs[curORef].next;
    }
    return FALSE; // for now, no blockage in home square
}

uchar game_obj_block(void *vmptr, uchar *_sclip, int *loc) {
    MapElem *mptr = (MapElem *)vmptr;
    Obj *cobj;
    ObjID cobjid;
    ObjRefID curORef;

    if ((home_ptr == mptr) || (((loc[0] >> 16) == _fr_x_cen) || ((loc[1] >> 16) == _fr_y_cen)))
        return game_obj_block_home(vmptr, _sclip, loc);

    curORef = mptr->objRef;
    while (curORef != OBJ_REF_NULL) {
        cobjid = objRefs[curORef].obj;
        if (ObjProps[OPNUM(cobjid)].flags & RENDER_BLOCK) { // if we are a block type object
            cobj = &objs[cobjid];
            if (((cobj->loc.p | cobj->loc.b) == 0) && ((cobj->loc.h & 0x3f) == 0) && (cobj->info.current_frame == 0))
                if ((objRefs[curORef].state.bin.sq.x == (cobj->loc.x >> 8)) &&
                    (objRefs[curORef].state.bin.sq.y == (cobj->loc.y >> 8))) { // is it at correct heading
                    if ((cobj->loc.h + 0x20) & 0x40) {
                        _me_subclip(mptr) |= _sclip[1];
                        return TRUE; // x direction
                    } else {
                        _me_subclip(mptr) |= _sclip[0];
                        //		    mprintf("Set %d %d to %x from sclip %x\n",
                        //			     objRefs[curORef].state.bin.sq.x,objRefs[curORef].state.bin.sq.y,me_subclip(mptr),_sclip[0]);
                        return FALSE; // y direction
                    }
                }
        }
        curORef = objRefs[curORef].next;
    }
    return FALSE;
}

int game_fr_idx(void) { return _game_fr_tmap; }

#define TIM_WERE_AWAKE
#ifdef TIM_WERE_AWAKE
#define IsTpropStars() (textprops[_game_fr_tmap].force_dir > 0)
#define IsTpStarDraw() (textprops[_game_fr_tmap].force_dir == 2)
#else
#define IsTpropStars() (_game_fr_tmap < 4)
#define IsTpStarDraw() (_game_fr_tmap < 2)
#endif

// should i draw this texture/map/so on
uchar draw_tmap_p(int ptcnt) {
    // JAEMZ JAEMZ JAEMZ JAEMZ
    // notify yourself here, i would guess....
    if (IsTpropStars()) {
        if (IsTpStarDraw()) {
            // texture map, don't draw, just eval
            star_empty(ptcnt, _fdt_tmppts);
            return TRUE;
        } else {
            star_poly(ptcnt, _fdt_tmppts);
            return FALSE;
            // g3_draw_poly(152,ptcnt,_fdt_tmppts);
        }
    }

    return TRUE;
}

// SPEED THIS UP
// major changes left to do:
//  -- rewrite in assembler and stop being a wuss, self modify in drop vals and full screen adjust
//  -- currently doesnt have support for full screen adjust.. should be easy assembler though, just need the structure
//  offsets
grs_bitmap *game_fr_tmap_128(void) {
    grs_bitmap *draw_me;
    int loop = TEXTURE_128_INDEX;
    int cur_drop = _frp.view.drop_rad[0] + textprops[_game_fr_tmap].distance_mod;

    if (cur_drop < _fdt_dist) {
        cur_drop += _frp.view.drop_rad[1];
        loop++;
        if (cur_drop < _fdt_dist) {
            loop++;
            if (cur_drop + _frp.view.drop_rad[2] < _fdt_dist)
                loop++;
        }
    }

    draw_me = get_texture_map(_game_fr_tmap + ANIMTEXT_FRAME(_game_fr_tmap), loop);
    return draw_me;
}

grs_bitmap *game_fr_tmap_64(void) {
    grs_bitmap *draw_me;
    int loop = TEXTURE_64_INDEX;
    int cur_drop = _frp.view.drop_rad[1] + textprops[_game_fr_tmap].distance_mod;

    if (cur_drop < _fdt_dist) {
        loop++; // now 32
        if (cur_drop + _frp.view.drop_rad[2] < _fdt_dist)
            loop++; // now 16
    }

    draw_me = get_texture_map(_game_fr_tmap + ANIMTEXT_FRAME(_game_fr_tmap), loop);
    return draw_me;
}

grs_bitmap *game_fr_tmap_full(void) {
    grs_bitmap *draw_me;
    int loop = TEXTURE_128_INDEX, lmask;
    int cur_drop = _frp.view.drop_rad[0] + textprops[_game_fr_tmap].distance_mod;

    if (cur_drop < _fdt_dist) {
        cur_drop += _frp.view.drop_rad[1];
        loop++;
        if (cur_drop < _fdt_dist) {
            loop++;
            if (cur_drop + _frp.view.drop_rad[2] < _fdt_dist) {
                loop++;
                goto draw_it;
            }
        }
    }
    lmask = (1 << loop);
#ifdef CAN_MISS
    if (((texture_array[_game_fr_tmap].sizes_loaded) & lmask) == 0) {
        do {
            loop++;
            lmask <<= 1;
        } while ((loop < TEXTURE_16_INDEX) && (((texture_array[_game_fr_tmap].sizes_loaded) & lmask) == 0));
    }
#endif

draw_it:
    draw_me = get_texture_map(_game_fr_tmap + ANIMTEXT_FRAME(_game_fr_tmap), loop);
    return draw_me;
}

void game_rend_start(void) {
    cams *cur_cam;
    // hey, gots to do this somewhere
    // remove self from object list
    if (cam_mode == OBJ_PLAYER_CAMERA)
        no_render_obj = PLAYER_OBJ;
    else {
        cur_cam = fr_camera_getdef();
        if (cur_cam->type & CAMBIT_OBJ)
            no_render_obj = cur_cam->obj_id;
        else
            no_render_obj = -1;
    }

    render_sort_start();
}

void game_fr_clip_start(uchar headnorth) {
    if (headnorth) {
        home_ptr = MAP_GET_XY(_fr_x_cen, _fr_y_cen);
        fr_obj_block = game_obj_block;
    }
}


uchar model_base_nums[MAX_VTEXT_OBJS];

void game_fr_startup(void) {
    short curr, index;

    // we know that the main screen we support is 320x200, so.....
    // KLC   frameBufferFreePtr=frameBuffer;
    // has to fixed, clearly
    fr_set_global_callbacks(gamesys_draw_func, NULL, gamesys_render_func);
    fr_mouse_hide = rend_mouse_hide;
    fr_mouse_show = rend_mouse_show;
    fr_get_idx = game_fr_idx;
    fr_get_tmap = game_fr_tmap_full;
    fr_clip_start = game_fr_clip_start;
    fr_rend_start = game_rend_start;
    // this has to be fixed as well, should be real_ship or something
    curr = 1;
    index = 0;
    model_base_nums[0] = 0;
    while (model_vtext_data[index] != -1) // check for a -1 in what was supposedly a nice place
    {
        while (model_vtext_data[index] != -1)
            index++;                         // eat up numbers until index points at -1
        model_base_nums[curr++] = index + 1; // point at nice one beyond our -1 delimter
        index++;                             // increment past the -1
    }

    //   for (i=0; i<num_materials; i++)
    //	 { g3_set_vtext(i, material_maps+i); }
    // _fr_dbgflg_tog(SHOW_PICKUP);
    // mprintf("SizeofRef %d\n",sizeof(ObjRef));

    // initialize stars randomly for now, eventually use some resource or another
    // oops, using malloc.  Oh well
    star_set(NUM_STARS, star_vec, star_col);
    srand(0);
    star_rand(211, 10);
    std_alias_size = 400;
}

void game_fr_shutdown(void) {}

void game_fr_reparam(int is_128s, int full_scrn, int show_all) {
    if (is_128s != -1)
        fr_get_tmap = is_128s ? game_fr_tmap_128 : game_fr_tmap_64;
    switch (_frp.faces.cyber = global_fullmap->cyber) {
    case 1:
        _fr_glob_flags |= FR_SHOWALL_MASK;
        _frp.view.radius = 13;
        break;
    case 0:
        _fr_glob_flags &= ~FR_SHOWALL_MASK;
        _frp.view.radius = 18;
        break;
    }
}

void game_redrop_rad(int rad_mod) {
    _frp.view.drop_rad[0] = _frp.view.odrop_rad[0] + rad_mod;
    _frp.view.drop_rad[1] = _frp.view.odrop_rad[1] + rad_mod;
    _frp.view.drop_rad[2] = _frp.view.odrop_rad[2] + rad_mod;
}

// errtype is icky
void change_detail_level(byte new_level) { ShockPlus::Options::videoDetail = static_cast<ShockPlus::Options::VideoDetail>(new_level); }

void set_global_lighting(short l_lev) { _frp.lighting.global_mod += l_lev; }

void rendedit_process_tilemap(FullMap *fmap, LGRect *r, uchar newMap) {
    //   mprintf("RPT %d\n",new);
    if (fmap == NULL) /* support null pass in */
        fmap = global_fullmap;
    if (newMap)
        fr_compile_restart(fmap);
    fr_compile_rect(fmap, r->ul.x, r->ul.y, r->lr.x, r->lr.y, FALSE);
}

extern fauxrend_context *_sr;

// Like fr_get_at, but takes real screen coordinates.
ushort fr_get_at_raw(frc *fr, int x, int y, uchar again, uchar transp) {
    if ((fr) == NULL)
        _fr = _sr;
    else
        _fr = (fauxrend_context *)(fr);

    if (again)
        return fr_get_again(_fr, x - _fr->xtop, y - _fr->ytop);
    else
        return fr_get_at(_fr, x - _fr->xtop, y - _fr->ytop, transp);
}

    /*KLC - no longer used
    // this is a hack for render canvas memory usage....
    uchar *get_free_frame_buffer_bits(int size)
    {
       if ((size==FRAME_BUFFER_SIZE)||(size<=0))
          return frameBuffer;
       else
       {
          uchar *tmp=frameBufferFreePtr;
          if (frameBufferFreePtr-frameBuffer+size>FRAME_BUFFER_SIZE)
          {
    //	 Warning(("Mini Frame Buffers Too Big %d\n",size));
             return NULL;
          }
          frameBufferFreePtr+=size;
          return tmp;
       }
    }
    */

#define MATERIAL_BASE RES_materialMaps

void load_model_vtexts(char model_num) {
    short curr = model_base_nums[model_num];
    grs_bitmap *stupid;
    if (model_num >= MAX_VTEXT_OBJS)
        return;
    while (model_vtext_data[curr] != -1) {
        stupid = lock_bitmap_from_ref_anchor(MKREF(MATERIAL_BASE + model_vtext_data[curr], 0), NULL);
        g3_set_vtext(model_vtext_data[curr], stupid);
        curr++;
    }
}

void free_model_vtexts(char model_num) {
    short curr = model_base_nums[model_num];
    if (model_num >= MAX_VTEXT_OBJS)
        return;
    while (model_vtext_data[curr] != -1) {
        RefUnlock(MKREF(MATERIAL_BASE + model_vtext_data[curr], 0));
        curr++;
    }
}
