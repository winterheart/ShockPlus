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
 * $Source: r:/prj/cit/src/RCS/plotware.c $
 * $Revision: 1.19 $
 * $Author: xemu $
 * $Date: 1994/11/09 16:27:33 $
 *
 *
 */

#include <stdbool.h>
#include <string.h>

#include "mfdint.h"
#include "mfdext.h"
#include "mfdfunc.h"
#include "mfddims.h"
#include "objsim.h"
#include "gamestrn.h"
#include "tools.h"
#include "mfdgadg.h"
#include "wares.h"
#include "gr2ss.h"

#include "mfdart.h"
#include "gamescr.h"
#include "otrip.h"
#include "cybstrng.h"
#include "plotware.h"
#include "shodan.h"
#include "str.h"
#include "trigger.h"

// ============================================================
//                      DA PLOTWARE
// ============================================================

// -------
// DEFINES
// -------

#define MFD_PLOTWARE_FUNC 16
#define ITEM_COLOR 0x5A
#define TITLE_COLOR 0x35

#define NULL_PAGE 0xFF
#define PLOTWARE_VERSION (player_struct.hardwarez[CPTRIP(STATUS_HARD_TRIPLE)])
#define NUM_PAGES 3

typedef struct _plot_display {
    ubyte page;     // Page number of this display.
    int name;       // string id of name.
    ubyte color;    // color to display in
    int baseval;    // base string id for val, zero means display as int
    short questvar; // Quest variable
} plot_display;

#define INT_TYPE 0
#define COUNTDOWN_TYPE 1
#define HACK_TYPE 2
#define PLAYER_FIELD(fld) (char *)&(((Player *)(0))->fld)

#define MAIN_PROGRAM_QDATA 0x1009

// ----------
//  PROTOTYPES
// ----------
void fill_time(short val, char *vbuf);
bool do_plotware_hack(int hack_num, char *vbuf);
void plotware_showpage(uchar page);
uchar plotware_button_handler(MFD *m, LGPoint bttn, uiEvent *ev, void *data);

// --------
//  GLOBALS
// --------
plot_display PlotDisplays[] = {
    {0, REF_STR_pwShodometer,   ITEM_COLOR,  HACK_TYPE,          2},
    {0, REF_STR_pwLaser,        ITEM_COLOR,  REF_STR_pwLaser0,   0x2008},
    {0, REF_STR_pwLifePods,     ITEM_COLOR,  REF_STR_pwEnabled,  0x2014},
    {0, REF_STR_pwShield,       ITEM_COLOR,  REF_STR_pwShield0,  0x2006},
    {0, REF_STR_pwCooling,      ITEM_COLOR,  REF_STR_pwCooling0, 0x2014},
    {0, REF_STR_pwDestructTime, ITEM_COLOR,  HACK_TYPE,          1},

    {1, REF_STR_pwNodes,        ITEM_COLOR,  HACK_TYPE,          6},
    // {1, REF_STR_pwMulti,        ITEM_COLOR,  INT_TYPE,           0x1000},
    // {1, REF_STR_pwCPUcool,      ITEM_COLOR,  REF_STR_pwCPUcool0, 0x2010},
    {1, REF_STR_pwMainProgram,  ITEM_COLOR,  REF_STR_pwNull,     0},
    {1, REF_STR_Null,           ITEM_COLOR,  REF_STR_pwMainProgram0, MAIN_PROGRAM_QDATA},
    // {1, REF_STR_pwDownLoadTime, ITEM_COLOR,  HACK_TYPE,          0 },
    // {1, REF_STR_pwBridgeTime,   ITEM_COLOR,  HACK_TYPE,          4 },
    // {1, REF_STR_pwVirusTime,    ITEM_COLOR,  HACK_TYPE,          5 },
    {1, REF_STR_pwComm,         ITEM_COLOR,  REF_STR_pwComm0,    0x1002},

    {2, REF_STR_pwGroveStatus,  TITLE_COLOR, REF_STR_pwNull,     0},
    {2, REF_STR_pwAlpha,        ITEM_COLOR,  HACK_TYPE,          7},
    {2, REF_STR_pwBeta,         ITEM_COLOR,  HACK_TYPE,          8},
    {2, REF_STR_pwGamma,        ITEM_COLOR,  REF_STR_pwGamma0,   0},
    {2, REF_STR_pwDelta,        ITEM_COLOR,  HACK_TYPE,          9},

    {NULL_PAGE}
};

void fill_time(short val, char *vbuf) {
    if (val != 0) {
        int16_t hours = val / 3600;
        int16_t minutes = (val - (3600 * hours)) / 60;
        int16_t seconds = (val - (3600 * hours) - minutes * 60);
        sprintf(vbuf, "%d:%02d:%02d", hours, minutes, seconds);
    } else {
        strcpy(vbuf, "-:--:--");
    }
}

#define DOWNLOAD_TIME 100
#define DOWNLOAD_PROGNUM 2
#define VIRUS_PROGNUM 1
#define BRIDGE_PROGNUM 4

#define REACTOR_QDATA 0x1002
#define REACTOR_DESTRUCT 1

#define SHODOMETER_BASE 0x1010
#define SHODOMETER_LEVELS MAX_SHODOMETER_LEVEL + 1

#define NODES_QDATA 0x1001
#define TOTAL_NODES 27

bool do_plotware_hack(int hack_num, char *vbuf) {
    switch (hack_num) {
    case 0:
        if (qdata_get(MAIN_PROGRAM_QDATA) != DOWNLOAD_PROGNUM)
            return false;
        sprintf(vbuf, "%d%%", player_struct.time2comp * 100 / DOWNLOAD_TIME);
        return true;
    case 1:
        if (qdata_get(REACTOR_QDATA) != REACTOR_DESTRUCT)
            return false;
        fill_time(player_struct.time2comp, vbuf);
        return true;
    case 2:
        if (player_struct.level >= SHODOMETER_LEVELS)
            return false;
        else {
            sprintf(vbuf, "%d%%",
                QUESTVAR_GET(0x10 + player_struct.level) * 100 / player_struct.initial_shodan_vals[player_struct.level]);
            return true;
        }
    // There is nooooooooooooooooooo case 3.
    // Mostly because we haven't necessarily loaded the other levels to figure
    // out what their shodometer levels were.
    case 4:
        if (qdata_get(MAIN_PROGRAM_QDATA) != BRIDGE_PROGNUM)
            return false;
        fill_time(player_struct.time2comp, vbuf);
        return true;
    case 5:
        if (qdata_get(MAIN_PROGRAM_QDATA) != VIRUS_PROGNUM)
            return false;
        fill_time(player_struct.time2comp, vbuf);
        return true;
    case 6:
        sprintf(vbuf, "%d", TOTAL_NODES - qdata_get(NODES_QDATA));
        return true;
    case 7:
        if (QUESTBIT_GET(0x0B))
            get_string(REF_STR_pwAlpha0 + 1, vbuf, 9);
        else
            get_string(REF_STR_pwAlpha0, vbuf, 9);
        return true;
    case 8:
        if (QUESTBIT_GET(0x0F))
            get_string(REF_STR_pwBeta0 + 2, vbuf, 9);
        else if (QUESTBIT_GET(0x0C))
            get_string(REF_STR_pwBeta0 + 1, vbuf, 9);
        else
            get_string(REF_STR_pwBeta0, vbuf, 9);
        return true;
    case 9:
        if (QUESTBIT_GET(0x0A))
            get_string(REF_STR_pwDelta0 + 1, vbuf, 9);
        else
            get_string(REF_STR_pwDelta0, vbuf, 9);
        return true;

    default:
        *vbuf = '\0';
        return false;
    }
}

// ---------------
// EXPOSE FUNCTION
// ---------------

#define PLOTWARE_MFD_FUNC 16

#define DISPLAY_TOP_MARGIN 10
#define LEFT_X 2
#define RIGHT_X (MFD_VIEW_WID - 2)
#define PLOTWARE_PAGENUM (player_struct.mfd_func_data[PLOTWARE_MFD_FUNC][0])

#define BUTTON_Y (MFD_VIEW_HGT - res_bm_size(REF_IMG_PrevPage).y - 2)

void mfd_plotware_expose(MFD *mfd, ubyte control) {
    uchar full = control & MFD_EXPOSE_FULL;
    if (control == 0) // MFD is drawing stuff
    {
        // Do unexpose stuff here.
    }
    if (control & MFD_EXPOSE) // Time to draw stuff
    {
        short y = DISPLAY_TOP_MARGIN;
        int i;
        // clear update rects
        mfd_clear_rects();
        // set up canvas
        gr_push_canvas(pmfd_canvas);
        ss_safe_set_cliprect(0, 0, MFD_VIEW_WID, MFD_VIEW_HGT);

        mfd_item_micro_expose(true, STATUS_HARD_TRIPLE);
        // mfd_item_micro_hires_expose(true,STATUS_HARD_TRIPLE);
        if (!full)
            mfd_clear_rects();

        // INSERT GRAPHICS CODE HERE
        gr_set_font(static_cast<grs_font *>(ResLock(MFD_FONT)));
        for (i = 0; PlotDisplays[i].page < NUM_PAGES; i++) {
            char buf[40], vbuf[40];
            short val;
            short w, h;
            plot_display *dp = &PlotDisplays[i];
            if (dp->page != PLOTWARE_PAGENUM)
                continue;
            switch (dp->baseval) {
            case INT_TYPE:
                val = qdata_get(dp->questvar);
                sprintf(vbuf, "%d", val);
                break;
            case COUNTDOWN_TYPE:
                val = *(short *)(((char *)&player_struct) + dp->questvar);
                fill_time(val, vbuf);
                break;
            case HACK_TYPE:
                if (!do_plotware_hack(dp->questvar, vbuf))
                    continue;
                break;
            default:
                val = qdata_get(dp->questvar);
                if (dp->questvar & 0x2000)
                    if (val)
                        val = 1;
                get_string(dp->baseval + val, vbuf, sizeof(vbuf));
                break;
            }
            get_string(dp->name, buf, sizeof(buf));
            mfd_full_draw_string(buf, LEFT_X, y, dp->color, MFD_FONT, true, true);
            gr_string_size(vbuf, &w, &h);
            mfd_full_draw_string(vbuf, RIGHT_X - w, y, dp->color, MFD_FONT, true, true);
            y += h + 1;
        }
        if (full) {
            char buf[50];
            short w, h;
            // Draw the page number
            get_string(REF_STR_pwPage0 + PLOTWARE_PAGENUM, buf, sizeof(buf));
            gr_string_size(buf, &w, &h);
            LGPoint size = res_bm_size(REF_IMG_NextPage);
            mfd_draw_string(buf, (MFD_VIEW_WID - w) / 2, BUTTON_Y + (size.y - h) / 2, ITEM_COLOR, true);
            // Draw the page buttons
            draw_raw_resource_bm(REF_IMG_PrevPage, LEFT_X, BUTTON_Y);
            draw_raw_resource_bm(REF_IMG_NextPage, RIGHT_X - size.x, BUTTON_Y);
        }

        ResUnlock(MFD_FONT);
        // on a full expose, make sure to draw everything

        if (full)
            mfd_add_rect(0, 0, MFD_VIEW_WID, MFD_VIEW_HGT);

        // Pop the canvas
        gr_pop_canvas();
        // Now that we've popped the canvas, we can send the
        // updated mfd to screen
        mfd_update_rects(mfd);
    }
}

void plotware_showpage(uchar page) {
    if (PLOTWARE_VERSION == 0 || page >= NUM_PAGES)
        return;
    PLOTWARE_PAGENUM = page;
    plotware_turnon(true, true);
}

// --------------
// BUTTON HANDLER
// --------------
uchar plotware_button_handler(MFD *n, LGPoint bttn, uiEvent *ev, void *data) {
    if (!(ev->subtype & MOUSE_LDOWN))
        return true;
    if (bttn.x == 0)
        PLOTWARE_PAGENUM = (PLOTWARE_PAGENUM == 0) ? NUM_PAGES - 1 : PLOTWARE_PAGENUM - 1;
    if (bttn.x == 1)
        PLOTWARE_PAGENUM = (PLOTWARE_PAGENUM >= NUM_PAGES - 1) ? 0 : PLOTWARE_PAGENUM + 1;
    mfd_notify_func(MFD_PLOTWARE_FUNC, MFD_ITEM_SLOT, false, MFD_ACTIVE, true);
    return true;
}

// --------------
// INITIALIZATION
// --------------
errtype mfd_plotware_init(MFD_Func *f) {
    int cnt = 0;
    LGPoint bsize;
    LGPoint bdims;
    LGRect r;
    errtype err;
    bsize.x = res_bm_size(REF_IMG_PrevPage).x;
    bsize.y = res_bm_size(REF_IMG_NextPage).y;
    bdims.x = 2;
    bdims.y = 1;
    r.ul.x = LEFT_X;
    r.ul.y = BUTTON_Y;
    r.lr.x = RIGHT_X;
    r.lr.y = r.ul.y + bsize.y;
    err = MFDBttnArrayInit(&f->handlers[cnt++], &r, bdims, bsize, plotware_button_handler, NULL);
    if (err != OK)
        return err;
    f->handler_count = cnt;
    return OK;
}

void plotware_turnon(uchar visible, uchar realstart) {
    if (visible) {
        set_inventory_mfd(MFD_INV_HARDWARE, CPTRIP(STATUS_HARD_TRIPLE), true);
        mfd_change_slot(mfd_grab_func(MFD_PLOTWARE_FUNC, MFD_ITEM_SLOT), MFD_ITEM_SLOT);
    }
    player_struct.hardwarez_status[CPTRIP(STATUS_HARD_TRIPLE)] &= ~(WARE_ON);
}
