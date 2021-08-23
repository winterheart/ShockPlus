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
 *  $Source: r:/prj/lib/src/2d/RCS/clpltab.c $
 *  $Revision: 1.2 $
 *  $Author: kevin $
 *  $Date: 1994/08/04 09:54:59 $
 */

#include "clpltyp.h"
#include "clpltab.h"
#include "grnull.h"
#include "line.h"

grt_line_clip_fill grd_line_clip_fill_table[GRD_LINE_TYPES] = {
    gri_line_clip_fill,           // GR_LINE
    gri_iline_clip_fill,          // GR_ILINE
    gri_hline_clip_fill,          // GR_HLINE
    gri_vline_clip_fill,          // GR_VLINE
    gri_sline_clip_fill,          // GR_SLINE
    gri_cline_clip_fill,          // GR_CLINE
    gri_wire_poly_line_clip_fill, // GR_WIRE_POLY_LINE
    gr_null,                      // GR_WIRE_POLY_SLINE
    gri_wire_poly_cline_clip_fill // GR_WIRE_POLY_CLINE
};

grt_line_clip_fill *grd_line_clip_fill_vector = grd_line_clip_fill_table;
