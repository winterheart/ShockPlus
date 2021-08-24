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
#ifndef __OBJCRIT_H
#define __OBJCRIT_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/objcrit.h $
 * $Revision: 1.41 $
 * $Author: minman $
 * $Date: 1994/08/02 22:09:15 $
 *
 *
 */

// Includes
#include "objsim.h"
#include "objclass.h"

#pragma pack(push, 2)

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
    fix des_heading;
    fix des_speed;
    fix urgency;
    int16_t wait_frames;
    ushort flags;
    uint32_t attack_count; // can attack when game time reaches this
    uint8_t ai_mode;
    uint8_t mood;
    uint8_t orders;
    uint8_t current_posture;
    int8_t x1;
    int8_t y1;
    int8_t dest_x; // Current destination coordinates
    int8_t dest_y;
    int8_t pf_x; // where we are currently pathfinding to (what our current step is)
    int8_t pf_y;
    int8_t path_id;     // what pathfinding track we are on
    int8_t path_tries;  // how many frames have we been trying to get to the next step on our pathfinding?
    ObjID loot1, loot2; // Some loot to get when we destroy critter
    // Note: Num frames is gotten from object properties
    fix sidestep;
} ObjCritter;

// --------------------------
// Class typedefs

#define NUM_CRITTER_POSTURES 8

typedef struct CritterAttack {
    int32_t damage_type;
    int16_t damage_modifier;
    uint8_t offense_value;
    uint8_t penetration;
    uint8_t attack_mass;
    int16_t attack_velocity;
    uint8_t accuracy;
    uint8_t att_range;
    int32_t speed;     // Wait this long between attacks .. in game_time units
    int32_t slow_proj; // what, if any, slow projectile we fire
} CritterAttack;

#define NUM_ALTERNATE_ATTACKS 2
#define MAX_CRITTER_VIEWS 8

typedef struct CritterProp {
    uint8_t intelligence;
    CritterAttack attacks[NUM_ALTERNATE_ATTACKS];
    uint8_t perception; // each ai interval that player is seeable, this is percent of detection
    uint8_t defense;
    uint8_t proj_offset; // slow projectile offset (y)
    // int speed;
    int32_t flags;                        // flying??, shield??, fixed point?, does it move??
    uint8_t mirror;                       // should it's views be mirrored?
    uint8_t frames[NUM_CRITTER_POSTURES]; // number of animation frames.
    uint8_t anim_speed;
    uint8_t attack_sound;  // play this when attacking.  -1 for no sound.
    uint8_t near_sound;    // play when creature is nearby
    uint8_t hurt_sound;    // play when damaged a large percentage
    uint8_t death_sound;   // play when dying
    uint8_t notice_sound;  // play when it notices the player
    int32_t corpse;        // object triple of thing to put here when we die.
    uint8_t views;         // number of views for multi-view postures
    uint8_t alt_perc;      // percentage of using alternate attack
    uint8_t disrupt_perc;  // chance of being disrupted if hit while attacking
    uint8_t treasure_type; // what kind of loot this critter carries.
    uint8_t hit_effect;    // what kind of class of hit effects should we do
    uint8_t fire_frame;    // what frame do we fire on
} CritterProp;

// ------------------
// Subclass typedefs

typedef struct RobotCritterProp {
    uint8_t backup_weapon;
    uint8_t metal_thickness;
} RobotCritterProp;

typedef struct CyborgCritterProp {
    int16_t shield_energy;
} CyborgCritterProp;

#define NUM_VCOLORS 3
typedef struct CyberCritterProp {
    uint8_t vcolors[NUM_VCOLORS];
    uint8_t alt_vcolors[NUM_VCOLORS];
} CyberCritterProp;

// Quantity defines - subclasses

#define NUM_MUTANT_CRITTER 9
#define NUM_ROBOT_CRITTER 12
#define NUM_CYBORG_CRITTER 7
#define NUM_CYBER_CRITTER 7
// Note that for our purposes, ROBOBABE = PLOT which might in itself tell you something significant about our game
#define NUM_ROBOBABE_CRITTER 2

#define NUM_CRITTER \
    (NUM_MUTANT_CRITTER + NUM_ROBOT_CRITTER + NUM_CYBORG_CRITTER + NUM_CYBER_CRITTER + NUM_ROBOBABE_CRITTER)

// Enumeration of subclasses
//

// Critter
#define CRITTER_SUBCLASS_MUTANT 0
#define CRITTER_SUBCLASS_ROBOT 1
#define CRITTER_SUBCLASS_CYBORG 2
#define CRITTER_SUBCLASS_CYBER 3
#define CRITTER_SUBCLASS_ROBOBABE 4

// Lots of posture stuff...

// view = 0-7 side views at angles
// view = 8 top
// view = 9 bottom

#define FRONT_VIEW 6

#define STANDING_CRITTER_POSTURE 0
#define MOVING_CRITTER_POSTURE 1
#define ATTACKING_CRITTER_POSTURE 2
#define ATTACK_REST_CRITTER_POSTURE 3
#define KNOCKBACK_CRITTER_POSTURE 4
#define DEATH_CRITTER_POSTURE 5
#define DISRUPT_CRITTER_POSTURE 6
#define ATTACKING2_CRITTER_POSTURE 7

#define FIRST_FRONT_POSTURE ATTACKING_CRITTER_POSTURE
#define DEFAULT_CRITTER_POSTURE STANDING_CRITTER_POSTURE

// single-view postures
#define CRITTER_ATTACK_BASE RES_bmCritterAttack_0
#define CRITTER_ATTACK2_BASE RES_bmCritterAttack2_0
#define CRITTER_ATTACK_REST_BASE RES_bmCritterAttackRest_0
#define CRITTER_DEATH_BASE RES_bmCritterDeath_0
#define CRITTER_DISRUPT_BASE RES_bmCritterDisrupt_0
#define CRITTER_KNOCKBACK_BASE RES_bmCritterKnockback_0

// multi-view postures
#define CRITTER_MOVE_BASE RES_bmCritterMovement_0
#define CRITTER_STAND_BASE RES_bmCritterStanding_0

// Properties of subclasses
//

extern CritterProp CritterProps[NUM_CRITTER];
extern RobotCritterProp RobotCritterProps[NUM_ROBOT_CRITTER];
extern CyborgCritterProp CyborgCritterProps[NUM_CYBORG_CRITTER];
extern CyberCritterProp CyberCritterProps[NUM_CYBER_CRITTER];

extern ObjCritter objCritters[NUM_OBJECTS_CRITTER];
extern ObjCritter default_critter;

#define get_crit_posture(osid) (objCritters[osid].current_posture & 0xFu)
#define set_crit_posture(osid, newpos) \
    objCritters[osid].current_posture = (objCritters[osid].current_posture & 0xF0u) + newpos

#define get_crit_view(oisd) (objCritters[osid].current_posture >> 8)
#define set_crit_view(osid, newview) \
    objCritters[osid].current_posture = (newview << 8) + (objCritters[osid].current_posture & 0xF)

#pragma pack(pop)

#endif // __OBJCRIT_H
