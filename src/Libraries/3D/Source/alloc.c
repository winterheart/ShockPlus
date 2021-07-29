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
//
// $Source: r:/prj/lib/src/3d/RCS/alloc.c $
// $Revision: 1.19 $
// $Author: jaemz $
// $Date: 1994/09/28 19:01:01 $
//
// Point allocation, system init and shutdown
//

#include "3d.h"
#include "globalv.h"
#include "lg.h"

//----------------------------------------------------------------------------
// Function: short g3_init(short max_points,int user_x_axis,int user_y_axis,int
// user_z_axis)
//
//
//
//
// Input:
// Output:
// Side effects:	allocates point array
//----------------------------------------------------------------------------
/**
 * Starts up the 3d system, allocating the requested number of points, installing the divide overflow handler,
 * and setting up the axes
 * @param max_points number of points requested axis numbers
 * @param user_x_axis
 * @param user_y_axis
 * @param user_z_axis
 * @return number actually allocated
 */
short g3_init(short max_points, int user_x_axis, int user_y_axis, int user_z_axis) {
    int temp_user_y_axis;
    long temp_long;
    char temp_char;
    long allocSize;
    char temp_neg_flags[3] = {0, 0, 0};

    // set axis neg flags
    axis_swap_flag = 0;
    axis_neg_flag = 0;

    temp_user_y_axis = user_y_axis;
    if (user_x_axis < 0) {
        user_x_axis = -user_x_axis;
        temp_neg_flags[0] = 1;
    }
    axis_x = user_x_axis;

    if (user_y_axis < 0) {
        user_y_axis = -user_y_axis;
        temp_neg_flags[1] = 1;
    }
    axis_y = user_y_axis;

    if (user_z_axis < 0) {
        user_z_axis = -user_z_axis;
        temp_neg_flags[2] = 1;
    }
    axis_z = user_z_axis;

    // set axis swap flags
    if (user_x_axis >= user_y_axis) {
        axis_swap_flag |= 1;
        temp_long = user_x_axis;
        user_x_axis = user_y_axis;
        user_y_axis = temp_long;

        temp_char = temp_neg_flags[0];
        temp_neg_flags[0] = temp_neg_flags[1];
        temp_neg_flags[1] = temp_char;
    }

    if (user_x_axis >= user_z_axis) {
        axis_swap_flag |= 2;
        temp_long = user_x_axis;
        user_x_axis = user_z_axis;
        user_z_axis = temp_long;

        temp_char = temp_neg_flags[0];
        temp_neg_flags[0] = temp_neg_flags[2];
        temp_neg_flags[2] = temp_char;
    }

    if (user_y_axis >= user_z_axis) {
        axis_swap_flag |= 4;
        temp_char = temp_neg_flags[1];
        temp_neg_flags[1] = temp_neg_flags[2];
        temp_neg_flags[2] = temp_char;
    }

    // set neg flags bitmask
    axis_neg_flag = (temp_neg_flags[2] << 2) | (temp_neg_flags[1] << 1) | temp_neg_flags[0];

    user_y_axis = temp_user_y_axis - 1;
    up_axis = (user_y_axis << 1) + user_y_axis;

    // set axis offset vars. offset is number of elements, not bytes
    axis_x_ofs = ((axis_x - 1) << 1) + (axis_x - 1);
    axis_y_ofs = ((axis_y - 1) << 1) + (axis_y - 1);
    axis_z_ofs = ((axis_z - 1) << 1) + (axis_z - 1);

    // get pixel ratio
    pixel_ratio = grd_cap->aspect;

    // now allocate point memory

    //	_mark_	<initialize 3d system>
    allocSize = max_points * sizeof(g3s_point);

    point_list = (g3s_point *)malloc(allocSize);
    if (!point_list)
        return (0);

    n_points = max_points;
    return (n_points);
}

void g3_start_frame(void) {
    g3s_point *pt3;

    // get pixel ratio again in case it's changed
    pixel_ratio = grd_cap->aspect;

    //>>>>>>> 1.15
    // set up window vars
    window_width = grd_canvas->bm.w;
    _biasx = _scrw = window_width << 15;
    ww2 = _scrw >> 16;

    window_height = grd_canvas->bm.h;
    _biasy = _scrh = window_height << 15;
    wh2 = _scrh >> 16;

    // mark all points as free
    if (n_points) {
        first_free = point_list;
        pt3 = point_list;
        for (int i = 0; i < n_points - 1; i++, pt3++)
            pt3->next = (g3s_phandle)(pt3 + 1);

        pt3->next = 0L;
    }
}

/**
 * Shut down the 3d system
 */
void g3_shutdown(void) {
    if (point_list)
        free(point_list);

    n_points = 0;
    first_free = 0;
}

/**
 * Does exactly what you would think
 * @return number of free points
 */
int g3_count_free_points(void) {
    int i = 0;
    g3s_point *free_p = first_free;

    while (free_p) {
        i++;
        free_p = free_p->next;
    }
    return i;
}

// check if all points free. returns number of points lost
int g3_end_frame(void) { return (g3_count_free_points() - n_points); }

// allocate a list of points
int g3_alloc_list(int n, g3s_phandle *p) {
    g3s_point *cur_ptr;

    if (!first_free)
        return (0);
    cur_ptr = first_free;

    for (int i = 0; i < n; i++) {
        p[i] = cur_ptr;
        cur_ptr = cur_ptr->next;
    }

    first_free = cur_ptr;
    return n;
}

// free the point in eax. trashes ebx
// adds to free list
void g3_free_point(g3s_phandle p) {
    p->next = first_free;
    first_free = p;
}

// free the list of points pointed at by esi, count in ecx
// adds to free list
void g3_free_list(int n_points, g3s_phandle *p) {
    g3s_point *tempPtr;

    for (int i = 0; i < n_points; i++) {
        tempPtr = p[i];
        tempPtr->next = first_free;
        first_free = tempPtr;
    }
}
