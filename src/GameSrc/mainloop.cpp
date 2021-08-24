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
 * $Source: r:/prj/cit/src/RCS/mainloop.c $
 * $Revision: 1.42 $
 * $Author: xemu $
 * $Date: 1994/11/09 02:09:05 $
 */

/*
 * Citadel main loops
 *
 * The idea here is that we have a separate loop for each game mode/setup
 * There is a 4/12 bit change flag which is 4 global and 12 local
 * The global loop checks OS and input
 * Then calls the local loop, which does it's internal inlined functions
 *  and then processes it's change flags
 * The global loop then gets control and does it's own change flags
 * If you want to switch modes/loops you call change_loop which both sets
 *  global change bit 3 as well setting some variables.  When the main loop
 *  reaches the bottom it triggers on change bit 3 and calls the switch code
 */

#include "InitMac.h"
#include "SDLFunctions.h"
#include "Shock.h"
#include "amaploop.h"
#include "cutsloop.h"
#include "game_screen.h"
#include "fullscrn.h"
#include "loops.h"
#include "fullamap.h"
#include "input.h"
#include "setup.h"
#include "status.h"
#include "tickcount.h"
#include "tools.h"
#include "wrapper.h"

extern "C" {
#include "sdl_events.h"
};

// how is the game doing, anyway, set to true at end of time
bool cit_success = false;

// are we "paused"
bool game_paused = false;

frc *_current_fr_context;
int16_t current_loop = SETUP_LOOP; /* which loop we currently are */
int16_t current_3d_flag = DEMOVIEW_UPDATE;
LGRegion *_current_view = nullptr;
uint32_t _change_flag = 0;   /* change flags for loop */
uint32_t _static_change = 0; /* current static changes */
int16_t _new_mode = 0;     /* mode to change to, if any */
int16_t _last_mode = 0;    /* last mode, if you want to change back to it */
bool time_passes = true;
bool saves_allowed = false;
bool physics_running = true;
bool ai_on = true;
bool anim_on = true;
bool player_invulnerable = false;
bool player_immortal = false;
bool always_render = false;
bool pal_fx_on = true;

// Note that in the shipping version, the edit_loop stuff should never
// get called, but needs to be SOMETHING as a place holder
// void
// (*citadel_loops[])(void)={game_loop,game_loop,game_loop,game_loop,setup_loop,game_loop,cutscene_loop,game_loop,automap_loop};
// void
// (*enter_modes[])(void)={screen_start,fullscreen_start,screen_start,screen_start,setup_start,screen_start,cutscene_start,fullscreen_start,amap_start};
// void (*exit_modes[])(void)={screen_exit,fullscreen_exit,screen_exit,
// screen_exit,setup_exit,screen_exit,cutscene_exit,fullscreen_exit,amap_exit};

void (*citadel_loops[])(void) = {game_loop, game_loop, game_loop, game_loop, setup_loop, game_loop, cutscene_loop, game_loop, automap_loop};
void (*enter_modes[])(void) = {screen_start, fullscreen_start, nullptr, nullptr, setup_start, nullptr, cutscene_start, fullscreen_start, amap_start};
void (*exit_modes[])(void) = {screen_exit, fullscreen_exit, nullptr, nullptr, setup_exit, nullptr, cutscene_exit, fullscreen_exit, amap_exit};

void loopmode_switch(short *cmode) {
    // Actually switch mode
    _last_mode = *cmode;
    (*exit_modes[_last_mode])();
    *cmode = _new_mode;
    _static_change = 0;
    if (*cmode >= 0)
        (*enter_modes[*cmode])();

#ifdef SVGA_SUPPORT
    if (wrapper_screenmode_hack) {
        wrapper_start(screenmode_screen_init);
    }
#endif
}

void loopmode_exit(short loopmode) {
    if (exit_modes[loopmode])
        (*exit_modes[loopmode])();
}

void loopmode_enter(short loopmode) { (*enter_modes[loopmode])(); }

void mainloop(int argc, char *argv[]) {
    while (current_loop >= 0 && gPlayingGame) {
        gShockTicks = TickCount();

        if (!(_change_flag & (ML_CHG_BASE << 1)))
            input_chk(); // go get the UI stuff going

        // DG: at the beginning of each frame, get all the events from SDL
        pump_events();

        // Run the loop
        (*citadel_loops[current_loop])();

        // really, only loopmode_switch (the <<3 case) will be in the game
        if (globalChanges) {
            if (_change_flag & (ML_CHG_BASE << 3)) {
                loopmode_switch(&current_loop);
            }
            chg_unset_flg(ML_CHG_BASE << 3);
        }
#ifdef ALWAYS_SHOW_FR
        fr_show_rate(-1);
#endif
        // OR in the static change flags...
        chg_set_flg(_static_change);

        status_bio_update();
        ZoomDrawProc(false); //draw zoom rectangle if enabled; if not, returns immediately

        SDLDraw();

        ZoomDrawProc(true); //erase zoom rectangle if enabled; if not, returns immediately
    }

    cit_success = true;
    // hit them atexit's
}

errtype static_change_copy() {
    if (always_render)
        chg_set_sta(current_3d_flag);
    else
        chg_unset_sta(current_3d_flag);
    return (OK);
}
