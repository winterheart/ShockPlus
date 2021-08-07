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
 * $Source: r:/prj/cit/src/RCS/hkeyfunc.c $
 * $Revision: 1.173 $
 * $Author: dc $
 * $Date: 1994/11/18 00:24:50 $
 */

#include <cstring>

#include "Engine/Options.h"
#include "Shock.h"

#include "ai.h"
#include "fullscrn.h"
#include "game_screen.h"
#include "grenades.h"
#include "hkeyfunc.h"
#include "invent.h"
#include "loops.h"
#include "mfdext.h"
#include "MacTune.h"
#include "musicai.h"
#include "newmfd.h"
#include "objwpn.h"
#include "saveload.h"
#include "softdef.h"
#include "tools.h"
#include "wares.h"
#include "mouselook.h"
#include "audiolog.h"
#include "Xmi.h"

//--------------
//  PROTOTYPES
//--------------
int select_object_by_class(int obclass, int num, ubyte *quantlist);

int current_palette_mode = TERRAIN_MODE;

uchar really_quit_key_func(ushort keycode, uint32_t context, intptr_t data) {
    gPlayingGame = false;
    return TRUE;
}

uchar toggle_bool_func(ushort keycode, uint32_t context, intptr_t data) {
    bool *tgl = (bool *)data;
    *tgl = !*tgl;
    return TRUE;
}

uchar change_mode_func(ushort keycode, uint32_t context, intptr_t data) {
    int newm = (int)data;

    if ((newm == AUTOMAP_LOOP) && ((!player_struct.hardwarez[HARDWARE_AUTOMAP]) || (global_fullmap->cyber)))
        return TRUE;
    _new_mode = newm;
    chg_set_flg(GL_CHG_LOOP);
    return TRUE;
}

#ifdef NOT_YET //

#ifdef HANDART_ADJUST

short hdx = 0, hdy = 0;
ubyte hcount = 0;

uchar move_handart(ushort keycode, uint32_t context, intptr_t data) {
    short amt = 1;
    ubyte foo = (ubyte)data;
    short *dir;

    if (foo & 0x10) {
        hdx = hdy = 0;
        return TRUE;
    }

    if (foo & 0x08)
        amt = 10;
    foo &= 0x7F;
    dir = (foo & 0x02) ? &hdx : &hdy;

    if (foo & 0x01)
        (*dir) += amt;
    else
        (*dir) -= amt;

    return TRUE;
}

uchar adv_handart(ushort keycode, uint32_t context, intptr_t data) {
    hcount = (hcount + 1) % 5;
    return TRUE;
}

#endif // HANDART_ADJUST

uchar toggle_view_func(ushort keycode, uint32_t context, intptr_t data) {
    extern uchar full_game_3d;
    return (change_mode_func(keycode, context, (full_game_3d) ? GAME_LOOP : FULLSCREEN_LOOP));
}

#endif // NOT_YET

void start_music(void) {
    //   if (music_card)
    //   {
    if (MacTuneInit() == 0) {
        music_on = TRUE;
        mlimbs_on = TRUE;
        mlimbs_AI_init();
        load_score_for_location(PLAYER_BIN_X, PLAYER_BIN_Y);
        MacTuneStartCurrentTheme();
    } else {
        ShockPlus::Options::enableSound = false;
        ShockPlus::Options::save();
    }
    //   }
}

void stop_music(void) {
    MacTuneShutdown();
    music_on = FALSE;
    mlimbs_on = FALSE;
    mlimbs_peril = DEFAULT_PERIL_MIN;
    mlimbs_monster = NO_MONSTER;
}

uchar toggle_music_func(ushort keycode, uint32_t context, intptr_t data) {
    if (music_on) {
        message_info("Music off.");
        StopTheMusic(); //do this here, not in stop_music(), to prevent silence when changing levels
        stop_music();
    } else {
        start_music();
        message_info("Music on.");
    }

    ShockPlus::Options::enableSound = music_on;
    ShockPlus::Options::save();

    return (FALSE);
}

uchar arm_grenade_hotkey(ushort keycode, uint32_t context, intptr_t data) {
    int i, row, act;

    if (!show_all_actives) {
        show_all_actives = TRUE;
        inv_last_page = -1;
        chg_set_flg(INVENTORY_UPDATE);
        mfd_force_update();
        return TRUE;
    }
    if (activate_grenade_on_cursor())
        return TRUE;
    act = player_struct.actives[ACTIVE_GRENADE];
    for (i = row = 0; i < act; i++)
        if (player_struct.grenades[i])
            row++;
    super_drop_func(ACTIVE_GRENADE, row);
    return TRUE;
}

int select_object_by_class(int obclass, int num, ubyte *quantlist) {
    int act = player_struct.actives[obclass];
    int newobj = act;

    inv_last_page = -1;
    chg_set_flg(INVENTORY_UPDATE);
    if (!show_all_actives) {
        show_all_actives = TRUE;
        return -1;
    }
    do {
        newobj = (newobj + 1) % num;
    } while (quantlist[newobj] == 0 && newobj != act);

    player_struct.actives[obclass] = newobj;
    return newobj;
}

uchar select_grenade_hotkey(ushort keycode, uint32_t context, intptr_t data) {
    int newobj;

    newobj = select_object_by_class(ACTIVE_GRENADE, NUM_GRENADES, player_struct.grenades);
    set_inventory_mfd(MFD_INV_GRENADE, newobj, TRUE);
    return TRUE;
}

uchar select_drug_hotkey(ushort keycode, uint32_t context, intptr_t data) {
    int newobj;

    newobj = select_object_by_class(ACTIVE_DRUG, NUM_DRUGS, player_struct.drugs);
    set_inventory_mfd(MFD_INV_DRUG, newobj, TRUE);
    return TRUE;
}

uchar use_drug_hotkey(ushort keycode, uint32_t context, intptr_t data) {
    int i, row, act;

    if (!show_all_actives) {
        show_all_actives = TRUE;
        inv_last_page = -1; // to force redraw
        chg_set_flg(INVENTORY_UPDATE);
        return TRUE;
    }
    act = player_struct.actives[ACTIVE_DRUG];
    for (i = row = 0; i < act; i++)
        if (player_struct.drugs[i])
            row++;
    super_use_func(ACTIVE_DRUG, row);
    return TRUE;
}

uchar clear_fullscreen_func(ushort keycode, uint32_t context, intptr_t data) {
    full_lower_region(&mfd[MFD_RIGHT].reg2);
    full_lower_region(&mfd[MFD_LEFT].reg2);
    full_lower_region(inventory_region_full);
    full_visible = 0;
    strcpy(last_message, "");
    chg_unset_sta(FULLSCREEN_UPDATE);
    return (FALSE);
}

uchar toggle_physics_func(ushort keycode, uint32_t context, intptr_t data) {
    physics_running = !physics_running;

    pacifism_on = !physics_running;

    if (physics_running)
        message_info("Physics turned on");
    else
        message_info("Physics turned off");

    return (FALSE);
}

uchar toggle_giveall_func(ushort keycode, uint32_t context, intptr_t data) {
    message_info("Kick some ass!");

    for (int i = 0; i < NUM_HARDWAREZ; i++)
        player_struct.hardwarez[i] = 1;
    player_struct.hardwarez[HARDWARE_360] = 3;

    //rail gun
    player_struct.weapons[0].type = GUN_SUBCLASS_SPECIAL;
    player_struct.weapons[0].subtype = 1;
    player_struct.weapons[0].ammo = 50;
    player_struct.weapons[0].ammo_type = 0;
    player_struct.weapons[0].make_info = 0;

    //ion beam
    player_struct.weapons[1].type = GUN_SUBCLASS_BEAM;
    player_struct.weapons[1].subtype = 2;
    player_struct.weapons[1].heat = 0;
    player_struct.weapons[1].setting = 40;
    player_struct.weapons[1].make_info = 0;

    //riot gun, hollow
    player_struct.weapons[2].type = GUN_SUBCLASS_PISTOL;
    player_struct.weapons[2].subtype = 4;
    player_struct.weapons[2].ammo = 100;
    player_struct.weapons[2].ammo_type = 0;
    player_struct.weapons[2].make_info = 0;

    //skorpion, slag
    player_struct.weapons[3].type = GUN_SUBCLASS_AUTO;
    player_struct.weapons[3].subtype = 1;
    player_struct.weapons[3].ammo = 150;
    player_struct.weapons[3].ammo_type = 0;
    player_struct.weapons[3].make_info = 0;

    //magpulse
    player_struct.weapons[4].type = GUN_SUBCLASS_SPECIAL;
    player_struct.weapons[4].subtype = 0;
    player_struct.weapons[4].ammo = 50;
    player_struct.weapons[4].ammo_type = 0;
    player_struct.weapons[4].make_info = 0;

    //sparq
    player_struct.weapons[5].type = GUN_SUBCLASS_BEAM;
    player_struct.weapons[5].subtype = 0;
    player_struct.weapons[5].heat = 0;
    player_struct.weapons[5].setting = 40;
    player_struct.weapons[5].make_info = 0;

    //laser rapier
    player_struct.weapons[6].type = GUN_SUBCLASS_HANDTOHAND;
    player_struct.weapons[6].subtype = 1;
    player_struct.weapons[6].heat = 0;
    player_struct.weapons[6].setting = 0;
    player_struct.weapons[6].make_info = 0;

    player_struct.hit_points = 255;
    player_struct.energy = 255;

    // Software stuff
    player_struct.softs.misc[SOFTWARE_TURBO] = 5;
    player_struct.softs.misc[SOFTWARE_FAKEID] = 5;
    player_struct.softs.misc[SOFTWARE_DECOY] = 5;
    player_struct.softs.misc[SOFTWARE_RECALL] = 5;

    // So we put games in your game so you can play game while you playing game!
    player_struct.softs.misc[SOFTWARE_GAMES] = 255;

    chg_set_flg(INVENTORY_UPDATE);
    chg_set_flg(VITALS_UPDATE);
    mfd_force_update();

    return (FALSE);
}

uchar toggle_up_level_func(ushort keycode, uint32_t context, intptr_t data) {
    message_info("Changing level!");
    go_to_different_level((player_struct.level + 1 + 15) % 15);

    return (TRUE);
}

uchar toggle_down_level_func(ushort keycode, uint32_t context, intptr_t data) {
    message_info("Changing level!");
    go_to_different_level((player_struct.level - 1 + 15) % 15);

    return (TRUE);
}

#ifdef NOT_YET //

#ifdef PLAYTEST

#define camera_info message_info
uchar reset_camera_func(ushort keycode, uint32_t context, intptr_t data) {
    extern uchar cam_mode;
    extern cams objmode_cam, *motion_cam, player_cam;

    if ((uchar *)data) {
        if (cam_mode != OBJ_STATIC_CAMERA) {
            camera_info("cant toggle");
            return FALSE;
        }
        if (motion_cam != NULL) {
            motion_cam = NULL;
            camera_info("back to cam control");
        } else {
            motion_cam = fr_camera_getdef();
            camera_info("back to obj control");
        }
    } else {
        camera_info("camera reset");
        cam_mode = OBJ_PLAYER_CAMERA;
        fr_camera_setdef(&player_cam);
    }
    chg_set_flg(_current_3d_flag);
    return (FALSE);
}

uchar current_camera_func(ushort keycode, uint32_t context, intptr_t data) {
    extern cams objmode_cam, *motion_cam;
    extern uchar cam_mode;
    fix cam_locs[6], *cam_ptr_hack;

    motion_cam = NULL;
    // Not sure what to pass for last two params....
    switch ((uchar)data) {
    case OBJ_STATIC_CAMERA:
        if (cam_mode == OBJ_DYNAMIC_CAMERA) {
            camera_info("cant go static");
            return FALSE;
        }
        motion_cam = fr_camera_getdef(); // note super sneaky fall through hack
        camera_info("camera static");
    case OBJ_DYNAMIC_CAMERA:
        fr_camera_modtype(&objmode_cam, CAMTYPE_ABS, CAMBIT_OBJ);
        cam_ptr_hack = fr_camera_getpos(NULL);
        memcpy(cam_locs, cam_ptr_hack, 6 * sizeof(fix));
        fr_camera_update(&objmode_cam, cam_locs, CAM_UPDATE_NONE, NULL);
        if (motion_cam == NULL)
            camera_info("camera dynamic");
        break;
    case OBJ_CURRENT_CAMERA:
        camera_info("current obj");
        fr_camera_modtype(&objmode_cam, CAMTYPE_OBJ, CAMBIT_OBJ);
        fr_camera_update(&objmode_cam, (void *)current_object, CAM_UPDATE_NONE, NULL);
        break;
    }
    cam_mode = (uchar)data;
    fr_camera_setdef(&objmode_cam);
    chg_set_flg(_current_3d_flag);
    return (FALSE);
}

uchar mono_log_on = FALSE;

uchar log_mono_func(ushort keycode, uint32_t context, intptr_t data) {
    if (mono_log_on) {
        mono_logoff();
        message_info("Mono logging off.");
        mono_log_on = FALSE;
    } else {
        mono_logon("monolog.txt", MONO_LOG_NEW, MONO_LOG_ALLWIN);
        message_info("Mono logging on.");
        mono_log_on = TRUE;
    }
    return (FALSE);
}

uchar clear_transient_lighting_func(ushort keycode, uint32_t context, intptr_t data) {
    int x, y;
    MapElem *pme;
    for (x = 0; x < MAP_XSIZE; x++) {
        for (y = 0; y < MAP_YSIZE; y++) {
            pme = MAP_GET_XY(x, y);
            me_templight_flr_set(pme, 0);
            me_templight_ceil_set(pme, 0);
        }
    }
    message_info("Trans. light cleared");
    return (FALSE);
}

uchar level_entry_trigger_func(ushort keycode, uint32_t context, intptr_t data) {
    extern errtype do_level_entry_triggers();
    do_level_entry_triggers();
    message_info("Level entry triggered.");
    return (FALSE);
}

uchar convert_one_level_func(ushort keycode, uint32_t context, intptr_t data) {
    extern errtype obj_level_munge();
#ifdef TEXTURE_CRUNCH_HACK
    extern errtype texture_crunch_init();

    texture_crunch_init();
#endif
    obj_level_munge();
    return (TRUE);
}

    //#define CONVERT_FROM_OLD_RESID
    //#define TEXTURE_CRUNCH_HACK

#define NUM_CONVERT_LEVELS 16

uchar convert_all_levels_func(ushort keycode, uint32_t context, intptr_t data) {
    int i;
    char atoi_buf[10], fn[10], curr_fname[40], new_fname[40];
    errtype retval;

    extern Datapath savegame_dpath;
    extern void edit_load_func(char *fn, uchar source, short level_num);
    extern void edit_save_func(char *fn, uchar source, short level_num);
    extern errtype obj_level_munge();
#ifdef TEXTURE_CRUNCH_HACK
    extern errtype texture_crunch_init();

    texture_crunch_init();
#endif

    // save off old level
    edit_save_func("templevl.dat", 0, 0);

    // loop through the real levels
    for (i = 0; i < NUM_CONVERT_LEVELS; i++) {
        retval = OK;
        // load level i
        strcpy(fn, "level");
        strcat(fn, itoa(i, atoi_buf, 10));
        strcat(fn, ".dat");
        Spew(DSRC_EDITOR_Modify, ("fn = %s\n", fn));
        if (DatapathFind(&savegame_dpath, fn, curr_fname)) {
#ifdef CONVERT_FROM_OLD_RESID
            retval = load_current_map(curr_fname, OLD_LEVEL_ID_NUM, &savegame_dpath);
#else
            retval = load_current_map(curr_fname, LEVEL_ID_NUM, &savegame_dpath);
#endif
            Spew(DSRC_EDITOR_Modify, ("convert_all trying to load %s\n", curr_fname));
        } else
            retval = ERR_FOPEN;

        Spew(DSRC_EDITOR_Modify, ("curr_fname = %s\n", curr_fname));
        if (retval != OK) {
            strcpy(new_fname, "R:\\prj\\cit\\src\\data\\");
            strcat(new_fname, fn);
            retval = load_current_map(new_fname, LEVEL_ID_NUM, NULL);
            Spew(DSRC_EDITOR_Modify, ("new_fname = %s\n", new_fname));
        }

        // Generate the report
        obj_level_munge();
        Spew(DSRC_EDITOR_Modify, ("convert_all trying to save %s\n", fn));
        save_current_map(fn, LEVEL_ID_NUM, TRUE, TRUE);
    }

    // reload original level
    edit_load_func("templevl.dat", 0, 0);

    return (FALSE);
}

#endif

uchar invulnerable_func(ushort keycode, uint32_t context, intptr_t data) {
    if (config_get_raw(CFG_HKEY_GO, NULL, 0)) {
        player_invulnerable = !player_invulnerable;
        if (player_invulnerable)
            message_info("invulnerability on");
        else
            message_info("invulnerability off");
    } else {
        message_info("Winners don't use hotkeys");
        damage_player(50, EXPLOSION_FLAG, 0);
    }
    return (FALSE);
}

uchar pacifist_func(ushort keycode, uint32_t context, intptr_t data) {
    extern uchar pacifism_on;
    pacifism_on = !pacifism_on;
    if (pacifism_on)
        message_info("pacifism on");
    else
        message_info("pacifism off");
    return (FALSE);
}

int pause_id;
uchar remove_pause_handler = FALSE;

uchar pause_callback(uiEvent *, LGRegion *, void *) { return (TRUE); }

uchar unpause_callback(uiEvent *, LGRegion *, void *) { return (TRUE); }

#endif // NOT_YET

uchar pause_game_func(ushort keycode, uint32_t context, intptr_t data) {
    game_paused = !game_paused;
    CaptureMouse(!game_paused);

    if (game_paused)
        uiPushGlobalCursor(&globcursor);
    else
        uiPopGlobalCursor();

    if (game_paused) {
        redraw_paused = TRUE;
		snd_kill_all_samples();
        audiolog_stop();
        return FALSE;
    }

    mouse_look_unpause();

    return TRUE;
    /* KLC - not needed for Mac version
            game_paused = !game_paused;
            if (game_paused)
            {
                    uiPushGlobalCursor(&globcursor);
                    uiInstallRegionHandler(inventory_region, UI_EVENT_MOUSE_MOVE, pause_callback, NULL, &pause_id);
                    uiGrabFocus(inventory_region, UI_EVENT_MOUSE_MOVE);
                    stop_digi_fx();
                    redraw_paused=TRUE;
            }
            else
            {
                    uiRemoveRegionHandler(inventory_region, pause_id);
                    uiReleaseFocus(inventory_region, UI_EVENT_MOUSE_MOVE);
                    uiPopGlobalCursor();
            }
    */
}

/*KLC - not needed for Mac version
uchar unpause_game_func(ushort, uint32_t, intptr_t)
{
        extern uchar game_paused;
        extern LGRegion *inventory_region;

        if (game_paused)
        {
                game_paused = !game_paused;
                uiRemoveRegionHandler(inventory_region, pause_id);
                uiReleaseFocus(inventory_region, UI_EVENT_MOUSE_MOVE|UI_EVENT_JOY);
                uiPopGlobalCursor();
        }
        return(FALSE);
}
*/

uchar toggle_mouse_look(ushort keycode, uint32_t context, intptr_t data) {
    mouse_look_toggle();
    return (TRUE);
}

//--------------------------------------------------------------------
//  For Mac version.  Save the current game.
//--------------------------------------------------------------------
/*
uchar save_hotkey_func(ushort keycode, uint32_t context, intptr_t data) {
    if (global_fullmap->cyber) // Can't save in cyberspace.
    {
        message_info("Can't save game in cyberspace.");
        return TRUE;
    }

    if (music_on) // Setup the environment for doing Mac stuff.
        MacTuneKillCurrentTheme();
    uiHideMouse(NULL);
    SS_ShowCursor();

    // CopyBits(&gMainWindow->portBits, &gMainOffScreen.bits->portBits, &gActiveArea, &gOffActiveArea, srcCopy, 0L);

    if (gIsNewGame) // Do the save thang.
    {
        status_bio_end();

        // Fixme: Save game here!

        status_bio_start();
    }

    uiShowMouse(NULL);
    if (music_on)
        MacTuneStartCurrentTheme();

    return TRUE;
}
*/

#ifdef NOT_YET //

//#define CHECK_STATE_N_HOTKEY
#ifdef PLAYTEST
uchar check_state_func(ushort keycode, uint32_t context, intptr_t data) {
    int avail_memory(int debug_src);
    avail_memory(DSRC_TESTING_Test3);
#ifdef CHECK_STATE_N_HOTKEY
    extern void check_state_every_n_seconds();
    check_state_every_n_seconds();
#endif
#ifdef CORVIN_ZILM_HKEY
    extern uchar CorvinZilm;
    extern int watchcount;
    MemStat pms;
    watchcount = 0;
    CorvinZilm = TRUE;
    MemStats(&pms);
#endif
    return (TRUE);
}

uchar diffdump_game_func(ushort keycode, uint32_t context, intptr_t data) {
    char goof[45];
    sprintf(goof, "diff=%d,%d,%d,%d\n", player_struct.difficulty[0], player_struct.difficulty[1],
            player_struct.difficulty[2], player_struct.difficulty[3]);
    message_info(goof);
    return (TRUE);
}

uchar toggle_difficulty_func(ushort keycode, uint32_t context, intptr_t data) {
    ubyte which = (ubyte)data - 1;

    player_struct.difficulty[which]++;
    player_struct.difficulty[which] %= 4;
    return (TRUE);
}

uchar toggle_ai_func(ushort keycode, uint32_t context, intptr_t data) {
    extern uchar ai_on;
    ai_on = !ai_on;
    if (ai_on)
        message_info("AI state on\n");
    else
        message_info("AI state off\n");
    return (TRUE);
}

uchar toggle_safety_net_func(ushort keycode, uint32_t context, intptr_t data) {
    extern uchar safety_net_on;
    safety_net_on = !safety_net_on;
    if (safety_net_on)
        message_info("Safety Net on\n");
    else
        message_info("Safety Net off\n");
    return (TRUE);
}
#endif

#ifdef NEW_RES_LIB_INSTALLED
uchar res_cache_usage_func(ushort keycode, uint32_t context, intptr_t data) {
    extern long ResViewCache(uchar only_locks);
    ResViewCache((bool)data);
    return (TRUE);
}
#endif

#endif // NOT_YET
