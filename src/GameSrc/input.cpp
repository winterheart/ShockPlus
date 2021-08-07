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
 * $Source: r:/prj/cit/src/RCS/input.c $
 * $Revision: 1.293 $
 * $Author: jaemz $
 * $Date: 1994/11/23 00:16:31 $
 */

#include <cstdio>
#include <cstring>
#include <climits>

#include "Engine/Options.h"

#include "Prefs.h"
#include "Shock.h"
#include "ShockBitmap.h"
#include "InitMac.h"
#include "input.h"
#include "ai.h"
#include "aiflags.h"
#include "citres.h"
#include "colors.h"
#include "cybstrng.h"
#include "doorparm.h"
#include "emailbit.h"
#include "faketime.h"
#include "fatigue.h"
#include "frflags.h" // until we do the right thing re: static
#include "frintern.h"
#include "frtypes.h"
#include "frutils.h"
#include "fullscrn.h"
#include "gamesys.h"
#include "gamescr.h"
#include "gamestrn.h"
#include "gr2ss.h"
#include "grenades.h"
#include "hotkey.h"
#include "invent.h"
#include "mainloop.h"
#include "movekeys.h"
#include "objbit.h"
#include "objects.h"
#include "objload.h"
#include "objsim.h"
#include "objprop.h"
#include "objuse.h"
#include "otrip.h"
#include "physics.h"
#include "player.h"
#include "render.h"
#include "rendtool.h"
#include "game_screen.h"
#include "svgacurs.h"
#include "textmaps.h"
#include "tools.h"
#include "weapons.h"
#include "mouselook.h"
#include "OpenGL.h"

// -------
// DEFINES
// -------

ubyte use_distance_mod = 0;
ubyte pickup_distance_mod = 0;
ubyte fatigue_threshold = 5;

#define FATIGUE_COEFF CIT_CYCLE
#define FATIGUE_THRESHOLD \
    ((player_struct.drug_status[CPTRIP(STAMINA_DRUG_TRIPLE)] == 0) ? (fatigue_threshold * CIT_CYCLE) : MAX_FATIGUE)
#define PLAYER_FATIGUE \
    ((player_struct.fatigue > FATIGUE_THRESHOLD) ? (player_struct.fatigue - FATIGUE_THRESHOLD) / FATIGUE_COEFF : 0)

#define MOTION_FOOTPLANT_SCANCODE 0x2A

#define AIM_SCREEN_MARGIN 5

static ushort mouse_constrain_bits = 0;

#define FIREKEY_CONSTRAIN_BIT 1
#define LBUTTON_CONSTRAIN_BIT MOUSE_LDOWN
#define RBUTTON_CONSTRAIN_BIT MOUSE_RDOWN
#define LOCK_CONSTRAIN_BIT 0x8000

typedef struct mouse3d_stuff {
    uchar ldown;
    uchar rdown;
    int lastsect;
    LGPoint lastleft;
    LGPoint lastright;
    frc *fr;
} view3d_data;

// -------
// GLOBALS
// -------

Ref motion_cursor_ids[] = {
    REF_IMG_bmUpLeftCursor,   REF_IMG_bmUpCursor,     REF_IMG_bmUpRightCursor,   0,
    REF_IMG_bmLeftCursor,     REF_IMG_bmDownCursor,   REF_IMG_bmRightCursor,     0,
    REF_IMG_bmCircLeftCursor, REF_IMG_bmTargetCursor, REF_IMG_bmCircRightCursor, 0,
    REF_IMG_bmUpLeftCursor,   REF_IMG_bmSprintCursor, REF_IMG_bmUpRightCursor,
};

#define NUM_CYBER_CURSORS 9
#define CYBER_CURSOR_BASE REF_IMG_bmCyberUpLeftCursor

LGCursor motion_cursors[NUM_MOTION_CURSORS];
grs_bitmap motion_cursor_bitmaps[NUM_MOTION_CURSORS];

int input_cursor_mode = INPUT_NORMAL_CURSOR;
int throw_oomph = 5;

fix inpJoystickSens = FIX_UNIT;

LGPoint use_cursor_pos;

// and joysticks, heck, why be efficient
uchar joystick_count = 0;

// -------------
//  PROTOTYPES
// -------------
void handle_keyboard_fatigue();
void poll_mouse();
// uchar eye_hotkey_func(ushort keycode, uint32_t context, intptr_t data);

int view3d_mouse_input(LGPoint pos, LGRegion *reg, uchar move, int *lastsect);
void view3d_dclick(LGPoint pos, frc *fr, bool shifted);
uchar view3d_mouse_handler(uiEvent *ev, LGRegion *r, intptr_t data);
void view3d_rightbutton_handler(uiEvent *ev, LGRegion *r, view3d_data *data);
uchar view3d_key_handler(uiEvent *ev, LGRegion *r, intptr_t data);
void use_object_in_3d(ObjID obj, bool shifted);

// uchar MacResFunc(ushort keycode, uint32_t context, intptr_t data);
// uchar MacSkiplinesFunc(ushort keycode, uint32_t context, intptr_t data);

// EXTERN FUNCTIONS

// -------------
// INPUT POLLING
// -------------

void handle_keyboard_fatigue() {
    byte cval;
    physics_get_one_control(KEYBD_CONTROL_BANK, CONTROL_YVEL, &cval);
    if (cval > 0) {
        int f = lg_max(CONTROL_MAX_VAL - PLAYER_FATIGUE, SPRINT_CONTROL_THRESHOLD);
        if (cval > f)
            physics_set_one_control(KEYBD_CONTROL_BANK, CONTROL_YVEL, f);
    }
    physics_get_one_control(KEYBD_CONTROL_BANK, CONTROL_ZVEL, &cval);
    if (cval > 0) {
        int f = lg_max(MAX_JUMP_CONTROL - PLAYER_FATIGUE, MAX_JUMP_CONTROL / 2);
        if (cval > f)
            physics_set_one_control(KEYBD_CONTROL_BANK, CONTROL_ZVEL, f);
    }
}

// Sends a motion event to the 3d view.

bool view3d_got_event = false;

void poll_mouse() {
    if (_current_view != nullptr) {
        uiEvent ev;
        uiMakeMotionEvent(&ev);
        ev.type = UI_EVENT_USER_DEFINED;
        mouse_constrain_bits |= LOCK_CONSTRAIN_BIT;
        uiDispatchEventToRegion(&ev, _current_view);
        mouse_constrain_bits &= ~LOCK_CONSTRAIN_BIT;
    }
}

bool checking_mouse_button_emulation = false;
bool mouse_button_emulated = false;

uchar citadel_check_input() {
    if (uiCheckInput())
        return true;

    if (checking_mouse_button_emulation)
        mouse_button_emulated = false;

    // if we're suppose to emulate a mouse button - let's do it!
    if (mouse_button_emulated)
        return true;
    return false;
}

void input_chk() {
    setup_motion_polling();
    view3d_got_event = false;
    uiPoll();
    if (!view3d_got_event)
        poll_mouse();

    process_motion_keys();
    handle_keyboard_fatigue();
}

uchar main_kb_callback(uiEvent *h, LGRegion *r, intptr_t udata) {
#ifdef INPUT_CHAINING
    kb_flush_bios();
#endif // INPUT_CHAINING

    // Broad event types related to KB
    if (h->sdl_data.type == SDL_KEYDOWN || h->sdl_data.type == SDL_KEYUP) {
        int result = hotkey_dispatch(h->subtype);
        if (result == OK) {
            DEBUG("%s: dispatched hotkey %d (SDL: %d, %d, %d)", __FUNCTION__, h->subtype, h->sdl_data.key.type,
                  h->sdl_data.key.state, h->sdl_data.key.keysym.scancode);
        }
        return (result == OK);
    }
    return false;
}

uchar posture_hotkey_func(ushort keycode, uint32_t context, intptr_t data) {
    return player_set_posture((unsigned int)data) == OK;
}

/*
uchar eye_hotkey_func(ushort keycode, uint32_t context, intptr_t data) {
    byte eyectl = player_get_eye();
    int r = 1 + (player_struct.drug_status[DRUG_REFLEX] > 0 && !global_fullmap->cyber);

    if (data == 0) {
        player_set_eye(0);
        return true;
    }
    for (; r > 0; r--) {
        if (data < 0) {
            if (eyectl > 0)
                eyectl = 0;
            else
                eyectl = (eyectl - CONTROL_MAX_VAL) / 3;
        } else {
            if (eyectl < 0)
                eyectl = 0;
            else
                eyectl = (eyectl + CONTROL_MAX_VAL) / 3;
        }
    }
    player_set_eye(eyectl);
    return true;
}
*/

#define EYE_POLLING
#ifndef EYE_POLLING
static ushort eye_up_keys[] = {
    KEY_UP | KB_FLAG_SHIFT,
    KEY_PAD_UP | KB_FLAG_SHIFT,
    'r',
    'R',
};

#define NUM_EYE_UP_KEYS (sizeof(eye_up_keys) / sizeof(ushort))

static ushort eye_dn_keys[] = {
    KEY_DOWN | KB_FLAG_SHIFT,
    KEY_PAD_DOWN | KB_FLAG_SHIFT,
    'v',
    'V',
};

#define NUM_EYE_DN_KEYS (sizeof(eye_dn_keys) / sizeof(ushort))
#endif // !EYE_POLLING

// -------------------------------------
// INITIALIZATION
#ifdef PLAYTEST
extern uchar automap_seen(ushort keycode, uint32_t context, intptr_t data);
extern uchar maim_player(ushort keycode, uint32_t context, intptr_t data);
extern uchar salt_the_player(ushort keycode, uint32_t context, intptr_t data);
extern uchar give_player_hotkey(ushort keycode, uint32_t context, intptr_t data);
extern uchar change_clipper(ushort keycode, uint32_t context, intptr_t data);
#endif

void reload_motion_cursors(uchar cyber) {
    for (int i = 0; i < NUM_MOTION_CURSORS; i++) {
        grs_bitmap *bm = &motion_cursor_bitmaps[i];
        if (bm->bits != nullptr) {
            free(bm->bits);
            memset(bm, 0, sizeof(grs_bitmap));
        }
    }

    if (!cyber) {
        for (int i = 0; i < NUM_MOTION_CURSORS; i++) {
            grs_bitmap *bm = &motion_cursor_bitmaps[i];
            if (motion_cursor_ids[i] != 0)
                load_res_bitmap_cursor(&motion_cursors[i], bm, motion_cursor_ids[i], true);
        }

        // slam the cursor color back to it's childhood colors
        cursor_color_offset = RED_BASE + 4;

        SetMotionCursorsColorForActiveWeapon();
    } else {
        for (int i = 0; i < NUM_CYBER_CURSORS; i++) {
            grs_bitmap *bm = &motion_cursor_bitmaps[i];
            load_res_bitmap_cursor(&motion_cursors[i], bm, CYBER_CURSOR_BASE + i, true);
        }
    }
}

void free_cursor_bitmaps() {
    // reload_motion_cursors() does everything now
}

void alloc_cursor_bitmaps() {
    // reload_motion_cursors() does everything now

    // I would just like to point out that this function
    // was a good example of what were they thinking?
}

// bool gShowFrameCounter = false;
// bool gShowMusicGlobals = false;

/*
uchar MacResFunc(ushort keycode, uint32_t context, intptr_t data) {
    ShockPlus::Options::halfResolution = !ShockPlus::Options::halfResolution;
    change_svga_screen_mode();

    if (ShockPlus::Options::halfResolution)
        message_info("Low res.");
    else {
        message_info("High res.");
        ShockPlus::Options::enableSkipLines = false;
    }
    ShockPlus::Options::save();

    return true;
}
*/

/*
uchar MacSkiplinesFunc(ushort keycode, uint32_t context, intptr_t data) {
    // Skip lines only applies in double-size mode.
    if (!ShockPlus::Options::halfResolution) {
        message_info("Skip lines works only in low-res mode.");
        return false;
    }
    ShockPlus::Options::enableSkipLines = !ShockPlus::Options::enableSkipLines;
    ShockPlus::Options::save();
    return true;
}
*/

uchar MacDetailFunc(ushort keycode, uint32_t context, intptr_t data) {
    char msg[32];
    char detailStr[8];
    fauxrend_context *_frc = (fauxrend_context *)svga_render_context;

    // Adjust for that global detail nonsense.
    if (_frc->detail == 4)
        _frc->detail = ShockPlus::Options::videoDetail;

    _frc->detail++; // Cycle through the detail levels.
    if (_frc->detail >= 4)
        _frc->detail = 0;
    // Update the global guy.
    ShockPlus::Options::videoDetail = static_cast<ShockPlus::Options::VideoDetail>(_frc->detail);
    // Update and save our prefs.
    ShockPlus::Options::save();

    // Show a nice, informative message.
    switch (_frc->detail) {
    case ShockPlus::Options::VIDEO_DETAIL_MIN:
        strcpy(detailStr, "Min");
        break;
    case ShockPlus::Options::VIDEO_DETAIL_LOW:
        strcpy(detailStr, "Low");
        break;
    case ShockPlus::Options::VIDEO_DETAIL_HIGH:
        strcpy(detailStr, "High");
        break;
    case ShockPlus::Options::VIDEO_DETAIL_MAX:
        strcpy(detailStr, "Max");
    }
    sprintf(msg, "Detail level: %s", detailStr);
    message_info(msg);
    return true;
}

uchar toggle_opengl_func(ushort keycode, uint32_t context, intptr_t data) {
    toggle_opengl();
    return true;
}

void init_input() {
    uiDoubleClickDelay = 8;
    uiDoubleClickTime = 45;
    uiDoubleClicksOn[MOUSE_LBUTTON] = true; // turn on left double clicks
    uiAltDoubleClick = true;

    alloc_cursor_bitmaps();
    reload_motion_cursors(false);
}

void shutdown_input() {
    hotkey_shutdown();
    kb_flush_bios();

    // kb_clear_state(0x1d, 3);
    // kb_clear_state(0x9d, 3);
    // kb_clear_state(0x38, 3);
    // kb_clear_state(0xb8, 3);
}

// ------------------------
// 3D VIEW/MOTION INTERFACE
// ------------------------

// -------
// DEFINES
// -------

#define VIEW_LSIDE 0
#define VIEW_HCENTER 1
#define VIEW_RSIDE 2

#define VIEW_TOP 0
#define VIEW_BOTTOM 4
#define VIEW_VCENTER 8
#define VIEW_WAYTOP 12

#define CYBER_VIEW_TOP 0
#define CYBER_VIEW_CENTER 3
#define CYBER_VIEW_BOTTOM 6

#define CENTER_WD_N 1
#define CENTER_WD_D 8
#define CYBER_CENTER_WD_D 6
#define CENTER_HT_N 1
#define CENTER_HT_D 8
#define CYBER_CENTER_HT_D 6

// -------
// GLOBALS
// -------

short object_on_cursor = 0;
LGCursor object_cursor;

// ------------------------------------------------------------------------------
// view3d_rightbutton_handler deals with firing/throwing objects in 3d.

uchar mouse_jump_ui = true;
uchar fire_slam = false;
uchar left_down_jump = false;

void reset_input_system() {
    if (fire_slam) {
        if (full_game_3d)
            uiPopSlabCursor(&fullscreen_slab);
        else
            uiPopSlabCursor(&main_slab);
        fire_slam = false;
    }
    mouse_unconstrain();
}

#define DROP_REGION_Y(reg) ((reg)->abs_y + 7 * RectHeight((reg)->r) / 8)
uchar weapon_button_up = true;

// ---------
// INTERNALS
// ---------

// -------------------------------------------------------------------------------------------
// CalcMotionCurOffset gets cursor position offset data for
//   SetMotionCursorForMouseXY() and view3d_mouse_input()
void CalcMotionCurOffset(uchar cyber, LGRegion *reg, short *cx, short *cy, short *cw, short *ch, short *x, short *y) {
    if (ShockPlus::Options::halfResolution) {
        (*x) *= 2;
        (*y) *= 2;
    }

    if (!cyber) {
        (*cx) = reg->abs_x + RectWidth(reg->r) / 2;
        (*cy) = reg->abs_y + 2 * RectHeight(reg->r) / 3;
        (*cw) = RectWidth(reg->r) * CENTER_WD_N / CENTER_WD_D;
        (*ch) = RectHeight(reg->r) * CENTER_HT_N / CENTER_HT_D;
    } else {
        (*cx) = reg->abs_x + RectWidth(reg->r) / 2;
        (*cy) = reg->abs_y + RectHeight(reg->r) / 2;
        (*cw) = RectWidth(reg->r) * CENTER_WD_N / CYBER_CENTER_WD_D;
        (*ch) = RectHeight(reg->r) * CENTER_HT_N / CYBER_CENTER_HT_D;
    }

#ifdef SVGA_SUPPORT
    ss_point_convert(cx, cy, false);
    ss_point_convert(cw, ch, false);
#endif

    (*x) -= (*cx);
    (*y) -= (*cy);
}

// -------------------------------------------------------------------------------------------
// SetMotionCursorForMouseXY sets motion cursor for current mouse x,y position
// Used to set cursor to weapon color immediately without having to move the mouse
void SetMotionCursorForMouseXY() {
    if (global_fullmap->cyber)
        return;

    int cnum;

    LGRegion *reg;

    if (full_game_3d)
        reg = fullview_region;
    else
        reg = mainview_region;

    if (mlook_enabled)
        cnum = VIEW_HCENTER | VIEW_VCENTER;
    else {
        short cx, cy, cw, ch, x, y;

        mouse_get_xy(&x, &y);

        CalcMotionCurOffset(false, reg, &cx, &cy, &cw, &ch, &x, &y);

        if (x < -cw)
            cnum = VIEW_LSIDE;
        else if (x > cw)
            cnum = VIEW_RSIDE;
        else
            cnum = VIEW_HCENTER;

        if (y < -ch)
            cnum |= VIEW_TOP;
        else if (y > ch)
            cnum |= VIEW_BOTTOM;
        else
            cnum |= VIEW_VCENTER;
    }

    LGCursor *c = &motion_cursors[cnum];

    if (reg == fullview_region)
        uiSetGlobalDefaultCursor(c);
    else
        uiSetRegionDefaultCursor(reg, c);
}

// -------------------------------------------------------------------------------------------
// view3d_mouse_input sets/unsets physics controls based on mouse position in 3d
// return whether any control was applied
int view3d_mouse_input(LGPoint pos, LGRegion *reg, uchar move, int *lastsect) {
    // do we really recompute these every frame?? couldnt we have a context or something...
    // something, a call to reinit, something
    static int dougs_goofy_hack = false;

    int cnum = 0;
    byte xvel = 0;
    byte yvel = 0;
    byte xyrot = 0;
    uchar thrust = false;
    uchar cyber = global_fullmap->cyber && time_passes;

    short cx, cy, cw, ch, x, y;

    x = pos.x;
    y = pos.y;

    CalcMotionCurOffset(cyber, reg, &cx, &cy, &cw, &ch, &x, &y);

    // ok, the idea here is to make sure single left click doesnt move, or at least tells you whats up...
    if ((dougs_goofy_hack == false) && move) {
        dougs_goofy_hack = true;
        move = false;
    } else if (!move)
        dougs_goofy_hack = false;

    if (x < -cw) {
        cnum = VIEW_LSIDE;
        if (move) {
            xyrot = (x + cw) * 100 / (cx - cw - reg->abs_x);
        }
    } else if (x > cw) {
        cnum = VIEW_RSIDE;
        if (move)
            xyrot = (x - cw) * 100 / (cx - cw - reg->abs_x);
    } else
        cnum = VIEW_HCENTER;

    if (cyber) {
        if (y < -ch) {
            if (move)
                yvel = -(-ch - y) * CONTROL_MAX_VAL / (cy - ch - reg->abs_y);
            cnum += CYBER_VIEW_TOP;
        } else if (y > ch) {
            cnum += CYBER_VIEW_BOTTOM;
            if (move) {
#ifdef CYBER_ROLL_REGION
                if (xyrot == 0)
#endif // CYBER_ROLL_REGION
                    yvel = -(ch - y) * CONTROL_MAX_VAL / (cy - ch - reg->abs_y);
#ifdef CYBER_ROLL_REGION
                else {
                    xvel = xyrot;
                    xyrot = 0;
                }
#endif // CYBER_ROLL_REGION
            }
        } else {
            if ((thrust = ((cnum == VIEW_HCENTER) && move)) == true)
                physics_set_one_control(MOUSE_CONTROL_BANK, CONTROL_ZVEL, MAX_JUMP_CONTROL);

            cnum += CYBER_VIEW_CENTER;
        }
    } else {
        if (y < -ch) {
            short ycntl = (-ch - y) * CONTROL_MAX_VAL / (cy - ch - reg->abs_y);
            if (move) {
                int f = PLAYER_FATIGUE;
                if (ycntl + f > CONTROL_MAX_VAL) {
                    // compute new mouse cursor position
                    f = lg_max(CONTROL_MAX_VAL - f, SPRINT_CONTROL_THRESHOLD);
                    int newy = f * (ch + reg->abs_y - cy) / CONTROL_MAX_VAL - ch + cy;
                    ycntl = (ycntl + f) / 2;
                    // put the cursor between here and there
                    if (newy > pos.y)
                        mouse_put_xy(pos.x, newy);
                }
                yvel = ycntl;
            }

            if (ycntl > SPRINT_CONTROL_THRESHOLD)
                cnum |= VIEW_WAYTOP;
            else
                cnum |= VIEW_TOP;

        } else if (y > ch) {
            cnum |= VIEW_BOTTOM;
            if (move) {
                if (xyrot == 0)
                    yvel = (ch - y) * CONTROL_MAX_VAL / (cy - ch - reg->abs_y);
                else {
                    xvel = xyrot;
                    xyrot = 0;
                }
            }
        } else
            cnum |= VIEW_VCENTER;
    }

    // If mouse look is enabled, just use the centered cursor
    if (mlook_enabled) {
        cnum = VIEW_HCENTER | VIEW_VCENTER;

        if (cyber)
            cnum = VIEW_HCENTER + CYBER_VIEW_CENTER;
    }

    if (*lastsect != cnum) {
        LGCursor *c = &motion_cursors[cnum];
        // Warning(("hey, cursor num = %d!\n",cnum));

        // set the cursor to the motion cursor
        if (reg == fullview_region)
            uiSetGlobalDefaultCursor(c);
        else
            uiSetRegionDefaultCursor(reg, c);

        *lastsect = cnum;
    }

    if (!thrust)
        physics_set_player_controls(MOUSE_CONTROL_BANK, xvel, yvel, CONTROL_NO_CHANGE, xyrot, CONTROL_NO_CHANGE,
                                    CONTROL_NO_CHANGE);

    if (dougs_goofy_hack)
        return xvel | yvel | xyrot;
    return 0;
}

// Not a directly-installed mouse handler, called from view3d_mouse_handler
void view3d_rightbutton_handler(uiEvent *ev, LGRegion *r, view3d_data *data) {
    LGPoint aimpos = ev->pos;

    // If double sizing, convert the y to 640x480, then half it. The x stays as is.
    if (ShockPlus::Options::halfResolution)
        aimpos.y = SCONV_Y(aimpos.y) >> 1;
    else
        ss_point_convert(&(aimpos.x), &(aimpos.y), false);

    // Don't do nuthin if we're in a hack camera
    if (hack_takeover)
        return;

    if (ev->mouse_data.action & MOUSE_RUP) {
        if (!data->rdown)
            data->lastright = aimpos;
        else
            data->rdown = false;
        left_down_jump = false;
        weapon_button_up = true;
        if (fire_slam) {
            if (full_game_3d)
                uiPopSlabCursor(&fullscreen_slab);
            else
                uiPopSlabCursor(&main_slab);
            fire_slam = false;
        }
    }

    if (ev->mouse_data.action & MOUSE_RDOWN) {
        data->rdown = true;
        data->lastright = aimpos;
        left_down_jump = data->ldown && !global_fullmap->cyber;
        // view3d_constrain_mouse(r,RBUTTON_CONSTRAIN_BIT);
    }

    /*
       if (mouse_jump_ui && data->ldown && !global_fullmap->cyber)
       {
               if (ev->action & MOUSE_RDOWN)
          {
                  physics_set_one_control(MOUSE_CONTROL_BANK,CONTROL_ZVEL, MAX_JUMP_CONTROL);
             return;
          }
       }
    */

    switch (input_cursor_mode) {
    case INPUT_NORMAL_CURSOR:
        if (!global_fullmap->cyber && (player_struct.fire_rate == 0) && !(ev->mouse_data.action & MOUSE_RDOWN))
            break;
        if (left_down_jump)
            break;
        if (data->rdown) {
            // printf("FIRE WEAPON!\n");
            if (fire_player_weapon(&aimpos, r, weapon_button_up) && (ev->mouse_data.action & MOUSE_RDOWN) &&
                !fire_slam) {
                if (full_game_3d)
                    uiPushSlabCursor(&fullscreen_slab, &fire_cursor);
                else
                    uiPushSlabCursor(&main_slab, &fire_cursor);
                fire_slam = true;
            }
            weapon_button_up = false;
        }
        break;
    case INPUT_OBJECT_CURSOR:
        if (ev->mouse_data.action & MOUSE_RUP) {
            fix vel = throw_oomph * FIX_UNIT;
            short dropy = DROP_REGION_Y(r);
            short y = aimpos.y;
            // if (convert_use_mode != 0)
            // If double sizing, convert the y to 640x480, then half it. The x stays as is.
            if (ShockPlus::Options::halfResolution)
                dropy = SCONV_Y(dropy) >> 1;
            else
                dropy = SCONV_Y(dropy);
            if (y >= dropy && data->lastright.y >= dropy) {
                vel = 0;
            }
            if (player_throw_object(object_on_cursor, aimpos.x, y, data->lastright.x, data->lastright.y, vel)) {
                pop_cursor_object();
                uiShowMouse(nullptr); // KLC - added to make sure new cursor shows.
            }
            data->rdown = false;
        }
        break;
    }
}

// ----------------------------------------------------------------
// use_object_in_3d deals with double-clicking on an object in the 3d
uchar check_object_dist(ObjID obj1, ObjID obj2, fix crit) {
    fix critrad = ID2radius(obj2);
    fix dx = fix_from_obj_coord(objs[obj1].loc.x) - fix_from_obj_coord(objs[obj2].loc.x);
    fix dy = fix_from_obj_coord(objs[obj1].loc.y) - fix_from_obj_coord(objs[obj2].loc.y);
    fix dz = fix_from_obj_height(obj1) - fix_from_obj_height(obj2);
    if (-dz > critrad / 2 && -dz < critrad + FIX_UNIT / 4) {
        crit *= 2;
    }
    bool retval = fix_fast_pyth_dist(dx, dy) < crit;
    if (retval) {
        retval = -(critrad * 2 + crit / 2) < dz && dz < crit / 2 + critrad * 2;
    }
    return retval;
}

#define TELE_ROD_DIST 16 // 16 feet

void use_object_in_3d(ObjID obj, bool shifted) {
    bool success = false;
    ObjID telerod = OBJ_NULL;
    bool showname = false;
    int mode = USE_MODE(obj);
    char buf[80];
    Ref usemode = ID_NULL;

    if (global_fullmap->cyber) {
        if (ID2TRIP(obj) != INFONODE_TRIPLE) {
            switch (USE_MODE(obj)) {
            case USE_USE_MODE:
                usemode = REF_STR_PhraseUse;
                break;
            case PICKUP_USE_MODE:
                usemode = REF_STR_PhrasePickUp;
                break;
            }
            // exceptions
            switch (objs[obj].obclass) {
            case CLASS_BIGSTUFF:
                usemode = REF_STR_PhrasePickUp;
                break;
            case CLASS_CRITTER:
                usemode = ID_NULL;
                break;
            }
            if (usemode != ID_NULL) {
                sprintf(buf, get_temp_string(REF_STR_CyberspaceUse), get_temp_string(usemode));
                message_info(buf);
            }
            return;
        }
    }

    if (input_cursor_mode == INPUT_OBJECT_CURSOR) {
        mode = USE_USE_MODE;
        if (ID2TRIP(object_on_cursor) == ROD_TRIPLE) {
            telerod = object_on_cursor;
            object_on_cursor = OBJ_NULL;
            use_distance_mod += TELE_ROD_DIST;
        }
    }

    switch (ID2TRIP(obj)) {

    case TMAP_TRIPLE:
        get_texture_use_string(loved_textures[objBigstuffs[objs[obj].specID].data2], buf, 80);
        message_info(buf);
        return;
    case BRIDGE_TRIPLE: {
        int dat = ((objBigstuffs[objs[obj].specID].data1) >> 16) & 0xFF;
        if (dat & 0x80) {
            get_texture_name(loved_textures[dat & (~0x80)], buf, 80);
            message_info(buf);
            return;
        }
        break;
    }
    }

    switch (mode) {
    case PICKUP_USE_MODE: {
        ObjLocState del_loc_state;

        if (!check_object_dist(obj, PLAYER_OBJ, MAX_PICKUP_DIST)) {
            string_message_info(REF_STR_PickupTooFar);
            showname = false;
            break;
        }
        // yank the object out of the map.
        del_loc_state.obj = obj;
        del_loc_state.loc = objs[obj].loc;
        del_loc_state.loc.x = -1;
        ObjRefStateBinSetNull(del_loc_state.refs[0].bin);
        ObjUpdateLocs(&del_loc_state);
        if (objs[obj].info.ph != -1) {
            EDMS_kill_object(objs[obj].info.ph);
            physics_handle_id[objs[obj].info.ph] = OBJ_NULL;
            objs[obj].info.ph = -1;
        }
        // Put it on the cursor
        // showname = true;
        push_cursor_object(obj);

        if (objs[obj].obclass == CLASS_GRENADE)
            grenade_contact(obj, INT_MAX);

        if (shifted) {
            absorb_object_on_cursor(0, 0, 0); // parameters unused
        } else {
            mouse_look_off();
        }

        success = true;
    } break;
    case USE_USE_MODE:
        showname = false;
        if (objs[obj].obclass != CLASS_CRITTER && ID2TRIP(obj) != MAPNOTE_TRIPLE &&
            !check_object_dist(obj, PLAYER_OBJ, MAX_USE_DIST)) {
            string_message_info(REF_STR_UseTooFar);
            break;
        }

        ObjectUseShifted = shifted;
        if (!object_use(obj, false, object_on_cursor)) {
            if (objs[obj].obclass != CLASS_DOOR)
                goto cantuse;
            else
                showname = true;
        }
        if (telerod != OBJ_NULL) {
            object_on_cursor = telerod;
            use_distance_mod -= TELE_ROD_DIST;
        }
        success = true;

        break;
    cantuse:
    default: {
        char use_str[80], buf2[50];
        sprintf(use_str, get_temp_string(REF_STR_CantUse), get_object_lookname(obj, buf2, 50));
        message_info(use_str);
    } break;
    }
    if (success && !global_fullmap->cyber) {
        objs[obj].info.inst_flags |= OLH_INST_FLAG;
    }
    if (showname)
        look_at_object(obj);
}

//-------------------------------------------------------------------------
// look_at_object prints a descriptive string of the object in the message line
// these are just cribbed here from email.c...
#define EMAIL_BASE_ID RES_email0
#define TITLE_IDX 1
#define SENDER_IDX 2

char *get_object_lookname(ObjID id, char use_string[], int sz) {
    int ref = -1;
    int l;
    int usetrip = ID2TRIP(id);

    strcpy(use_string, "");

    switch (objs[id].obclass) {
    case CLASS_FIXTURE:
    case CLASS_DOOR:
        if (objs[id].info.make_info != 0)
            ref = REF_STR_Name0 + objs[id].info.make_info;
        break;
    case CLASS_GRENADE:
        if (objGrenades[objs[id].specID].flags & GREN_ACTIVE_FLAG) {
            get_string(REF_STR_WordLiveGrenade, use_string, sz);
            l = strlen(use_string);
            if (l + 1 < sz)
                use_string[l] = ' ';
        }
        break;
    case CLASS_SOFTWARE:
        if (objs[id].subclass == SOFTWARE_SUBCLASS_DATA) {
            short cont = objSoftwares[objs[id].specID].data_munge;
            short num = cont & 0xFF;
            if (global_fullmap->cyber) {
                ref = REF_STR_DataObj;
                break;
            }
            switch (cont >> 8) {
            case LOG_VER:
                num += NUM_EMAIL_PROPER;
                break;
            case DATA_VER:
                num += (NUM_EMAIL - NUM_DATA);
                break;
            }
            ref = MKREF(EMAIL_BASE_ID + num, TITLE_IDX);
        }
        break;
    case CLASS_BIGSTUFF:
        if (global_fullmap->cyber) {
            usetrip =
                MAKETRIP(CLASS_SOFTWARE, objBigstuffs[objs[id].specID].data1, objBigstuffs[objs[id].specID].data2);
        } else {
            switch (ID2TRIP(id)) {
            case ICON_TRIPLE:
                ref = REF_STR_IconName0 + objs[id].info.current_frame;
                break;
            case TMAP_TRIPLE:
                get_texture_name(loved_textures[objBigstuffs[objs[id].specID].data2], use_string, sz);
                return (use_string);
            case BRIDGE_TRIPLE: {
                int dat = ((objBigstuffs[objs[id].specID].data1) >> 16) & 0xFF;
                if (dat & 0x80) {
                    get_texture_name(loved_textures[dat & (~0x80)], use_string, sz);
                    return (use_string);
                }
                break;
            }
            }
            if (ref < 0) {
                if (objs[id].info.make_info != 0)
                    ref = REF_STR_Name0 + objs[id].info.make_info;
            }
        }
        break;
    case CLASS_SMALLSTUFF: {
        switch (ID2TRIP(id)) {
        case PERSCARD_TRIPLE: {
            char buf[50];
            int acc = objSmallstuffs[objs[id].specID].data1;
#define PERSONAL_BITS_SHIFT 24
            // get rid of all but personal access bits
            get_object_long_name(ID2TRIP(id), use_string, sz);
            acc = acc >> PERSONAL_BITS_SHIFT;
            ref = PERSONAL_BITS_SHIFT;
            if (acc == 0)
                return (use_string);
            for (; (acc & 1) == 0; acc = acc >> 1)
                ref++;
            ref = MKREF(RES_accessCards, (ref << 1) + 1);
            get_string(ref, buf, sizeof(buf));
            int len = strlen(buf);
            while (!isspace(buf[len]) && len > 0)
                len--;
            if (isspace(buf[len])) {
                strcat(use_string, "-");
                strcat(use_string, buf + len + 1);
            }
            return (use_string);
        }
        case HEAD_TRIPLE:
        case HEAD2_TRIPLE:
            if (objs[id].info.make_info != 0)
                ref = REF_STR_Name0 + objs[id].info.make_info;
            break;
        }
    } break;
    case CLASS_HARDWARE: {
        sprintf(use_string, "%s v%d", get_object_long_name(ID2TRIP(id), nullptr, sz),
                objHardwares[objs[id].specID].version);
        return (use_string);
    }
    case CLASS_CRITTER: {
        char temp[128];
        Ref mod_refid = -1;
        if (objCritters[objs[id].specID].orders == AI_ORDERS_SLEEP)
            mod_refid = REF_STR_Sleeping;
        else if (objCritters[objs[id].specID].flags & AI_FLAG_TRANQ)
            mod_refid = REF_STR_Drugged;
        else if (objCritters[objs[id].specID].flags & AI_FLAG_CONFUSED)
            mod_refid = REF_STR_Stunned;
        if (mod_refid != -1) {
            get_string(mod_refid, temp, 128);
            sprintf(use_string, temp, get_object_long_name(usetrip, nullptr, 0));
            return (use_string);
        }
    } break;
    }
    // If we haven't set ref or ref is garbage, use the long name.
    char *temp = static_cast<char *>((ref == -1) ? nullptr : RefGet(ref));
    if (temp == nullptr) {
        strcat(use_string, get_object_long_name(usetrip, nullptr, 0));
    } else {
        strncpy(use_string, temp, sz);
        use_string[sz - 1] = '\0';
    }
    return (use_string);
}

void look_at_object(ObjID id) {
    char buf[50];
    get_object_lookname(id, buf, sizeof(buf));
    message_info(buf);
}

// ------------------------------------------------------------------------
// view3d_dclick dispatches double clicks based on cursor mode
// Not a directly-installed mouse handler, called from view3d_mouse_handler
void view3d_dclick(LGPoint pos, frc *fr, bool shifted) {
    short obj_trans, obj;
    frc *use_frc;

    if (hack_takeover)
        return;
    switch (input_cursor_mode) {
    case INPUT_NORMAL_CURSOR:
    case INPUT_OBJECT_CURSOR:
        use_frc = svga_render_context;
        obj = fr_get_at_raw(use_frc, pos.x, pos.y, false, false);
        if ((obj > 0) && (objs[obj].obclass == CLASS_DOOR)) {
            obj_trans = fr_get_at_raw(use_frc, pos.x, pos.y, false, true);
            if (obj != obj_trans) {
                if (DOOR_REALLY_CLOSED(obj)) {
                    string_message_info(REF_STR_PickupTooFar);
                    return;
                } else
                    obj = obj_trans;
            }
        } else if (obj > 0) {
            obj = fr_get_at_raw(use_frc, pos.x, pos.y, false, true);
        }
        if ((short)obj < 0) {
            // Don't display texture look strings in cspace....eventually we should do some cool hack
            // for looking through walls, some sort of cspace fr_get_at or something
            if (global_fullmap->cyber)
                string_message_info(REF_STR_CybWallUse);
            else
                message_info(get_texture_use_string(loved_textures[~obj], nullptr, 0));
        } else if ((short)obj > 0) {
            use_cursor_pos = pos;
            use_object_in_3d(obj, shifted);
        } else {
            if (!global_fullmap->cyber) {
                if (!(_fr_glob_flags & FR_SOLIDFR_STATIC))
                    string_message_info(REF_STR_InkyUse);
            }
        }
    }
}

// -------------------------------------------------------------------------------
// view3d_mouse_handler is the actual installed mouse handler, dispatching to the above functions
uchar view3d_mouse_handler(uiEvent *ev, LGRegion *r, intptr_t data) {
    static bool got_focus = false;
    uiMouseData *md = &ev->mouse_data;
    view3d_data *view3d = (view3d_data *)data;
    bool retval = true;
    LGPoint pt;
    LGPoint evp = ev->pos;

    pt = evp;

    // If double sizing, convert the y to 640x480, then half it. The x stays as is.
    if (ShockPlus::Options::halfResolution)
        evp.y = SCONV_Y(evp.y) >> 1;
    else
        ss_point_convert(&(evp.x), &(evp.y), false);

    view3d_got_event = true;
    pt.x += r->r->ul.x - r->abs_x;
    pt.y += r->r->ul.y - r->abs_y;

    if (!RECT_TEST_PT(r->r, pt)) {
        view3d->ldown = false;
        physics_set_player_controls(MOUSE_CONTROL_BANK, 0, 0, CONTROL_NO_CHANGE, 0, CONTROL_NO_CHANGE,
                                    CONTROL_NO_CHANGE);
        return false;
    }
    if (md->action & MOUSE_LDOWN) {
        view3d->ldown = true;
        view3d->lastleft = evp;
        if (full_game_3d && !got_focus) {
            if (uiGrabFocus(r, UI_EVENT_MOUSE | UI_EVENT_MOUSE_MOVE) == OK)
                got_focus = true;
        }
        chg_set_flg(_current_3d_flag);
        // view3d_constrain_mouse(r,LBUTTON_CONSTRAIN_BIT);
    }
    if (md->action & MOUSE_LUP || !(md->buttons & (1 << MOUSE_LBUTTON))) {
        view3d->ldown = false;
        if (full_game_3d && got_focus) {
            if (uiReleaseFocus(r, UI_EVENT_MOUSE | UI_EVENT_MOUSE_MOVE) == OK)
                got_focus = false;
        }
        // view3d_unconstrain_mouse(LBUTTON_CONSTRAIN_BIT);
    }
    if (md->action & MOUSE_LUP && abs(evp.y - view3d->lastleft.y) < uiDoubleClickTolerance &&
        abs(evp.x - view3d->lastleft.x) < uiDoubleClickTolerance) {
        // make shift+leftclick act as double-leftclick with alternate effects shifted click; see sdl_events.c
        if (md->modifiers & 1) {
            view3d_dclick(evp, view3d->fr, true); // true indicates shifted
            view3d->lastleft = MakePoint(-100, -100);
        } else {
            ObjID id;
            frc *use_frc;
            short rabsx, rabsy;

            use_frc = svga_render_context;
            rabsx = r->abs_x;
            rabsy = r->abs_y;
            if (!ShockPlus::Options::halfResolution)
                ss_point_convert(&rabsx, &rabsy, false);

            id = fr_get_at(use_frc, evp.x - rabsx, evp.y - rabsy, true);
            if ((short)id > 0) {
                look_at_object(id);
            } else if ((short)id < 0) {
                int tnum = loved_textures[~id];
                if (global_fullmap->cyber)
                    string_message_info(REF_STR_CybWall);
                else
                    message_info(get_texture_name(tnum, nullptr, 0));
            } else {
                if (!global_fullmap->cyber) {
                    if (!(_fr_glob_flags & FR_SOLIDFR_STATIC))
                        string_message_info(REF_STR_InkyBlack);
                }
            }
            view3d->lastleft.x = -255;
        }
    }
    if ((md->action & (MOUSE_RDOWN | MOUSE_RUP)) || (md->buttons & (1 << MOUSE_RBUTTON)))
        view3d_rightbutton_handler(ev, r, view3d);

    /* KLC - done in another place now.
       else
       {
          view3d_unconstrain_mouse(RBUTTON_CONSTRAIN_BIT);
          if (fire_slam)
          {
                    if (full_game_3d)
                            uiPopSlabCursor(&fullscreen_slab);
             else
                            uiPopSlabCursor(&main_slab);
             fire_slam = false;
          }
       }

    */
    if ((md->buttons & (1 << MOUSE_RBUTTON)) == 0 ||
        ((md->buttons & (1 << MOUSE_RBUTTON)) == 0 && global_fullmap->cyber))
        physics_set_one_control(MOUSE_CONTROL_BANK, CONTROL_ZVEL, 0);

    if (md->action & UI_MOUSE_LDOUBLE) {
        // Spew(DSRC_USER_I_Motion,("use this, bay-bee!\n"));
        view3d_dclick(evp, view3d->fr, false);
        view3d->lastleft = MakePoint(-100, -100);
    }

    if (md->action & (MOUSE_WHEELUP | MOUSE_WHEELDN)) {
        cycle_weapons_func(0, 0, md->action & MOUSE_WHEELUP ? -1 : 1);
    }

    // data->ldown = true;

    // Do mouse motion.
    if (view3d_mouse_input(evp, r, view3d->ldown, &view3d->lastsect) != 0)
        view3d->lastleft = MakePoint(-1, -1); // if the player is moving, not a down

    return retval;
}

uchar view3d_key_handler(uiEvent *ev, LGRegion *r, intptr_t data) {
    uiCookedKeyData *kd = &ev->cooked_key_data;
    int i = 0, detect = 0, fire_pressed = 0;

    while (FireKeys[i] != 0) {
        if (kd->code == FireKeys[i])
            detect = 1;
        if (kd->code == (FireKeys[i] | KB_FLAG_DOWN)) {
            detect = 1;
            fire_pressed = 1;
            break;
        }
        i++;
    }
    if (!detect)
        return false;

    if (fire_pressed) {
        if (weapon_button_up) // if we haven't fired already
        {
            LGPoint evp = ev->pos;
            ss_point_convert(&(evp.x), &(evp.y), false);
            fire_player_weapon(&evp, r, !fire_slam);
            fire_slam = true;
            weapon_button_up = false;
        }
    } else {
        weapon_button_up = true;
        fire_slam = false;
    }

    return false;
}

// ---------
// EXTERNALS
// ---------

void install_motion_mouse_handler(LGRegion *r, frc *fr) {
    int cid;
    view3d_data *data = (view3d_data *)malloc(sizeof(view3d_data));
    data->ldown = false;
    data->rdown = false;
    data->lastsect = 0;
    data->lastleft.x = 0;
    data->lastleft.y = 0;
    data->lastright.x = 0;
    data->lastright.y = 0;
    data->fr = fr;
    uiInstallRegionHandler(r, UI_EVENT_MOUSE | UI_EVENT_MOUSE_MOVE | UI_EVENT_USER_DEFINED, view3d_mouse_handler,
                           (intptr_t)data, &cid);

    // Yeah, yeah, I know, it's not a mouse handler...
    uiInstallRegionHandler(r, UI_EVENT_KBD_COOKED, view3d_key_handler, 0, &cid);
    uiSetRegionDefaultCursor(r, nullptr);
}

void install_motion_keyboard_handler(LGRegion *r) {
    int cid;
    uiInstallRegionHandler(r, UI_EVENT_KBD_POLL, motion_keycheck_handler, 0, &cid);
}

void pop_cursor_object() {
    if (input_cursor_mode != INPUT_OBJECT_CURSOR)
        return;
    object_on_cursor = OBJ_NULL;
    uiPopSlabCursor(&fullscreen_slab);
    uiPopSlabCursor(&main_slab);
    input_cursor_mode = INPUT_NORMAL_CURSOR;
}

void push_cursor_object(short obj) {
    LGPoint hotspot;
    grs_bitmap *bmp;
#ifdef CURSOR_BACKUPS
    extern LGCursor backup_object_cursor;
#endif
    if (objs[obj].obclass == CLASS_GRENADE && objGrenades[objs[obj].specID].flags & GREN_ACTIVE_FLAG) {
        push_live_grenade_cursor(obj);
        return;
    }
    uiHideMouse(nullptr);
    if ((ID2TRIP(obj) == HEAD_TRIPLE) || (ID2TRIP(obj) == HEAD2_TRIPLE))
        bmp = bitmaps_3d[BMAP_NUM_3D(ObjProps[OPNUM(obj)].bitmap_3d) + objs[obj].info.current_frame];
    else
        bmp = bitmaps_2d[OPNUM(obj)];

    if (bmp == nullptr)
        return;

    object_on_cursor = obj;
    input_cursor_mode = INPUT_OBJECT_CURSOR;
#ifdef SVGA_SUPPORT
    if (convert_use_mode != 0) {
        grs_canvas temp_canv;
        // Get a new bigger bitmap
        gr_init_bitmap(&svga_cursor_bmp, svga_cursor_bits, BMT_FLAT8, BMF_TRANS,
                       lg_min(MODE_SCONV_X(bmp->w, 2), SVGA_CURSOR_WIDTH),
                       lg_min(MODE_SCONV_Y(bmp->h, 2), SVGA_CURSOR_HEIGHT));
        gr_make_canvas(&svga_cursor_bmp, &temp_canv);

        // Draw into it
        gr_push_canvas(&temp_canv);
        gr_clear(0);
        gr_scale_bitmap(bmp, 0, 0, svga_cursor_bmp.w, svga_cursor_bmp.h);
        gr_pop_canvas();

        // use it
        bmp = &svga_cursor_bmp;
    }
#endif
    hotspot.x = bmp->w / 2;
    hotspot.y = bmp->h / 2;
    uiMakeBitmapCursor(&object_cursor, bmp, hotspot);
#ifdef CURSOR_BACKUPS
    uiMakeBitmapCursor(&backup_object_cursor, bmp, hotspot);
#endif
    uiPushSlabCursor(&fullscreen_slab, &object_cursor);
    uiPushSlabCursor(&main_slab, &object_cursor);
    uiShowMouse(nullptr);
    look_at_object(obj);
}
