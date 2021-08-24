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
#ifndef __OBJGAME_H
#define __OBJGAME_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/objgame.h $
 * $Revision: 1.52 $
 * $Author: minman $
 * $Date: 1994/07/30 00:19:06 $
 *
 */

// Includes
#include "objclass.h"

#pragma pack(push,2)

// Instance Typedefs
typedef struct {
    union {
        ObjID id;
        ObjSpecID headused;
    };
    union {
        ObjSpecID next;
        ObjSpecID headfree;
    };
    ObjSpecID prev;
    uint8_t trap_type;
    uint8_t destroy_count;
    uint32_t comparator;
    uint32_t p1;
    uint32_t p2;
    uint32_t p3;
    uint32_t p4;
    int16_t access_level;
} ObjFixture;

typedef struct {
    union {
        ObjID id;
        ObjSpecID headused;
    };
    union {
        ObjSpecID next;
        ObjSpecID headfree;
    };
    ObjSpecID prev;
    int16_t locked;
    uint8_t stringnum;
    uint8_t cosmetic_value;
    uint8_t access_level;
    uint8_t autoclose_time;
    ObjID other_half;
} ObjDoor;

typedef struct {
    union {
        ObjID id;
        ObjSpecID headused;
    };
    union {
        ObjSpecID next;
        ObjSpecID headfree;
    };
    ObjSpecID prev;
    uint8_t start_frame;
    uint8_t end_frame;
    ObjID owner;
} ObjAnimating;

typedef struct {
    union {
        ObjID id;
        ObjSpecID headused;
    };
    union {
        ObjSpecID next;
        ObjSpecID headfree;
    };
    ObjSpecID prev;
    uint8_t trap_type;
    uint8_t destroy_count;
    uint32_t comparator;
    uint32_t p1;
    uint32_t p2;
    uint32_t p3;
    uint32_t p4;
} ObjTrap;

typedef struct {
    union {
        ObjID id;
        ObjSpecID headused;
    };
    union {
        ObjSpecID next;
        ObjSpecID headfree;
    };
    ObjSpecID prev;
    int32_t contents1;
    int32_t contents2;
    uint8_t dim_x;
    uint8_t dim_y;
    uint8_t dim_z;
    int32_t data1;
} ObjContainer;

// Class typedefs
typedef struct FixtureProp {
    uint8_t characteristics;
} FixtureProp;

typedef struct DoorProp {
    uint8_t security_level; // i.e difficulty to unlock
} DoorProp;

#define ANIM_FLAG_NONE 0
#define ANIM_FLAG_REPEAT 1
#define ANIM_FLAG_REVERSE 2

typedef struct AnimatingProp {
    uint8_t speed;
    uint8_t flags;
} AnimatingProp;

typedef struct ContainerProp {
    ObjID contents; // obviously not the way to do it, but you get the idea
    uint8_t num_contents;
} ContainerProp;

// Subclass typedefs
typedef struct ExplosionAnimatingProp {
    uint8_t frame_explode;
} ExplosionAnimatingProp;

// Quantity defines - subclasses
// Fixture
#define NUM_CONTROL_FIXTURE    9
#define NUM_RECEPTACLE_FIXTURE 7
#define NUM_TERMINAL_FIXTURE   3
#define NUM_PANEL_FIXTURE     11
#define NUM_VENDING_FIXTURE    2
#define NUM_CYBER_FIXTURE      3

// Door
#define NUM_NORMAL_DOOR  10
#define NUM_DOORWAYS_DOOR 9
#define NUM_FORCE_DOOR    7
#define NUM_ELEVATOR_DOOR 5
#define NUM_SPECIAL_DOOR 10

// Animating
#define NUM_OBJECT_ANIMATING      9
#define NUM_TRANSITORY_ANIMATING 11
#define NUM_EXPLOSION_ANIMATING  14

// Trap
#define NUM_TRIGGER_TRAP  13
#define NUM_FEEDBACKS_TRAP 1
#define NUM_SECRET_TRAP    5

// Container
#define NUM_ACTUAL_CONTAINER        3
#define NUM_WASTE_CONTAINER         3
#define NUM_LIQUID_CONTAINER        4
#define NUM_MUTANT_CORPSE_CONTAINER 8
#define NUM_ROBOT_CORPSE_CONTAINER 13
#define NUM_CYBORG_CORPSE_CONTAINER 7
#define NUM_OTHER_CORPSE_CONTAINER  8

#define NUM_FIXTURE                                                                                                \
    (NUM_CONTROL_FIXTURE + NUM_RECEPTACLE_FIXTURE + NUM_TERMINAL_FIXTURE + NUM_PANEL_FIXTURE + NUM_CYBER_FIXTURE + \
     NUM_VENDING_FIXTURE)
#define NUM_DOOR (NUM_NORMAL_DOOR + NUM_DOORWAYS_DOOR + NUM_FORCE_DOOR + NUM_ELEVATOR_DOOR + NUM_SPECIAL_DOOR)
#define NUM_ANIMATING (NUM_OBJECT_ANIMATING + NUM_TRANSITORY_ANIMATING + NUM_EXPLOSION_ANIMATING)
#define NUM_TRAP (NUM_TRIGGER_TRAP + NUM_FEEDBACKS_TRAP + NUM_SECRET_TRAP)
#define NUM_CONTAINER                                                                                  \
    (NUM_ACTUAL_CONTAINER + NUM_WASTE_CONTAINER + NUM_LIQUID_CONTAINER + NUM_MUTANT_CORPSE_CONTAINER + \
     NUM_ROBOT_CORPSE_CONTAINER + NUM_CYBORG_CORPSE_CONTAINER + NUM_OTHER_CORPSE_CONTAINER)

// Enumeration of subclasses
// Fixture
#define FIXTURE_SUBCLASS_CONTROL    0
#define FIXTURE_SUBCLASS_RECEPTACLE 1
#define FIXTURE_SUBCLASS_TERMINAL   2
#define FIXTURE_SUBCLASS_PANEL      3
#define FIXTURE_SUBCLASS_VENDING    4
#define FIXTURE_SUBCLASS_CYBER      5

// Door
#define DOOR_SUBCLASS_NORMAL   0
#define DOOR_SUBCLASS_DOORWAYS 1
#define DOOR_SUBCLASS_FORCE    2
#define DOOR_SUBCLASS_ELEVATOR 3
#define DOOR_SUBCLASS_SPECIAL  4

// Animating
#define ANIMATING_SUBCLASS_OBJECTS    0
#define ANIMATING_SUBCLASS_TRANSITORY 1
#define ANIMATING_SUBCLASS_EXPLOSION  2

// Trap
#define TRAP_SUBCLASS_TRIGGER   0
#define TRAP_SUBCLASS_FEEDBACKS 1
#define TRAP_SUBCLASS_SECRET    2

// Container
#define CONTAINER_SUBCLASS_ACTUAL        0
#define CONTAINER_SUBCLASS_WASTE         1
#define CONTAINER_SUBCLASS_LIQUID        2
#define CONTAINER_SUBCLASS_MUTANT_CORPSE 3
#define CONTAINER_SUBCLASS_ROBOT_CORPSE  4
#define CONTAINER_SUBCLASS_CYBORG_CORPSE 5
#define CONTAINER_SUBCLASS_OTHER_CORPSE  6

extern FixtureProp FixtureProps[NUM_FIXTURE];

extern DoorProp DoorProps[NUM_DOOR];

extern AnimatingProp AnimatingProps[NUM_ANIMATING];
extern ExplosionAnimatingProp ExplosionAnimatingProps[NUM_EXPLOSION_ANIMATING];

extern ContainerProp ContainerProps[NUM_CONTAINER];

extern ObjFixture objFixtures[NUM_OBJECTS_FIXTURE];
extern ObjDoor objDoors[NUM_OBJECTS_DOOR];
extern ObjAnimating objAnimatings[NUM_OBJECTS_ANIMATING];
extern ObjTrap objTraps[NUM_OBJECTS_TRAP];
extern ObjContainer objContainers[NUM_OBJECTS_CONTAINER];
extern ObjFixture default_fixture;
extern ObjDoor default_door;
extern ObjAnimating default_animating;
extern ObjTrap default_trap;
extern ObjContainer default_container;

#pragma pack(pop)

#endif // __OBJGAME_H
