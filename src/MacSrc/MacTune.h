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
//==============================================================================
//
//		System Shock - ©1994-1995 Looking Glass Technologies, Inc.
//
//		MacTune.h	-	Rewrite of Shock's MLIMBS.H file to use QuickTime MIDI rather than AIL.
//
//==============================================================================

//-----------------
//  TYPES & DEFINES
//-----------------

#define MLIMBS_MAX_SEQUENCES 8
#define MLIMBS_MAX_CHANNELS 8

#include "mlimbs.h"

//-----------------
//  EXTERN GLOBALS
//-----------------
extern bool gReadyToQueue; // True when it's time to queue up a new sequence.

// extern TuneCallBackUPP	gTuneCBProc;						// The tune's callback proc.
// extern CalcTuneTask		gCalcTuneTask;					// Global to hold task info.
// extern TimerUPP			gCalcTuneProcPtr;				// UPP for the 6-second time manager
// tune determiner task.

//-----------------
//  PROTOTYPES
//-----------------
int MacTuneInit(void);
void MacTuneShutdown(void);
int MacTuneLoadTheme(char *theme, int themeID);
void MacTuneStartCurrentTheme(void);
void MacTuneKillCurrentTheme(void);
void MacTunePurgeCurrentTheme(void);
void MacTunePlayTune(int tune);
void MacTuneQueueTune(int tune);
void MacTunePrimeTimer(void);
void MacTuneUpdateVolume(void);
