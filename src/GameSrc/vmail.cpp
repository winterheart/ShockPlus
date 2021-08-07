/*

Copyright (C) 2015-2018 Night Dive Studios, LLC.
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
/*
 * $Source: r:/prj/cit/src/RCS/vmail.c $
 * $Revision: 1.31 $
 * $Author: xemu $
 * $Date: 1994/11/01 09:17:43 $
 *
 */

#include "SDLFunctions.h"
#include "Shock.h"

#include "anim.h"
#include "email.h"
#include "game_screen.h"
#include "vmail.h"
#include "input.h"
#include "invent.h"
#include "mainloop.h"
#include "gameloop.h"
#include "tools.h"
#include "gametime.h"
#include "sfxlist.h"
#include "musicai.h"
#include "player.h"
#include "statics.h"
#include "fullscrn.h"
#include "render.h"
#include "gr2ss.h"
#include "criterr.h"

extern "C" {
#include "sdl_events.h"
}

uchar vmail_wait_for_input = true;

//#define CONTINUOUS_VMAIL_TEST

ActAnim *main_anim;

#define NUM_VMAIL 6
#define INTRO_VMAIL (NUM_VMAIL + 1)

byte current_vmail = -1;

Ref vmail_frame_anim[NUM_VMAIL] = {RES_FRAMES_shield, RES_FRAMES_grove,  RES_FRAMES_bridge,
                                   RES_FRAMES_laser1, RES_FRAMES_status, RES_FRAMES_explode1};

Ref vmail_res[NUM_VMAIL] = {RES_shield, RES_grove, RES_bridge, RES_laser1, RES_status, RES_explode1};

ubyte vmail_len[NUM_VMAIL] = {
    1, // shield
    1, // grove
    1, // bridge
    2, // laser
    1, // status
    5, // explode
};

#define MAX_VMAIL_SIZE 100000

// --------------------------------------------------------------------
//
//

grs_bitmap *vmail_background = nullptr;

void vmail_intro(LGRect *area, ubyte flags) {
    if (flags & BEFORE_ANIM_BITMAP) {
        if (vmail_background)
            gr_bitmap(vmail_background, 0, 0);
    }
}

// --------------------------------------------------------------------
//
//

void vmail_anim_end(ActAnim *paa, AnimCode ancode, AnimCodeData *pdata) {
#ifdef PLAYTEST
    if (current_vmail == -1) {
        Warning(("Trying to end vmail, with no current vmail!\n"));
    }
#endif
    current_vmail = -1;
}

// --------------------------------------------------------------------
//
//

void vmail_start_anim_end(ActAnim *paa, AnimCode ancode, AnimCodeData *pdata) {
#ifdef PLAYTEST
    if (current_vmail == -1) {
        Warning(("Trying to end vmail, with no current vmail!\n"));
    }
#endif
    current_vmail = -1;
}

// ---------------------------------------------
//
// play_vmail_intro()
//

errtype play_vmail_intro(uchar use_texture_buffer) {
    LGPoint animloc = {VINTRO_X, VINTRO_Y};
    uchar *p, *useBuffer;
    int bsize;
    short w, h;

    DEBUG("Playing vmail intro");

    main_anim = AnimPlayRegion(REF_ANIM_vintro, mainview_region, animloc, 0, vmail_intro);
    if (main_anim == nullptr)
        return (ERR_NOEFFECT);

    if (use_texture_buffer) {
        AnimSetDataBufferSafe(main_anim, tmap_static_mem, sizeof(tmap_static_mem));
        AnimPreloadFrames(main_anim, REF_ANIM_vintro);
    }

    // let's slork up memory!!!!
    w = VINTRO_W;
    h = VINTRO_H;
    {
        useBuffer = frameBuffer;
        bsize = sizeof(frameBuffer);
    }
    if ((w * h) + sizeof(grs_bitmap) > bsize)
        critical_error(CRITERR_MEM | 8);
    p = useBuffer + bsize - (w * h);
    vmail_background = (grs_bitmap *)(p - sizeof(grs_bitmap));

    gr_init_bitmap(vmail_background, p, BMT_FLAT8, 0, w, h);
    uiHideMouse(nullptr);
#ifdef SVGA_SUPPORT
    if (convert_use_mode) {
        grs_canvas tempcanv;
        gr_make_canvas(vmail_background, &tempcanv);
        gr_push_canvas(&tempcanv);
        gr_clear(1);
        gr_pop_canvas();
    } else
#endif
        gr_get_bitmap(vmail_background, VINTRO_X, VINTRO_Y);
    uiShowMouse(nullptr);

    AnimSetNotify(main_anim, nullptr, ANCODE_KILL, vmail_start_anim_end);
    current_vmail = INTRO_VMAIL;
    play_digi_fx(SFX_VMAIL, 1);

    while (current_vmail != -1) {
        AnimRecur();
        tight_loop(true);
    }
    vmail_background = nullptr;

    return (OK);
}

// --------------------------------------------------------------------
//
// play_vmail()
//

errtype play_vmail(byte vmail_no) {
    LGPoint animloc = {VINTRO_X, VINTRO_Y};
    errtype intro_error;
    int vmail_animfile_num = 0;
    uchar early_exit = false;
    uchar preload_animation = true;
    uchar use_texture_buffer = false;
    int len = vmail_len[vmail_no];
    int i;

    // let's extern

    DEBUG("Playing vmail %i", vmail_no);

    // the more I look at this procedure - the more I think
    // art - what were you thinking

    // make sure we don't have a current vmail, and we're given a valid vmail num
    if ((current_vmail != -1) || (vmail_no < 0) || (vmail_no >= NUM_VMAIL))
        return (ERR_NOEFFECT);

    if (full_game_3d)
        render_run();

    // spew the appropriate text for vmail - full screen needs a draw!
    suspend_game_time();
    time_passes = false;
    game_paused = true;

    // open the res file
    vmail_animfile_num = ResOpenFile("res/data/vidmail.res");
    if (vmail_animfile_num < 0)
        return (ERR_FOPEN);

    uiPushSlabCursor(&fullscreen_slab, &vmail_cursor);
    uiPushSlabCursor(&main_slab, &vmail_cursor);

    // if we're not using the texture buffer - then we can probably
    // preload the animations
    if (!use_texture_buffer) {
        uchar cant_preload_all = false;

        // load the intro in first! before checking for preloading
        if (ResLock(RES_FRAMES_vintro) == nullptr)
            use_texture_buffer = true;
        else {
            for (i = 0; i < len && !cant_preload_all; i++) {
                // preload vmail frame animation first, and then play intro -> no pause between the two
                // if it fails on the lock - then say you can't preload!
                if (ResLock(vmail_frame_anim[vmail_no] + i) == nullptr) {
                    cant_preload_all = true;
                    break;
                }
            }
            // if we failed our preloading for whatever reason let's unlock it all - drop it so that it doesn't stay
            // in memory
            if (cant_preload_all) {
                int j;
                preload_animation = false;
                for (j = 0; j < i; j++) {
                    ResUnlock(vmail_frame_anim[vmail_no] + j);
                    ResDrop(vmail_frame_anim[vmail_no] + j);
                }
                ResUnlock(RES_FRAMES_vintro);
                ResDrop(RES_FRAMES_vintro);
                use_texture_buffer = true;
            }
        }
    } else {
        preload_animation = false;
    }

    intro_error = play_vmail_intro(use_texture_buffer);
    if (preload_animation) {
        ResUnlock(RES_FRAMES_vintro);
        ResDrop(RES_FRAMES_vintro);
    }

    if (intro_error != OK) // did it have no effect - don't worry about texture buffer
    {
        // if we had a problem with the intro - then flush the animation all out - close it and
        // then return error code
        DEBUG("Could not play vmail intro");
        if (preload_animation)
            for (i = 0; i < len; i++) {
                ResUnlock(vmail_frame_anim[vmail_no] + i);
                ResDrop(vmail_frame_anim[vmail_no] + i);
            }
        ResCloseFile(vmail_animfile_num);
        return (intro_error);
    }

    for (i = 0; (i < len) && !early_exit; i++) {
        DEBUG("Playing segment.");
        Ref vmail_ref = MKREF(vmail_res[vmail_no] + i, 0);
        main_anim = AnimPlayRegion(vmail_ref, mainview_region, animloc, 0, nullptr);
        if (main_anim == nullptr) {
            early_exit = true;
            break;
        }
        if (use_texture_buffer) {
            AnimSetDataBufferSafe(main_anim, tmap_static_mem, sizeof(tmap_static_mem));
            AnimPreloadFrames(main_anim, vmail_ref);
        }
        current_vmail = vmail_no;

        AnimSetNotify(main_anim, nullptr, ANCODE_KILL, vmail_anim_end);
        uiFlush();
        while (current_vmail != -1) {
            AnimRecur();
            if (citadel_check_input()) {
                early_exit = true;
                AnimKill(main_anim);
            }
            tight_loop(true);
        }
    }

    DEBUG("Finished playing!");
    if (preload_animation) {
        for (i = 0; i < len; i++) {
            ResUnlock(vmail_frame_anim[vmail_no] + i);
            ResDrop(vmail_frame_anim[vmail_no] + i);
        }
    }
    ResCloseFile(vmail_animfile_num);

    uiFlush();

    if (use_texture_buffer) {
        load_textures();
    }

#ifndef CONTINUOUS_VMAIL_TEST
    if (!early_exit && vmail_wait_for_input)
        while (!citadel_check_input()) {
            // trick to prevent mouse pointer freeze
            uiHideMouse(nullptr);
            uiShowMouse(nullptr);
            pump_events();
            SDLDraw();
            tight_loop(false); // keep the music playing
        }
#endif

    email_page_exit();

    screen_draw();
    inventory_draw_new_page(old_invent_page);

    DEBUG("Resuming game time");
    resume_game_time();
    game_paused = false;

    // let the player wait before firing auto fire weapon
    player_struct.auto_fire_click = player_struct.game_time + 60;
    time_passes = true;

    uiPopSlabCursor(&fullscreen_slab);
    uiPopSlabCursor(&main_slab);
    chg_set_flg(DEMOVIEW_UPDATE);

    return (OK);
}

byte test_vmail = 0;

uchar shield_test_func(short keycode, ulong context, void *data) {
    int i;
    vmail_wait_for_input = false;
    for (i = 0; i < 5; i++) {
        play_vmail(test_vmail);
        test_vmail = (test_vmail + 1) % NUM_VMAIL;
    }
    vmail_wait_for_input = true;
    return (true);
}

#ifdef PLAYTEST
uchar shield_off_func(short keycode, ulong context, void *data) { return (true); }

#endif
