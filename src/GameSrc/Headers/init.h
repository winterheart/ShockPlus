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
#ifndef __INIT_H
#define __INIT_H

#include "lg_error.h"
#include "lg_types.h"

// init.h
extern void init_all(void);
extern void free_all(void);
extern uchar ppall[]; // pointer to main shadow palette

extern void shock_alloc_ipal();

errtype load_da_palette(void);

void object_data_flush(void);
errtype object_data_load(void);

extern const char *mfdart_files[];
extern int mfdart_res_file;
extern uchar clear_player_data;
extern uchar objdata_loaded;

#endif
