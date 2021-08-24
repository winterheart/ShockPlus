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
 * $Source: r:/prj/lib/src/2d/RCS/detect.c $
 * $Revision: 1.11 $
 * $Author: kevin $
 * $Date: 1994/08/16 13:16:57 $
 *
 * Routine to detect what kind of video card is present and which
 * graphics modes are available.
 *
 * This file is part of the 2d library.
 */

#include "grs.h"
#include "bitmap.h"
#include "cnvtab.h"
#include "idevice.h"
#include "mode.h"
#include "tabdat.h"

// extern
extern void (**grd_device_table_list[])();

// ======================================================================
// Mac version of gr_detect
int gr_detect(grs_sys_info *info) {
    /* default to 640x480x8 standard Mac res */
    info->id_maj = 0;
    info->id_min = 0;
    info->memory = 300;
    info->modes[0] = GRM_640x480x8;
    info->modes[1] = GRM_320x400x8;
    info->modes[2] = GRM_640x400x8;
    info->modes[3] = GRM_320x200x8;
    info->modes[4] = GRM_1024x768x8;

    grd_device_table = grd_device_table_list[info->id_maj];
    grd_canvas_table_list[BMT_DEVICE] = (void (**)())grd_device_table[GRT_CANVAS_TABLE];

    return (0);
}
