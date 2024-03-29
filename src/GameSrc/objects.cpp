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
** $Header: r:/prj/cit/src/RCS/objects.c 1.13 1994/08/31 16:27:36 tjs Exp $
*
*/

//
// READ ME FIRST!
//
// So then, let us determine some determinology for our terms, here
//
// There are really three places a thing can be:
//   WORLD: in the world and being used
//   FLOATING: existent but not connected to the real world in any way
//   UNUSED: not used at all, ready to be reclaimed
//
// Any function modifying the world in some way will probably contain
// one of the following words:
//
// Grab: get an unused thing                  UNUSED -> FLOATING
// Free: return a thing to the unused pile  FLOATING -> UNUSED
//
//  Add: add to reality                     FLOATING -> WORLD
//  Rem: remove from reality                   WORLD -> FLOATING
//
// Make: spring fully formed into the world   UNUSED -> WORLD
//  Del: terminate with extreme prejudice      WORLD -> UNUSED
//
// For things for which there is no real FLOATING state, Del and Make are used.
// Basically, whenever you have Rem'ed something, assume that it is
//   still floating and that you must do something more to it.
//
// Object system functions tend to have names beginning with "Obj."
// If one main argument is being passed in, and it would otherwise
//   be unclear what type of argument it is, that type is then put in the name.
// Then follow any additional nouns and verbs describing what the function does.
// All Objs, ObjRefs, ObjSpecs, etc. are referred to by their ID's.
// Thus, ObjRefLinkDel deletes a link from an ObjRef to an Obj and is passed an ObjRefID.
//

#include <cstring>

#include "objects.h"
#include "map.h"

// See objects.h and objapp.h for a description of global variables

#ifdef SUPPORT_VERSION_26_OBJS
old_Obj old_objs[NUM_OBJECTS];
#endif

#ifdef THESE_WERENT_IN_STATIC_S_BUT_THEY_ARE_SO_THESE_ARENT_REALLY_HERE
Obj objs[NUM_OBJECTS];
ObjRef objRefs[NUM_REF_OBJECTS];
uchar objsDealt[NUM_OBJECTS / 8];
#endif

#ifdef HASH_OBJECTS

ObjHashElem objHashTable[OBJ_HASH_ENTRIES];

uchar ObjDeleteHashElem(ObjRefStateBin bin);

#ifndef USE_FUNCTION_FOR_HASH_GET
ObjHashElemID HASHENTRY;
#endif

#endif // HASH_OBJECTS

static void ObjRefRem(ObjRefID ref);
static uchar ObjLinkMake(ObjRefID ref, ObjID obj);
static ObjID ObjRefLinkDel(ObjRefID ref);
static uchar ObjRefAdd(ObjRefID ref, ObjRefState refstate);
static uchar ObjDelRefs(ObjID obj);
static ObjID ObjGrab();
static uchar ObjFree(ObjID obj);
static ObjRefID ObjRefGrab();
static ObjID ObjRefFree(ObjRefID ref, uchar cleanup);
static ObjSpecID ObjSpecGrab(ObjClass obclass);
static uchar ObjSpecFree(ObjClass obclass, ObjSpecID id);
static uchar ObjAndSpecFree(ObjID obj);

//
// PUBLIC FUNCTIONS
//

//
// Initializes all object stuff
//
// This entire function should be made faster, as it is currently quite stupid.
//
void ObjsInit() {
    int i;
    short c;
    ObjSpecHeader *head;
    ObjSpec *os;

    // clear out everything
    memset((void *)objs, 0, sizeof(Obj) * NUM_OBJECTS);
    memset((void *)objRefs, 0, sizeof(ObjRef) * NUM_REF_OBJECTS);
    for (c = CLASS_FIRST; c < NUM_CLASSES; c++) {
        head = &objSpecHeaders[c];
        memset((void *)head->data, 0, head->size * head->struct_size);
    }

    // set up the free chains for objects
    for (i = 0; i < NUM_OBJECTS - 1; i++)
        objs[i].next = i + 1;
    objs[NUM_OBJECTS - 1].next = 0;
    objs[0].headused = 0;

    // set up the free chains for references
    for (i = 0; i < NUM_REF_OBJECTS - 1; i++)
        objRefs[i].next = i + 1;
    objRefs[NUM_REF_OBJECTS - 1].next = 0;

    // set up the free chains for class-specific info
    for (c = CLASS_FIRST; c < NUM_CLASSES; c++) {
        head = &objSpecHeaders[c];
        ((ObjSpec *)(head->data))->bits.id = 0; // really head of used chain
        for (i = 0; i < head->size - 1; i++) {
            os = (ObjSpec *)(head->data + i * head->struct_size);
            os->next = i + 1;
        }
        ((ObjSpec *)(head->data + (head->size - 1) * head->struct_size))->next = 0;
    }

#ifdef HASH_OBJECTS
    // set up the free chains for the hash table
    LG_memset((void *)objHashTable, 0, sizeof(ObjHashElem) * OBJ_HASH_ENTRIES);
    for (i = 0; i < OBJ_HASH_HEAD_ENTRIES_START - 1; i++)
        objHashTable[i].next = i + 1;
    objHashTable[OBJ_HASH_HEAD_ENTRIES_START - 1].next = 0;
#endif
}

//
// Finds a free Obj and a free ObjSpec of the appropriate class, and
// links them together.  Fills up the given fields.  Returns whether it
// succeeded.
//
uchar ObjAndSpecGrab(ObjClass obclass, ObjID *id, ObjSpecID *specid) {
    ObjSpecHeader *head;

    if ((*id = ObjGrab()) == OBJ_NULL)
        return false;
    if ((*specid = ObjSpecGrab(obclass)) == OBJ_SPEC_NULL) {
        ObjFree(*id);
        return false;
    }
    objs[*id].obclass = obclass;
    objs[*id].specID = *specid;
    head = &objSpecHeaders[obclass];
    ((ObjSpec *)(head->data + *specid * head->struct_size))->bits.id = *id;

    return true;
}

//
// Sets the given fields of an object appropriately, and makes it active.
// Currently always returns true.
//
uchar ObjPlace(ObjID id, ObjLoc *loc) {
    ObjLocCopy(*loc, objs[id].loc);
    return true;
}

//
// Removes the specified ObjRef from the map,
//   destroys its reference to its Obj,
//   and then destroys it.
//
// Quite a vicious little function really.
//
ObjID ObjRefDel(ObjRefID ref) {
    uchar tmp;

    ObjRefRem(ref);
    tmp = ObjRefFree(ref, true);

    return tmp;
}

//
// Gets a free ObjRef, assigns it to the given Obj,
// and puts it in the world in the given place.
//
// Returns the ObjRefID used, or OBJ_REF_NULL if it
// couldn't do it for any reason.
//
ObjRefID ObjRefMake(ObjID obj, ObjRefState refstate) {
    ObjRefID ref;
    if ((ref = ObjRefGrab()) == OBJ_REF_NULL)
        return OBJ_REF_NULL;

    ObjLinkMake(ref, obj);
    ObjRefAdd(ref, refstate);
    return ref;
}

//
// Deletes the given obj and all its references.
// Returns whether success was achieved in this quest.
//
uchar ObjDel(ObjID obj) {
    ObjDelRefs(obj);
    ObjAndSpecFree(obj);

    return true;
}

//
// Updates the location of a moving object.
// Returns whether it was successful in doing so.
//
// The given ObjLocState contains information about the new location of the
// object.  We know the old location of the object by following ref pointers
// around.  This function updates all the ObjRefs's referring to that object
// correctly.
//
uchar ObjUpdateLocs(ObjLocState *olsp) {
    ObjID obj;                        // this object
    ObjRefID ref;                     // each reference of it
    ObjRefID firstref;                // first ref that refers to it
    ObjRefState *newrefs;             // squares the object will soon be in
    ObjRefState in[MAX_REFS_PER_OBJ]; // places moved into
    ObjRefID out[MAX_REFS_PER_OBJ];   // old references of the object
    int incount = 0;                  // number of places moved into
    int outcount = 0;                 // number of places moved out of
    int newcount;                     // # of new square being moved into
    ObjRefState *stCur;               // current place being checked
    int i;                            // loopy loopy

    // Get some data first
    obj = olsp->obj;
    newrefs = olsp->refs;
    ObjLocCopy(olsp->loc, objs[obj].loc);

    // Figure out where we are now
    outcount = 0;
    firstref = ref = objs[obj].ref;

    if (firstref != OBJ_REF_NULL) {
        // We use a do-while so that we don't fail the loop condition
        // at the very beginning of the loop.
        do {
            out[outcount++] = ref;
            ref = objRefs[ref].nextref;
        } while (ref != firstref);
    }

    // Now, for each square we will be in,
    //
    //   - if the bin is in out[], then we are staying in it;
    //     update its info field and then delete it from out[]
    //
    //   - if the bin is not in out[], then we are moving into it;
    //     add it to in[]
    //
    // At the end of the following loop, all the bins we are moving out of
    // will be in out[], and all the bins we are moving into will be in in[].

    incount = 0;
    for (newcount = 0; !ObjRefStateBinCheckNull(newrefs[newcount].bin); newcount++) {
        stCur = &newrefs[newcount];

        // Check all bins in out[] for a match
        for (i = 0; i < outcount; i++) {
            if (ObjRefStateBinEqual(objRefs[out[i]].state.bin, stCur->bin)) {
#ifndef NO_OBJ_REF_STATE_INFO
                objRefs[out[i]].state.info = stCur->info; // update info
#endif
                out[i] = out[--outcount]; // delete this bin from out
                goto found_bin_in_out;    // go back to the outer loop
            }
        }

        // this bin was not in out[], so we add it to in[].
        in[incount++] = *stCur;

    found_bin_in_out:;
    }

    // Now we must delete the references in out[] and add the ones in in[].
    // Note that we might be able to speed up the following code if we had to
    // by, instead of freeing RefID's and then grabbing them again, reusing
    // them somehow.  It is unclear how often both out[] and in[] will have
    // stuff in them; if this case occurs often it would probably be worth
    // speeding up.

    for (i = 0; i < outcount; i++)
        ObjRefDel(out[i]);

    for (i = 0; i < incount; i++)
        if (!ObjRefMake(obj, in[i]))
            return false;

    return true;
}

#ifdef HASH_OBJECTS
//////////////////////////////
//
// Return a free hash table element, or 0 if none.
//
ObjHashElemID ObjGrabHashEntry(void) {
    ObjHashElemID elem = objHashTable[0].next;

    if (elem == 0)
        return elem;
    objHashTable[0].next = objHashTable[elem].next;
    return elem;
}

//////////////////////////////
//
// Free up the given hash table element.
//
void ObjFreeHashEntry(ObjHashElemID elem) {
    objHashTable[elem].ref = 0; // make sure nobody thinks
                                // there's something here
    objHashTable[elem].next = objHashTable[0].next;
    objHashTable[0].next = elem;
}

//////////////////////////////
//
// Returns the ID of a hash entry pointing to the contents of the given bin.
// If create is true, then return an entry even if once doesn't exist now.
// It will then have a ref field of NULL, signifying that it was just
// created.  You must immediately set the ref's StateBin correctly.
//
#ifdef USE_FUNCTION_FOR_HASH_GET
ObjHashElemID ObjGetHashElem(ObjRefStateBin bin, uchar create) {
    ObjHashElemID entry;
    ObjHashElemID firstentry, nextentry;

    entry = OBJ_HASH_FUNC(bin);
    if (objHashTable[entry].ref == OBJ_REF_NULL) {
        // Nothing at all at this hash location
        if (!create)
            return 0;
        else
            return entry;
    }

    // Check if the first element is correct
    if (ObjRefStateBinEqual(objRefs[objHashTable[entry].ref].state.bin, bin))
        return entry;

    // Go through the list looking for the right object chain
    firstentry = entry;
    while (objHashTable[entry].next != 0) {
        nextentry = objHashTable[entry].next;
        if (ObjRefStateBinEqual(objRefs[objHashTable[nextentry].ref].state.bin, bin)) {
            ObjRefID tmpref;

            // Move nextentry to the top
            tmpref = objHashTable[firstentry].ref;
            objHashTable[firstentry].ref = objHashTable[nextentry].ref;
            objHashTable[nextentry].ref = tmpref;
            return firstentry;
        }
        entry = nextentry;
    }

    // Couldn't find it
    if (!create)
        return 0;

    // Make a new one
    if ((nextentry = ObjGrabHashEntry()) == 0)
        return 0;
    objHashTable[entry].next = nextentry;
    objHashTable[nextentry].ref = objHashTable[firstentry].ref;
    objHashTable[nextentry].next = 0;
    objHashTable[firstentry].ref = OBJ_REF_NULL;
    return firstentry;
}
#else
//////////////////////////////
//
// This is just the special case of the ObjGetHashElem() function, which is now
// called from the macro version of ObjGetHashElem() when it sees that it needs
// it.  At this point, the entry is used, but not by us, so we go down the
// chain looking for the right entry.
//
ObjHashElemID ObjGetHashElemFromChain(ObjRefStateBin bin, uchar create, ObjHashElemID firstentry) {
    ObjHashElemID entry = firstentry;
    ObjHashElemID nextentry;

    // Go through the list looking for the right object chain
    while (objHashTable[entry].next != 0) {
        nextentry = objHashTable[entry].next;
        if (ObjRefStateBinEqual(objRefs[objHashTable[nextentry].ref].state.bin, bin)) {
            ObjRefID tmpref;

            // Move nextentry to the top
            tmpref = objHashTable[firstentry].ref;
            objHashTable[firstentry].ref = objHashTable[nextentry].ref;
            objHashTable[nextentry].ref = tmpref;
            return firstentry;
        }
        entry = nextentry;
    }

    // Couldn't find it
    if (!create)
        return 0;

    // Make a new one
    if ((nextentry = ObjGrabHashEntry()) == 0)
        return 0;
    objHashTable[entry].next = nextentry;
    objHashTable[nextentry].ref = objHashTable[firstentry].ref;
    objHashTable[nextentry].next = 0;
    objHashTable[firstentry].ref = OBJ_REF_NULL;
    return firstentry;
}
#endif

//////////////////////////////
//
// Deletes the entry in the hash table corresponding to the ref
// chain at the given bin.  Returns false if there was nothing to delete.
//
uchar ObjDeleteHashElem(ObjRefStateBin bin) {
    ObjHashElemID firstentry = OBJ_HASH_FUNC(bin);
    ObjHashElemID entry, nextentry;

    if (objHashTable[firstentry].ref == OBJ_REF_NULL)
        return false;

    // This should be true if we always move to the front of the list like above
    if (ObjRefStateBinEqual(objRefs[objHashTable[firstentry].ref].state.bin, bin)) {
        if ((nextentry = objHashTable[firstentry].next) == 0) {
            // This was the only one
            objHashTable[firstentry].ref = OBJ_REF_NULL;
            return true;
        }
        objHashTable[firstentry].ref = objHashTable[nextentry].ref;
        objHashTable[firstentry].next = objHashTable[nextentry].next;
        ObjFreeHashEntry(nextentry);
        return true;
    }

    entry = firstentry;
    nextentry = objHashTable[entry].next;
    while (nextentry != 0) {
        if (ObjRefStateBinEqual(objRefs[objHashTable[nextentry].ref].state.bin, bin)) {
            objHashTable[entry].next = objHashTable[nextentry].next;
            ObjFreeHashEntry(nextentry);
            return true;
        }
        entry = nextentry;
        nextentry = objHashTable[entry].next;
    }
    return false;
}

static int hash_i;

void ObjHashIteratorInit(void) { hash_i = 0; }

uchar ObjHashIterator(ObjRefID *ref) {
    while (hash_i < OBJ_HASH_ENTRIES && objHashTable[hash_i].ref == 0)
        hash_i++;
    if (hash_i == OBJ_HASH_ENTRIES)
        return false;
    *ref = objHashTable[hash_i++].ref;
    return true;
}

#define MAX_CHAIN_LENGTH 10 // let's hope it gets no higher
int numlengths[MAX_CHAIN_LENGTH + 1];
//////////////////////////////
//
//
//
ObjHashStats(void) {
    int i;

    for (i = 0; i <= MAX_CHAIN_LENGTH; i++)
        numlengths[i] = 0;
    for (i = OBJ_HASH_HEAD_ENTRIES_START; i < OBJ_HASH_ENTRIES; i++) {
        int j = i, length = 0;
        if (objHashTable[i].ref != 0) {
            while (j != 0)
                length++, j = objHashTable[j].next;
        }
        if (length > MAX_CHAIN_LENGTH)
            length = MAX_CHAIN_LENGTH;
        numlengths[length]++;
    }
    for (i = 0; i <= MAX_CHAIN_LENGTH; i++) {
        if (numlengths[i] > 0)
            SpewHash(("Object hash chains of length %d: %d\n", i, numlengths[i]));
    }
}

#endif // HASH_OBJECTS

//
// DEBUGGING FUNCTIONS
//

#define OBJ_NO_STATE 0
#define OBJ_FREE 1
#define OBJ_USED 2
#define OBJ_IN_MAP 4

//
// Check whether the object system is consistent.
//
uchar ObjSysOkay() {
    char usedObj[NUM_OBJECTS];
    char usedRef[NUM_REF_OBJECTS];
    ObjID cur;
    ObjSpecHeader *head;
    int i, j;
    ObjRefStateBin refbin;
    ObjRefID ref;

    // 1. Every Obj is in either the free chain or the used chain,
    // and does not appear twice in any chain.

    memset(usedObj, 0, NUM_OBJECTS);

    //	SpewAnal (("Free Objs: "));
    //	DBG_Anal ({RangeInit ();})

    cur = objs[OBJ_NULL].next;
    while (cur) {
        if (cur < 0 || cur >= NUM_OBJECTS) {
            DEBUG("%s: Invalid ID in free chain", __FUNCTION__); // cur
            return false;
        }
        if (objs[cur].active) {
            DEBUG("%s: Active Obj in free chain", __FUNCTION__); // objs[cur]
            return false;
        }
        if (usedObj[cur] == OBJ_FREE) {
            DEBUG("%s: Obj is free more than once", __FUNCTION__); // usedObj[cur]
            return false;
        }
        usedObj[cur] = OBJ_FREE;
        cur = objs[cur].next;
    }

    cur = objs[OBJ_NULL].ref;
    while (cur) {
        if (cur < 0 || cur >= NUM_OBJECTS) {
            DEBUG("%s: Invalid ID in Obj used chain", __FUNCTION__); // cur
            return false;
        }
        if (usedObj[cur] == OBJ_FREE) {
            DEBUG("%s: Obj is free and used chain", __FUNCTION__); // cur
            return false;
        }
        if (usedObj[cur] == OBJ_USED) {
            DEBUG("%s: Obj is used twice", __FUNCTION__); // cur
            return false;
        }
        usedObj[cur] = OBJ_USED;
        cur = objs[cur].next;
    }

    for (cur = 1; cur < NUM_OBJECTS; cur++) {
        if (usedObj[cur] == 0) {
            DEBUG("%s: Obj is neither free nor used", __FUNCTION__); // cur
            return false;
        }
    }

    // 3. No ObjRef occurs twice in the free chain.

    memset(usedRef, 0, NUM_REF_OBJECTS);

    cur = objRefs[OBJ_REF_NULL].next;

    while (cur) {
        if (cur < 0 || cur >= NUM_REF_OBJECTS) {
            DEBUG("%s: Invalid ID in ObjRef free chain", __FUNCTION__); // cur
            return false;
        }
        if (usedRef[cur] == OBJ_FREE) {
            DEBUG("%s: ObjRef is free more than once", __FUNCTION__); // cur
            return false;
        }
        usedRef[cur] = OBJ_FREE;
        cur = objRefs[cur].next;
    }

    // 2. Every ObjSpec is in either the free chain or the used chain, and does
    // not appear twice in any chain.

    for (i = CLASS_FIRST; i < NUM_CLASSES; i++) {
        memset(usedObj, 0, NUM_OBJECTS);
        head = &objSpecHeaders[i];
        cur = ((ObjSpec *)head->data)->next;
        while (cur) {
            if (cur < 0 || cur >= head->size) {
                DEBUG("%s: Invalid ID (cur) in Class (i) free chain", __FUNCTION__);
                return false;
            }
            if (usedObj[cur] == OBJ_FREE) {
                DEBUG("%s: Class (i) ObjSpec (cur) is free more than once", __FUNCTION__);
                return false;
            }
            usedObj[cur] = OBJ_FREE;
            cur = ((ObjSpec *)(head->data + cur * head->struct_size))->next;
        }
        cur = ((ObjSpec *)head->data)->bits.id;
        while (cur) {
            if (cur < 0 || cur >= head->size) {
                DEBUG("%s: Invalid ID (cur) in Class (i) used chain", __FUNCTION__);
                return false;
            }
            if (usedObj[cur] == OBJ_FREE) {
                DEBUG("%s: Class (i) ObjSpec (cur) is free and used", __FUNCTION__);
                return false;
            }
            if (usedObj[cur] == OBJ_USED) {
                DEBUG("%s: Class (i) ObjSpec (cur) is used twice", __FUNCTION__);
                return false;
            }
            usedObj[cur] = OBJ_USED;
            cur = ((ObjSpec *)(head->data + cur * head->struct_size))->next;
        }
        for (cur = 1; cur < head->size; cur++) {
            if (usedObj[cur] == 0) {
                DEBUG("%s: Class (i) ObjSpec (cur) is neither free nor used", __FUNCTION__);
                return false;
            }
        }
    }

    // 4. All active ObjRefs occur exactly once in the map.
    // 5. All active ObjRefs point to the map element in which they occur.
    // 6. All active ObjRefs point to active Objs.

#ifdef HASH_OBJECTS
    ObjHashIteratorInit();
    while (ObjHashIterator(&ref))
#else
    ObjRefStateBinIteratorInit();
    while (ObjRefStateBinIterator(&refbin))
#endif
    {
#ifdef HASH_OBJECTS
        refbin = objRefs[ref].state.bin;
#else
        ref = ObjRefHead(refbin);
#endif

        while (ref != OBJ_REF_NULL) {
            if (usedRef[ref] & OBJ_FREE) {
                DEBUG("%s: ObjRef (ref) is free and in map bin (str)", __FUNCTION__);
                return false;
            }

            if (usedRef[ref] & OBJ_IN_MAP) {
                DEBUG("%s: ObjRef (ref) exists in two bins (one is str)", __FUNCTION__);
                return false;
            }

            if (!ObjRefStateBinEqual(objRefs[ref].state.bin, refbin)) {
                DEBUG("%s: ObjRef (ref) thinks it is in (str2) but is in (str)", __FUNCTION__);
                return false;
            }

            if (objRefs[ref].obj == OBJ_NULL) {
                DEBUG("%s: ObjRef (ref) in (str) points to null Obj", __FUNCTION__);
                return false;
            }

            usedRef[ref] |= OBJ_IN_MAP;

            ref = objRefs[ref].next;
        }
    }

    // 7. All ObjRefs referring to a single Obj reside in distinct map elements.
    // 8. The links between an Obj and its ObjSpec are valid.
    // 9. The links between an Obj and its ObjRefs are valid.

    cur = objs[OBJ_NULL].headused;
    while (cur != OBJ_NULL) {
        ObjRefStateBin refbins[MAX_REFS_PER_OBJ];
        ObjRefID firstref;
        ObjRefID ref;
        int i = 0;

        if (!objs[cur].active)
            goto done_checking_obj;

        ref = firstref = objs[cur].ref;
        if (ref == OBJ_REF_NULL)
            goto done_checking_refs;

        do {
            if (i == MAX_REFS_PER_OBJ) {
                DEBUG("%s: Too many ObjRefs refer to Obj (cur)", __FUNCTION__);
                return false;
            }

            if (objRefs[ref].obj != cur) {
                DEBUG("%s: Obj (cur) points to ObjRef (ref) but not vice versa", __FUNCTION__);
                return false;
            }

            for (j = 0; j < i; j++)
                if (ObjRefStateBinEqual(refbins[j], objRefs[ref].state.bin))
                    return false;

            refbins[i++] = objRefs[ref].state.bin;
            ref = objRefs[ref].nextref;
        } while (ref != firstref);

    done_checking_refs:

        if (objs[cur].specID != OBJ_SPEC_NULL) {
            char *data;
            ObjSpecHeader *head;

            if (objs[cur].obclass < 0 || objs[cur].obclass >= NUM_CLASSES) {
                DEBUG("%s: Obj (cur) has invalid obclass (objs[cur].obclass)", __FUNCTION__);
                return false;
            }

            head = &objSpecHeaders[objs[cur].obclass];
            data = head->data;

            if (((ObjSpec *)(data + head->struct_size * objs[cur].specID))->bits.id != cur ||
                ((ObjSpec *)(data + head->struct_size * objs[cur].specID))->bits.tile == true) {
                DEBUG("%s: Obj (cur) obclass-specific data does not point back to it", __FUNCTION__);
                return false;
            }
        }

    done_checking_obj:

        cur = objs[cur].next;
    }

#ifdef HASH_OBJECTS
    DBG_Hash({ ObjHashStats(); })
#endif

        return true;
}

//
// STATIC FUNCTIONS
//
// Some of these may want to become public at some point.
// However, before making one of these functions public, first
// make sure that there isn't already a public interface that you
// can use instead.
//

//
// Returns a ObjID that is not currently being used
// or OBJ_NULL If there are none
//
// The caller is responsible for setting the fieds of the Obj, including active.
// This function does clear the ref field to ensure we don't follow bad pointers around.
//
static ObjID ObjGrab() {
    ObjID obj; // the next free ObjID
    // all gone
    if (objs[OBJ_NULL].next == OBJ_NULL) {
        return OBJ_NULL;
    }

    // remove the head of the free chain and return it
    obj = objs[OBJ_NULL].next;
    objs[OBJ_NULL].next = objs[obj].next;

    // and put obj at the head of the used chain
    objs[obj].next = objs[OBJ_NULL].headused;
    objs[objs[OBJ_NULL].headused].prev = obj;
    objs[obj].prev = OBJ_NULL;
    objs[OBJ_NULL].headused = obj;

    objs[obj].ref = OBJ_REF_NULL;

    ObjInfoInit(&objs[obj].info);

    return obj;
}

//
// Frees up the given object
// Returns false (and refuses to free the object)
//   if there are objRefs referring to the object
//
static uchar ObjFree(ObjID obj) {
    objs[obj].active = false;

    // take obj out of the used chain
    if (objs[obj].prev == OBJ_NULL)
        objs[OBJ_NULL].headused = objs[obj].next;
    else
        objs[objs[obj].prev].next = objs[obj].next;

    objs[objs[obj].next].prev = objs[obj].prev;
    // don't need to clear objs[obj].next since we are resetting it immediately

    // and put it back at the head of the free chain
    objs[obj].next = objs[OBJ_NULL].next;
    objs[OBJ_NULL].next = obj;

    return true;
}

//
// Returns an ObjRefID that is not currently being used
// or OBJ_REF_NULL if there are none
//
static ObjRefID ObjRefGrab() {
    ObjRefID thisobj; // the next free ObjRefID

    if (objRefs[OBJ_REF_NULL].next == OBJ_REF_NULL) {
        return OBJ_REF_NULL;
    }

    // remove the head of the free chain and return it
    thisobj = objRefs[OBJ_REF_NULL].next;
    objRefs[OBJ_REF_NULL].next = objRefs[thisobj].next;

    objRefs[thisobj].obj = OBJ_NULL;
    objRefs[thisobj].next = OBJ_REF_NULL;
    objRefs[thisobj].nextref = OBJ_REF_NULL;

    return thisobj;
}

//
// Frees up the space used by the ObjRef referred to by ref.
//
// If cleanup is true, also deletes the reference of the ObjRef to the Obj.
//   If this is the last reference to an Obj, returns that Obj's ID.
//
static ObjID ObjRefFree(ObjRefID ref, uchar cleanup) {
    ObjID obj; // the object ref refers to

    if (cleanup)
        obj = ObjRefLinkDel(ref); // remove the link
    else
        obj = OBJ_REF_NULL;
    objRefs[ref].next = objRefs[OBJ_NULL].next;
    objRefs[OBJ_REF_NULL].next = ref;

    return obj;
}

//
// Returns an ObjSpecID of the specified obclass that is not currently being
// used, or OBJ_SPEC_NULL if there are none available
//
static ObjSpecID ObjSpecGrab(ObjClass obclass) {
    char *data;
    ObjSpecHeader *head;
    ObjSpecID thisid;
    ObjSpec *spec0, *thisspec;

    head = &objSpecHeaders[obclass];
    data = head->data;
    spec0 = (ObjSpec *)data;

    if (spec0->next == OBJ_SPEC_NULL)
        return OBJ_SPEC_NULL;

    // remove the head of the free chain and return it
    thisid = spec0->next;
    thisspec = (ObjSpec *)(data + head->struct_size * thisid);
    spec0->next = thisspec->next;

    // and put this at the head of the used chain
    thisspec->next = spec0->headused;
    ((ObjSpec *)(data + head->struct_size * spec0->headused))->prev = thisid;
    thisspec->prev = OBJ_SPEC_NULL;
    spec0->headused = thisid;

    return thisid;
}

#ifdef COMPRESS_OBJSPECS
ObjSpecID HeaderObjSpecGrab(ObjClass obclass, ObjSpecHeader *head) {
    char *data;
    ObjSpecID thisid;
    ObjSpec *spec0, *thisspec;

    SpewReport(("ObjSpecGrab (obclass %d)\n", obclass));

    DBG_Check({
        if (obclass >= NUM_CLASSES) {
            Warning(("Invalid obclass %d in ObjSpecGrab\n", obclass));
            return OBJ_SPEC_NULL;
        }
    })

        data = head->data;
    spec0 = (ObjSpec *)data;

    if (spec0->next == OBJ_SPEC_NULL)
        return OBJ_SPEC_NULL;

    // remove the head of the free chain and return it
    thisid = spec0->next;
    thisspec = (ObjSpec *)(data + head->struct_size * thisid);
    spec0->next = thisspec->next;

    // and put this at the head of the used chain
    thisspec->next = spec0->headused;
    ((ObjSpec *)(data + head->struct_size * spec0->headused))->prev = thisid;
    thisspec->prev = OBJ_SPEC_NULL;
    spec0->headused = thisid;

    return thisid;
}
#endif

//
// Frees up the space used by the ObjSpec in the specified class
// referred to by id.
//
static uchar ObjSpecFree(ObjClass obclass, ObjSpecID id) {
    char *data;
    ObjSpecHeader *head;
    ObjSpec *spec0, *thisspec;

    head = &objSpecHeaders[obclass];
    data = head->data;
    spec0 = (ObjSpec *)&data[0];
    thisspec = (ObjSpec *)(data + head->struct_size * id);

    // take this out of the used chain
    if (thisspec->prev == OBJ_SPEC_NULL)
        spec0->headused = thisspec->next;
    else
        ((ObjSpec *)(data + head->struct_size * thisspec->prev))->next = thisspec->next;

    ((ObjSpec *)(data + head->struct_size * thisspec->next))->prev = thisspec->prev;
    // don't need to clear thisspec->next since we are resetting it immediately

    // and put it back at the head of the free chain
    thisspec->next = spec0->headfree;
    spec0->headfree = id;

    return true;
}

#ifdef COMPRESS_OBJSPECS
uchar HeaderObjSpecFree(ObjClass obclass, ObjSpecID id, ObjSpecHeader *head) {
    char *data;
    ObjSpec *spec0, *thisspec;

    SpewReport(("ObjSpecFree (obclass %d, specid %d)\n", obclass, id));

    DBG_Check({
        if (obclass >= NUM_CLASSES) {
            Warning(("Invalid obclass %d in ObjSpecFree\n", obclass));
            return false;
        }
    })

        data = head->data;
    spec0 = (ObjSpec *)&data[0];
    thisspec = (ObjSpec *)(data + head->struct_size * id);

    // take this out of the used chain
    if (thisspec->prev == OBJ_SPEC_NULL)
        spec0->headused = thisspec->next;
    else
        ((ObjSpec *)(data + head->struct_size * thisspec->prev))->next = thisspec->next;

    ((ObjSpec *)(data + head->struct_size * thisspec->next))->prev = thisspec->prev;
    // don't need to clear thisspec->next since we are resetting it immediately

    // and put it back at the head of the free chain
    thisspec->next = spec0->headfree;
    spec0->headfree = id;

    return true;
}
#endif

#ifdef COMPRESS_OBJSPECS
uchar HeaderObjSpecCopy(ObjClass cls, ObjSpecID old, ObjSpecID new, ObjSpecHeader *head) {
    char *data;
    ObjSpec *spec0;
    int size;

    SpewReport(("ObjSpecCopy (obclass %d)\n", cls));

    DBG_Check({
        if (cls >= NUM_CLASSES) {
            Warning(("Invalid obclass %d in ObjSpecCopy\n", cls));
            return false;
        }
    })

        data = head->data;
    spec0 = (ObjSpec *)data;

    // the ObjSpec (generic) part of the new spec is already set; we
    // need to copy the rest

    if ((size = head->struct_size - sizeof(ObjSpec)) > 0) {
        memcpy(data + head->struct_size * new + sizeof(ObjSpec), data + head->struct_size * old + sizeof(ObjSpec),
               size);
    }
    return true;
}
#endif

//
// Removes the given ObjRef from the object list in a map bin.
//
static void ObjRefRem(ObjRefID ref) {
    ObjRefID *ptr; // what we must change to splice ref out
#ifdef HASH_OBJECTS
    ObjHashElemID hash_entry;
    if ((hash_entry = ObjGetHashElem(objRefs[ref].state.bin, false)) == 0) {
        Warning(("Tried to remove ref %d not in hash table in ObjRefRem\n", ref));
        return;
    }

    if (objHashTable[hash_entry].ref == ref) {
        if (objRefs[ref].next == OBJ_REF_NULL) {
            // This was the only one
            if (!ObjDeleteHashElem(objRefs[ref].state.bin)) {
                Warning(("Couldn't delete refchain %d from hash table in ObjRefRem\n", ref));
                return;
            }
        } else {
            objHashTable[hash_entry].ref = objRefs[ref].next;
        }
        objRefs[ref].next = OBJ_REF_NULL;              // we are no longer in a chain
        ObjRefStateBinSetNull(objRefs[ref].state.bin); // we are no longer in the world
        return;
    }

    ptr = &objRefs[objHashTable[hash_entry].ref].next; // next field of head of ref chain
#else
    ptr = &ObjRefHead(objRefs[ref].state.bin);
#endif

    while (*ptr != ref)
        ptr = &(objRefs[*ptr].next);

    // ptr is now the address of an ObjRefID equal to ref.
    // It could be the address of the obj field of a MapPoint
    //   or the address of the next field of an ObjRef
    // Got it?

    *ptr = objRefs[ref].next;                      // we have spliced ref out
    objRefs[ref].next = OBJ_REF_NULL;              // we are no longer in a chain
    ObjRefStateBinSetNull(objRefs[ref].state.bin); // we are no longer in the world
}

//
// Makes ref be a reference to obj
// Returns whether we could do it
//
static uchar ObjLinkMake(ObjRefID ref, ObjID obj) {
    if (objs[obj].ref == OBJ_REF_NULL) {
        // we are the first ObjRef to refer to obj
        objs[obj].ref = ref;
        objRefs[ref].nextref = ref;
    } else {
        // there are other references already
        objRefs[ref].nextref = objRefs[objs[obj].ref].nextref;
        objRefs[objs[obj].ref].nextref = ref;
    }

    // add the reference
    objRefs[ref].obj = obj;
    return true;
}

//
// Deletes the references of ref to its obj
// If this was the last reference to that obj,
//   returns its ID, otherwise returns OBJ_NULL
//
static ObjID ObjRefLinkDel(ObjRefID ref) {
    ObjID obj = objRefs[ref].obj;

    if (objRefs[ref].nextref == ref) // last one
    {
        objs[obj].ref = OBJ_REF_NULL;
        return obj;
    } else {
        // run around the circular list until we reach ourselves and splice ourselves out
        ObjRefID curref = ref;
        while (objRefs[curref].nextref != ref)
            curref = objRefs[curref].nextref;
        objRefs[curref].nextref = objRefs[ref].nextref;
        objs[obj].ref = curref; // easier than checking objs[obj].ref
    }

    // delete the reference
    objRefs[ref].obj = OBJ_NULL;
    return OBJ_NULL;
}

//
// Takes an ObjRef and adds it to the real world at the given place.
// This ObjRef must already refer to a valid Obj.
// Returns whether everything worked okay.
//
static uchar ObjRefAdd(ObjRefID ref, ObjRefState refstate) {
    ObjRefID *refhead;
#ifdef HASH_OBJECTS
    ObjHashElemID hash_entry;

    if ((hash_entry = ObjGetHashElem(refstate.bin, true)) == 0) {
        char str[80];
        Warning(("Could not create hash entry at %s in ObjRefAdd\n", str));
        return false;
    }
    refhead = &objHashTable[hash_entry].ref;
#else
    refhead = &ObjRefHead(refstate.bin);
#endif

    objRefs[ref].next = *refhead;
    *refhead = ref;

    ObjRefStateBinCopy(refstate.bin, objRefs[ref].state.bin);
#ifndef NO_OBJ_REF_STATE_INFO
    ObjRefStateInfoCopy(refstate.info, objRefs[ref].state.info);
#endif

    return true;
}

//
// Deletes all references to a given object.
// Returns whether the object really exists or not.
//
// NOTE: This is currently much slower than it could be because
//   we delete references one at a time and clean up each time.
//
static uchar ObjDelRefs(ObjID obj) {
    ObjRefID ref;
    ObjRefID nextref = OBJ_REF_NULL;

    ref = objs[obj].ref;
    if (ref == OBJ_REF_NULL)
        return true;

    while (true) {
        nextref = objRefs[ref].nextref;
        ObjRefRem(ref);
        ObjRefFree(ref, true);
        if (ref == nextref)
            break;
        ref = nextref;
    }

    objs[obj].ref = OBJ_REF_NULL;
    return true;
}

//
// Frees	up the given object and its class-specific data.
// Returns false (and refuses to free the object)
//   if there are ObjRefs referring to the object.
//
static uchar ObjAndSpecFree(ObjID obj) {
    ObjClass obclass;
    ObjSpecID specID;

    // The only place that things should be able to go wrong is in ObjFree, so we do that first

    obclass = objs[obj].obclass;
    specID = objs[obj].specID;

    ObjFree(obj);
    ObjSpecFree(obclass, specID);

    return true;
}
