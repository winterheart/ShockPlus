
#ifndef __CUTSLOOP_H
#define __CUTSLOOP_H

// Includes
#include <SDL_audio.h>

// C Library Includes

// System Library Includes

// Master Game Includes

// Game Library Includes

// Game Object Includes

// Defines

// CC: These are all wrong, should find the right resource IDs
#define START_CUTSCENE 0
#define DEATH_CUTSCENE 1
#define WIN_CUTSCENE 2
#define ENDGAME_CUTSCENE 3

// Prototypes

void cutscene_loop(void);
void cutscene_start(void);
void cutscene_exit(void);
short play_cutscene(int id, bool show_credits);

// Globals
extern SDL_AudioStream *cutscene_audiostream;

#endif // __CUTSLOOP_H