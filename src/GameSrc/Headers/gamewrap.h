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
#ifndef __GAMEWRAP_H
#define __GAMEWRAP_H

/*
 * $Source: n:/project/cit/src/inc/RCS/gamewrap.h $
 * $Revision: 1.13 $
 * $Author: xemu $
 * $Date: 1994/03/24 01:22:08 $
 *
 *
 */

// Includes

// Remember, change in wrapper.c also

extern char *modding_archive_override;

#define OLD_SAVE_GAME_ID_BASE 550
#define SAVE_GAME_ID_BASE 4000
#define NUM_RESIDS_PER_LEVEL 100
#define CURRENT_GAME_FNAME "CurrentGame.dat"
#define ARCHIVE_FNAME "res/data/archive.dat"
// #define ARCHIVE_FNAME (modding_archive_override != NULL ? modding_archive_override : "res/data/archive.dat")

#define ResIdFromLevel(level) (SAVE_GAME_ID_BASE + (level * NUM_RESIDS_PER_LEVEL) + 2)

// Defines

// Typedefs

// Prototypes

// Loads or saves a game named by fname.
errtype copy_file(char *src_fname, char *dest_fname);

/**
 * Save game state to file
 * @param fname path to file
 * @param comment comment
 * @return OK on success, ERR_FOPEN on problems
 */
errtype save_game(char *fname, char *comment);

/**
 * Load game state from file
 * @param fname path to file
 * @return OK on success
 */
errtype load_game(char *fname);

errtype write_level_to_disk(int idnum, uchar flush_mem);
uchar create_initial_game_func(short keycode, ulong context, void *data);
errtype load_level_from_file(int level_num);
void startup_game(uchar visible);
void closedown_game(uchar visible);

// Globals

#endif // __GAMEWRAP_H
