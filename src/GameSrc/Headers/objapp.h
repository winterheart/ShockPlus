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
#ifndef __OBJAPP_H
#define __OBJAPP_H

#pragma pack(push, 2)

/*
** $Header: r:/prj/cit/src/inc/RCS/objapp.h 1.25 1994/08/30 07:15:21 xemu Exp $
*
*/

//
// An ObjClass is an enum encompassing all the different classes
// in the world.  Be sure to set NUM_CLASSES and CLASS_FIRST
// correctly.
//
// An ObjRefState specifies the location of an ObjRef.
//
// It is made up of two parts:
//
//   - ObjRefStateBin is used to choose where to put an ObjRef
//     in the world-wide data structure.  All ObjRefs with the
//     same ObjRefStateBin are part of the same chain.
//
//   - ObjRefStateInfo is extra information associated with an
//     ObjRef's location.  For example, in Freefall we specify
//     here what rendering triangles an object overlaps.  If
//     you have no such information in your application, then
//     #define NO_OBJ_REF_STATE_INFO.
//
// There must be a specific ObjRefStateBin which is "null".
// For example, it is used to terminate lists of ObjRefStateBins.
// The ObjRefStateBinSetNull and -CheckNull macros respectively
// set a bin to be null and check if it is null.
//
// An ObjLoc specifies the location of an Obj.
// An ObjInfo contains any extra information that you need to have associated
// with each and every object.
//
// #define HASH_OBJECTS if you want ObjRef chains to be hashed
// by location.  You must then define the number of entries, and
// how many of those entries are accessible by the hashing function.
// (I recommend keeping these in the ratio 2:1).  You must also write
// a macro (or function if you prefer) that computes a number n such that
// OBJ_HASH_HEAD_ENTRIES_START <= n < OBJ_HASH_ENTRIES, given an
// ObjRefStateBin.
//
// If you do not #define HASH_OBJECTS, you must write a macro that provides
// the head of the ObjRef chain for a given bin.  (You are responsible for
// keeping a 2-dimensional array of bins, or whatever.)  This must be a macro
// so that the object system can take the address of the result.

////////////////////////////// HERE IS THE STUFF YOU MUST CHANGE
//                          //
//                          ////////////////////////////////


#include "rect.h"

#define NO_OBJ_REF_STATE_INFO

// enumeration of classes
// ## INSERT NEW CLASS HERE
// Note these have a fixed size and fixed values in the game files. They are
// as defined constants than an enum, and gcc does not like to make enums
// 8-bit values.
typedef uint8_t ObjClass;
#define CLASS_GUN 0
#define CLASS_AMMO 1
#define CLASS_PHYSICS 2
#define CLASS_GRENADE 3
#define CLASS_DRUG 4
#define CLASS_HARDWARE 5
#define CLASS_SOFTWARE 6
#define CLASS_BIGSTUFF 7
#define CLASS_SMALLSTUFF 8
#define CLASS_FIXTURE 9
#define CLASS_DOOR 10
#define CLASS_ANIMATING 11
#define CLASS_TRAP 12
#define CLASS_CONTAINER 13
#define CLASS_CRITTER 14
#define NUM_CLASSES 15
#define CLASS_FIRST CLASS_GUN

// The total number of objects in the game, and of each type
// ## INSERT NEW CLASS HERE

#define NUM_OBJECTS 872
#define NUM_OBJECTS_GUN 16
#define NUM_OBJECTS_AMMO 32
#define NUM_OBJECTS_PHYSICS 32
#define NUM_OBJECTS_GRENADE 32
#define NUM_OBJECTS_DRUG 32
#define NUM_OBJECTS_HARDWARE 8
#define NUM_OBJECTS_SOFTWARE 16
#define NUM_OBJECTS_BIGSTUFF 176
#define NUM_OBJECTS_SMALLSTUFF 128
#define NUM_OBJECTS_FIXTURE 64
#define NUM_OBJECTS_DOOR 64
#define NUM_OBJECTS_ANIMATING 32
#define NUM_OBJECTS_TRAP 160
#define NUM_OBJECTS_CONTAINER 64
#define NUM_OBJECTS_CRITTER 64

// THe total number of references of objects
#define NUM_REF_OBJECTS 1600

// i hate cpp, no sizeof() in #if, so we have to do this
#define SIZEOF_AN_OBJREFSTATEBIN 4
typedef struct {
    LGPoint sq;
} ObjRefStateBin;

// i hate cpp, no sizeof() in #if, so we have to do this
#define SIZEOF_AN_OBJREFSTATEINFO 1
typedef struct {
    uint8_t flags;
} ObjRefStateInfo;

#define ObjRefStateBinSetNull(bin) PointSetNull((bin).sq)
#define ObjRefStateBinCheckNull(bin) (PointCheckNull((bin).sq))

// i hate cpp, no sizeof() in #if, so we have to do this
#define SIZEOF_AN_OBJLOC 8
typedef struct {
    uint16_t x; // x-coord, high 8 bits - what square, low 8 bits - where within square
    uint16_t y; // y-coord, high 8 bits - what square, low 8 bits - where within square
    uint8_t z;  // z-coord
    uint8_t p;  // pitch
    uint8_t h;  // heading
    uint8_t b;  // bank
} ObjLoc;

#define OBJ_LOC_BIN_X(oloc) ((oloc).x >> 8u)
#define OBJ_LOC_BIN_Y(oloc) ((oloc).y >> 8u)
#define OBJ_LOC_FINE_X(oloc) ((ushort)((oloc).x & 0xFF00u))
#define OBJ_LOC_FINE_Y(oloc) ((ushort)((oloc).y & 0xFF00u))
#ifdef SAFE_FIX
#define OBJ_LOC_VAL_TO_FIX(value) (fix_make((value >> 8), ((value & 0xFF00) << 8)))
#else
#define OBJ_LOC_VAL_TO_FIX(value) (((fix)value) << 8)
#endif

typedef struct {
    int8_t ph;
    int8_t type;
    int16_t current_hp;
    uint8_t make_info;      // maker, as in Zortech MK III laser rifle or whatever
    uint8_t current_frame;  // animdata
    uint8_t time_remainder; // animdata
    uint8_t inst_flags;     // flags for instance data. Right now 0x01 is used by Mahk's render tricks
} ObjInfo;

#ifdef HASH_OBJECTS
#define OBJ_HASH_ENTRIES 512
#define OBJ_HASH_HEAD_ENTRIES 256
#define OBJ_HASH_HEAD_ENTRIES_START (OBJ_HASH_ENTRIES - OBJ_HASH_HEAD_ENTRIES)
#define OBJ_HASH_FUNC(bin) \
    ((((((bin).sq.x) << 2) + ((bin).sq.y)) & (OBJ_HASH_HEAD_ENTRIES - 1)) + OBJ_HASH_HEAD_ENTRIES_START)
#define ObjRefHead(bin) (objHashTable[ObjGetHashElem((bin), FALSE)].ref) /* don't change this */
#else
#define ObjRefHead(bin) (MAP_GET_XY((bin).sq.x, (bin).sq.y))->objRef
#endif

// WASN'T THAT EASY?

typedef struct {
    ObjRefStateBin bin;
} ObjRefState;

// The following macros perform simple comparing and copying operations.
// If your structures are immensely complicated, you can turn them into
// functions.  It will slow things down, though.

// isnt it neat that you cant do sizeof(ObjRefStateBin) in a #if
// i love cpp with an unholy, inhuman, and altogether pathetic way

#if (SIZEOF_AN_OBJREFSTATEBIN == 4)
#define ObjRefStateBinEqual(bin1, bin2) (*((int *)(&bin1)) == *((int *)(&bin2)))
#elif (SIZEOF_AN_OBJREFSTATEBIN == 2)
#define ObjRefStateBinEqual(bin1, bin2) (*((short *)(&bin1)) == *((short *)(&bin2)))
#elif (SIZEOF_AN_OBJREFSTATEBIN == 1)
#define ObjRefStateBinEqual(bin1, bin2) (*((char *)(&bin1)) == *((char *)(&bin2)))
#else
#define ObjRefStateBinEqual(bin1, bin2) (!memcmp(&(bin1), &(bin2), sizeof(ObjRefStateBin)))
#endif
#define ObjRefStateBinCopy(srcbin, dstbin) \
    do {                                   \
        dstbin = srcbin;                   \
    } while (0)

#ifndef NO_OBJ_REF_STATE_INFO
#if (SIZEOF_AN_OBJREFSTATEINFO == 4)
#define ObjRefStateInfoEqual(info1, info2) (*((int *)(&info1)) == *((int *)(&info2)))
#elif (SIZEOF_AN_OBJREFSTATEINFO == 2)
#define ObjRefStateInfoEqual(info1, info2) (*((short *)(&info1)) == *((short *)(&info2)))
#elif (SIZEOF_AN_OBJREFSTATEINFO == 1)
#define ObjRefStateInfoEqual(info1, info2) (*((char *)(&info1)) == *((char *)(&info2)))
#else
#define ObjRefStateInfoEqual(info1, info2) (!memcmp(&(info1), &(info2), sizeof(ObjRefStateInfo)))
#endif
#define ObjRefStateInfoCopy(srcinfo, dstinfo) \
    do {                                      \
        dstinfo = srcinfo;                    \
    } while (0)
#endif

#if (SIZEOF_AN_OBJLOC == 4)
#define ObjLocEqual(bin1, bin2) (*((int *)(&bin1)) == *((int *)(&bin2)))
#elif (SIZEOF_AN_OBJLOC == 2)
#define ObjLocEqual(bin1, bin2) (*((short *)(&bin1)) == *((short *)(&bin2)))
#elif (SIZEOF_AN_OBJLOC == 1)
#define ObjLocEqual(bin1, bin2) (*((char *)(&bin1)) == *((char *)(&bin2)))
#else
#define ObjLocEqual(bin1, bin2) (!memcmp(&(bin1), &(bin2), sizeof(ObjLoc)))
#endif
#define ObjLocCopy(srcbin, dstbin) \
    do {                           \
        dstbin = srcbin;           \
    } while (0)

//
// Here are prototypes of a few functions you should define in
// objapp.c.  Any macros above that you decided to turn into functions
// should also be defined in objapp.c.
//

void ObjInfoInit(ObjInfo *info);

// This should initialize the following iterator.  Nice name, huh?

void ObjRefStateBinIteratorInit();

//
// After ObjRefStateBinIteratorInit () has been called, calling this
// should put a new valid ObjRefStateBin in bin every time it is called.
// It returns FALSE if it has already returned all valid bins (in which
// case the value of bin is undefined); otherwise it returns TRUE, of
// course.
//
// It can use static variables; the iterator is guaranteed to be active
// only once at a time.

uchar ObjRefStateBinIterator(ObjRefStateBin *bin);

#pragma pack(pop)

#endif // OBJAPP_H
