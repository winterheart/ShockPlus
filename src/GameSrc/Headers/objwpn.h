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
#ifndef __OBJWPN_H
#define __OBJWPN_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/objwpn.h $
 * $Revision: 1.33 $
 * $Author: minman $
 * $Date: 1994/07/26 00:50:45 $
 *
 *
 */

// Includes
#include "objclass.h"

// Instance Data
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
    uint8_t ammo_type;
    uint8_t ammo_count;
} ObjGun;

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
} ObjAmmo;

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
    ObjID owner;
    int32_t bullet_triple;
    int32_t duration;
    //   char     power;
    ObjLoc p1, p2, p3;
} ObjPhysics;

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
    uint8_t unique_id;
    uint8_t walls_hit;
    int16_t flags;
    int16_t timestamp;
} ObjGrenade;

// Class typedefs

typedef struct _GunProp {
    uint8_t fire_rate;
    uint8_t useable_ammo_type;
} GunProp;

typedef struct _AmmoProp {
    int16_t damage_modifier;
    uint8_t offense_value;
    int32_t damage_type;
    uint8_t penetration;
    uint8_t cartridge_size;
    uint8_t bullet_mass;
    uint16_t bullet_speed;
    uint8_t range;
    uint8_t recoil_force;
} AmmoProp;

typedef struct _PhysicsProp {
    uint8_t flags;
} PhysicsProp;

typedef struct _GrenadeProp {
    int16_t damage_modifier;
    uint8_t offense_value;
    int32_t damage_type;
    uint8_t penetration;
    uint8_t touchiness;
    uint8_t radius;
    uint8_t radius_change;
    uint8_t damage_change;
    uint8_t attack_mass;
    int16_t flags; // Does it spew shrapnel? Can timer be set? Can it stick to wall?
} GrenadeProp;

// Subclass typedefs

typedef struct _SpecialGunProp {
    int16_t damage_modifier;
    uint8_t offense_value;
    int32_t damage_type;
    uint8_t penetration;
    uint8_t speed;
    int32_t proj_triple;
    uint8_t attack_mass;
    int16_t attack_speed;
} SpecialGunProp;

typedef struct _HandtohandGunProp {
    int16_t damage_modifier;
    uint8_t offense_value;
    int32_t damage_type;
    uint8_t penetration;
    uint8_t energy_use;
    uint8_t attack_mass;
    uint8_t attack_range;
    int16_t attack_speed;
} HandtohandGunProp;

typedef struct _BeamGunProp {
    int16_t damage_modifier;
    uint8_t offense_value;
    int32_t damage_type;
    uint8_t penetration;
    uint8_t max_charge;
    uint8_t attack_mass;
    uint8_t attack_range;
    int16_t attack_speed;
} BeamGunProp;

typedef struct _BeamprojGunProp {
    int16_t damage_modifier;
    uint8_t offense_value;
    int32_t damage_type;
    uint8_t penetration;
    uint8_t max_charge;
    uint8_t attack_mass;
    int16_t attack_speed;
    uint8_t speed;
    int32_t proj_triple;
    uint8_t flags;
} BeamprojGunProp;

typedef struct _TracerPhysicsProp {
    int16_t xcoords[4];
    int16_t ycoords[4];
    uint8_t zcoords[4];
} TracerPhysicsProp;

#define NUM_SLOW_VCOLORS 6
typedef struct _SlowPhysicsProp {
    uint8_t vcolors[NUM_SLOW_VCOLORS];
} SlowPhysicsProp;

typedef struct _TimedGrenadeProp {
    uint8_t min_time_set;
    uint8_t max_time_set;
    uint8_t timing_deviation;
} TimedGrenadeProp;

// Gun
#define NUM_PISTOL_GUN     5
#define NUM_AUTO_GUN       2
#define NUM_SPECIAL_GUN    2
#define NUM_HANDTOHAND_GUN 2
#define NUM_BEAM_GUN       3
#define NUM_BEAMPROJ_GUN   2

// Ammo
#define NUM_PISTOL_AMMO    2
#define NUM_NEEDLE_AMMO    2
#define NUM_MAGNUM_AMMO    3
#define NUM_RIFLE_AMMO     2
#define NUM_FLECHETTE_AMMO 2
#define NUM_AUTO_AMMO      2
#define NUM_PROJ_AMMO      2

// Physics
#define NUM_TRACER_PHYSICS 6
#define NUM_SLOW_PHYSICS  16
#define NUM_CAMERA_PHYSICS 2

// Grenade
#define NUM_DIRECT_GRENADE 5
#define NUM_TIMED_GRENADE  3

#define NUM_GUN (NUM_PISTOL_GUN + NUM_AUTO_GUN + NUM_SPECIAL_GUN + NUM_HANDTOHAND_GUN + NUM_BEAM_GUN + NUM_BEAMPROJ_GUN)

#define NUM_AMMO                                                                                                 \
    (NUM_PISTOL_AMMO + NUM_NEEDLE_AMMO + NUM_MAGNUM_AMMO + NUM_RIFLE_AMMO + NUM_FLECHETTE_AMMO + NUM_AUTO_AMMO + \
     NUM_PROJ_AMMO)

#define NUM_PHYSICS (NUM_TRACER_PHYSICS + NUM_SLOW_PHYSICS + NUM_CAMERA_PHYSICS)

#define NUM_GRENADE (NUM_DIRECT_GRENADE + NUM_TIMED_GRENADE)

// Gun
#define GUN_SUBCLASS_PISTOL     0
#define GUN_SUBCLASS_AUTO       1
#define GUN_SUBCLASS_SPECIAL    2
#define GUN_SUBCLASS_HANDTOHAND 3
#define GUN_SUBCLASS_BEAM       4
#define GUN_SUBCLASS_BEAMPROJ   5

// Ammo
#define AMMO_SUBCLASS_PISTOL    0
#define AMMO_SUBCLASS_NEEDLE    1
#define AMMO_SUBCLASS_MAGNUM    2
#define AMMO_SUBCLASS_RIFLE     3
#define AMMO_SUBCLASS_FLECHETTE 4
#define AMMO_SUBCLASS_AUTO      5
#define AMMO_SUBCLASS_PROJ      6

// Physics
#define PHYSICS_SUBCLASS_TRACER 0
#define PHYSICS_SUBCLASS_SLOW   1
#define PHYSICS_SUBCLASS_CAMERA 2

// Grenade
#define GRENADE_SUBCLASS_DIRECT 0
#define GRENADE_SUBCLASS_TIMED  1

extern GunProp GunProps[NUM_GUN];
extern SpecialGunProp SpecialGunProps[NUM_SPECIAL_GUN];
extern HandtohandGunProp HandtohandGunProps[NUM_HANDTOHAND_GUN];
extern BeamGunProp BeamGunProps[NUM_BEAM_GUN];
extern BeamprojGunProp BeamprojGunProps[NUM_BEAMPROJ_GUN];
extern AmmoProp AmmoProps[NUM_AMMO];
extern PhysicsProp PhysicsProps[NUM_PHYSICS];
extern TracerPhysicsProp TracerPhysicsProps[NUM_TRACER_PHYSICS];
extern SlowPhysicsProp SlowPhysicsProps[NUM_SLOW_PHYSICS];
extern GrenadeProp GrenadeProps[NUM_GRENADE];
extern TimedGrenadeProp TimedGrenadeProps[NUM_TIMED_GRENADE];

extern ObjGun objGuns[NUM_OBJECTS_GUN];
extern ObjAmmo objAmmos[NUM_OBJECTS_AMMO];
extern ObjPhysics objPhysicss[NUM_OBJECTS_PHYSICS];
extern ObjGrenade objGrenades[NUM_OBJECTS_GRENADE];
extern ObjGun default_gun;
extern ObjAmmo default_ammo;
extern ObjPhysics default_physics;
extern ObjGrenade default_grenade;

#endif // __OBJWPN_H
