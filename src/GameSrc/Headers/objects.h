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

#ifndef __OBJECTS_H
#define __OBJECTS_H

#include "lg_types.h"

/*
** $Header: r:/prj/cit/src/inc/RCS/objects.h 1.18 1994/08/30 07:15:13 xemu Exp $
*
*/

// The overriding concept: there is a master array of all objects in the game.
// This is an array of the Obj structure.  Every object has a distinct ID
// which is its index in the array.  The Obj structure contains only
// information which is common to all objects.  Space for extra information is
// allocated separately, as the amount of extra information varies widely
// from object to object.  There are several different classes of objects
// (the typedef ObjClass), each of which has a distinct array storing
// information specific to objects of that class.  This information is kept
// track of in the following manner: the Obj structure contains a "class"
// field, a "subclass" field, and a "specID" field.  The specID field
// specifies what number object of this particular class this object is;
// any given combination of class and specID is unique.  specID is used
// as an index into that class's array to retrieve class-specific information.

// Within a class, there may be many subclasses.  For example, rocks and
// buildings are both static objects, but rocks probably only need one
// field for physical state while a building would more likely want at
// least four.  They do share the same data structure, but may interpret it
// differently.
typedef uchar ObjSubClass;

// An object is ID'd by its position in the Obj array.  For all of these
// arrays, 0 is a null object, and the zeroth element of the array is
// reserved for storing special information.

typedef short ObjID;
#define OBJ_NULL 0 // null object

typedef short ObjRefID;
#define OBJ_REF_NULL 0

typedef short ObjSpecID;
#define OBJ_SPEC_NULL 0

// Now that we have some basic typedef's, we include the application-specific ones.
#ifndef OBJAPP_H
#include "objapp.h"
#endif

#pragma pack(push, 2)

// The common data for all objects
typedef struct Obj {
    uchar active;         // does this object really exist?
    ObjClass obclass;     // what class this is
    ObjSubClass subclass; // subclass within that class
    ObjSpecID specID;     // ID within that class
    union {
        ObjRefID ref; // what refers to this
        ObjID headused;
    };
    union {
        ObjID next; // next Obj in free chain or used chain
        ObjID headfree;
    };
    ObjID prev;   // prev Obj in used chain
    ObjLoc loc;   // location
    ObjInfo info; // extra, application-specific information
} Obj;

// The "next" field of object 0 is the ID of the first element of the chain
// of "free" objects; objects that are not currently in the world.  The "next"
// field of that object points to the next element in the free chain, and so
// on, until a "next" field of 0 means that there are no more free objects.
//
// For an object not in the free chain, the "next" field points to the next
// object in the used chain.  The "ref" field (sorry, next was taken) of the
// zeroth element points to the head of the used chain.  Every element is in
// either the free chain or the used chain.

// The header for an array of class-specific data
typedef struct ObjSpecHeader {
    uchar size;        // size of array
    uchar struct_size; // size of each element
    char *data;        // pointer to array of class-specific data
} ObjSpecHeader;

// The common part of any class-specific structure.  You can cast any class-specific
// structure to an ObjSpec if you want to write super-general code.
// As with Objs and ObjRefs, the 0th element is reserved, and its next
// field is the head of the free chain, while its id element is the head of the
// used chain.  The next field is the next element in the free chain or used chain,
// as appropriate.

typedef struct ObjSpec {
    union {
        struct {
            ObjID id : 15;   // ID in master list
            ushort tile : 1; // look in tiled array?
        } bits;
        ObjSpecID headused;
    };
    union {
        ObjSpecID next; // next struct in free or used chain
        ObjSpecID headfree;
    };
    ObjSpecID prev; // prev struct in used chain
} ObjSpec;

// The master array of objects
extern Obj objs[NUM_OBJECTS];

// The array of class-specific headers.  Index into this array by an ObjClass.
extern /*const*/ ObjSpecHeader objSpecHeaders[NUM_CLASSES];

//
// Now, we get to actual references of objects.  Since a given object can reside
// in more than one map element (if it is large), we need different map elements to be
// able to refer to it.  A map element, then, contains not the object itself
// (an Obj) but rather a reference to an object (an ObjRef).  Each Obj may
// have several different ObjRefs referring to it, each in a different map element.
// All the ObjRefs referring to a given Obj are linked in a circular list by
// the nextref field, and every Obj contains the ID of some ObjRef referring
// to it in its ref field.  As an Obj moves from location to location, at some
// times occupying just one map element and at other times overlapping two or more,
// appropriate ObjRefs will be created and deleted in those map elements.  Note
// that if we implement an Underworld-like "link" field for objects (used to
// specify that one object somehow "contains" another), it can be put in the
// Obj itself and does not need to be put out in the ObjRef.

typedef struct ObjRef {
    ObjRefState state; // location
    ObjID obj;         // what Obj this refers to
    ObjRefID next;     // next ObjRef in this square, or OBJ_REF_NULL if last
    ObjRefID nextref;  // next ObjRef to refer to the same Obj
} ObjRef;

extern ObjRef objRefs[NUM_REF_OBJECTS];

//
// Routines to make it easy to deal with objects only once

extern uchar objsDealt[NUM_OBJECTS / 8];

#define ObjsClearDealt()                       \
    do {                                       \
        memset(objsDealt, 0, NUM_OBJECTS / 8); \
    } while (0)
#define ObjSetDealt(x)                         \
    do {                                       \
        objsDealt[(x) >> 3] |= (1 << ((x)&7)); \
    } while (0)
#define ObjCheckDealt(x) (objsDealt[(x) >> 3] & (1 << ((x)&7)))

//
// Here is a structure by which an object's location is specified.
// Physics will use it to tell the object manager how to update the world.

#define MAX_REFS_PER_OBJ 12 // set as appropriate

typedef struct ObjLocState {
    ObjID obj; // which obj is this?
    ObjLoc loc;
    ObjRefState refs[MAX_REFS_PER_OBJ + 1]; // list of points extended into
} ObjLocState;

//
// Hashing stuff
//

#ifdef HASH_OBJECTS

typedef short ObjHashElemID;

typedef struct ObjHashElem {
    ObjRefID ref;
    ObjHashElemID next;
} ObjHashElem;

// The entries which can actually be accessed by the hash function
// range from OBJ_HASH_HEAD_ENTRIES_START to that + OBJ_HASH_HEAD_ENTRIES.
// We don't start at zero because we want to reserve the zeroth element
// to be null.

extern ObjHashElem objHashTable[OBJ_HASH_ENTRIES];

// ObjGetHashElem() is called by the macro ObjRefHead(), which tends
// to be called in inner loops.  Thus, making it a function slows things
// down a lot.  The solution used here is to make it a macro that handles
// the simple cases (which happen most of the time) and that calls a function
// when it encounters the complicated case (a chain is hanging off of the
// entry).  This seems to speed up code which calls ObjRefHead() repeatedly
// by a factor of two.
//
// See the full ObjGetHashElem() function in objects.c for a commented
// version of what this is doing.
//
// The global variable is a pain but I don't see a way to get rid of it.

#ifdef USE_FUNCTION_FOR_HASH_GET
ObjHashElemID ObjGetHashElem(ObjRefStateBin thebin, uchar create);
#else
extern ObjHashElemID HASHENTRY; // global, found in objects.c
#define ObjGetHashElem(thebin, create)                                                   \
    (HASHENTRY = OBJ_HASH_FUNC(thebin),                                                  \
     (objHashTable[HASHENTRY].ref == OBJ_REF_NULL                                        \
          ? (create ? HASHENTRY : 0)                                                     \
          : (ObjRefStateBinEqual(objRefs[objHashTable[HASHENTRY].ref].state.bin, thebin) \
                 ? HASHENTRY                                                             \
                 : ObjGetHashElemFromChain(thebin, create, HASHENTRY))))
ObjHashElemID ObjGetHashElemFromChain(ObjRefStateBin bin, uchar create, ObjHashElemID firstentry);
#endif

#endif // HASH_OBJECTS

//
// Public functions
//

void ObjsInit();
uchar ObjAndSpecGrab(ObjClass obclass, ObjID *id, ObjSpecID *specid);
uchar ObjPlace(ObjID id, ObjLoc *loc);
ObjID ObjRefDel(ObjRefID ref);
ObjRefID ObjRefMake(ObjID obj, ObjRefState refstate);
uchar ObjDel(ObjID obj);
uchar ObjUpdateLocs(ObjLocState *olsp);

uchar ObjSysOkay();

#pragma pack(pop)

#endif // __OBJECTS_H
