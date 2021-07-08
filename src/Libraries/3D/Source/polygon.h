/*

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

#ifndef _POLYGON_H
#define _POLYGON_H

#define MAX_VERTS 100 // max for one poly

// array of 2d points
extern grs_vertex p_vlist[MAX_VERTS];
extern grs_vertex *p_vpl[MAX_VERTS];

extern long _n_verts;


// globals
extern char gour_flag; // gour flag for actual polygon drawer

// arrays of point handles, used in clipping
extern g3s_phandle vbuf[];
extern g3s_phandle _vbuf2[];

// prototypes
int check_and_draw_common(long c, int n_verts, g3s_phandle *p);
int draw_poly_common(long c, int n_verts, g3s_phandle *p);
int draw_line_common(g3s_phandle p0, g3s_phandle p1);

#endif