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
#ifndef __GAMEREND_H
#define __GAMEREND_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/gamerend.h $
 * $Revision: 1.5 $
 * $Author: tjs $
 * $Date: 1994/08/14 02:33:18 $
 *
 * $Log: gamerend.h $
 * Revision 1.5  1994/08/14  02:33:18  tjs
 * time limit
 *
 * Revision 1.4  1994/05/20  03:39:31  dc
 * dmg types
 *
 * Revision 1.3  1994/05/09  06:06:10  dc
 * protoypes and defines for secret_fx
 *
 * Revision 1.2  1993/12/21  03:03:47  minman
 * added gamerend_init() prototype
 *
 * Revision 1.1  1993/10/18  23:42:26  xemu
 * Initial revision
 *
 *
 */

// Includes
#include "2d.h"
#include "lg_error.h"

#define SNOW_COLOR_SET   0
#define BLOOD_COLOR_SET  1
#define SHIELD_COLOR_SET 2

#define TYPE_REND_SFX 0x0F00
#define VAL_REND_SFX  0x00FF

#define DYING_REND_SFX     0x0100
#define REBORN_REND_SFX    0x0200
#define FAKEWIN_REND_SFX   0x0300
#define TIMELIMIT_REND_SFX 0x0400

#define DMG_SHIELD 0
#define DMG_BLOOD  1
#define DMG_RAD    2

// Prototypes
void begin_shodan_conquer_fx(uchar begin);
errtype gamerend_init(void);
void set_dmg_percentage(int which, ubyte percent);
void draw_full_static(grs_bitmap *stat_dest, int c_base);

int gamesys_draw_func(void *fake_dest_canvas, void *fake_dest_bm, int x, int y, int flags);
void gamesys_render_func(void *fake_dest_bitmap, int flags);
void set_shield_raisage(uchar going_up);

extern ulong secret_sfx_time;
extern int secret_render_fx;
extern short vhold_shift;

#endif // __GAMEREND_H
