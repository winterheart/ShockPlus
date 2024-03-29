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
 * $Source: r:/prj/cit/src/RCS/saveload.c $
 * $Revision: 1.145 $
 * $Author: xemu $
 * $Date: 1994/11/21 21:07:36 $
 */
#include <cassert>
#include <cstring>
#include <filesystem>

#include "Engine/Options.h"
#include "MacTune.h"

#include "saveload.h"
#include "archiveformat.h"
#include "criterr.h"
#include "cyber.h"
#include "cybmem.h"
#include "dynmem.h"
#include "effect.h"
#include "frflags.h"
#include "frprotox.h"
#include "gametime.h"
#include "gamewrap.h"
#include "hkeyfunc.h"
#include "input.h"
#include "lvldata.h"
#include "objapp.h"
#include "objects.h"
#include "objload.h"
#include "objprop.h"
#include "objsim.h"
#include "objwpn.h"
#include "objwarez.h"
#include "objstuff.h"
#include "objgame.h"
#include "objcrit.h"
#include "objver.h"
#include "objuse.h"
#include "otrip.h"
#include "map.h"
#include "mfdext.h"
#include "musicai.h"
#include "pathfind.h"
#include "physics.h"
#include "player.h"
#include "render.h"
#include "rendtool.h"
#include "schedule.h"
#include "sfxlist.h"
#include "shodan.h"
#include "statics.h"
#include "textmaps.h"
#include "tools.h"
#include "trigger.h"
#include "verify.h"


// INTERNAL PROTOTYPES
// -----------------
void load_level_data();
void store_objects(char **buf, ObjID *obj_array, char obj_count);
void restore_objects(char *buf, ObjID *obj_array, char obj_count);
errtype write_id(Id id_num, short index, uint32_t version, void *ptr, long sz, int fd, short flags);

const ResourceFormat *format_from_idx_version(int index, uint32_t version) {
    // I believe these are the only formats extant.
    const ResourceFormat *table = (version > MAP_VERSION_NUMBER) ?
        LevelVersion12Format : LevelVersion11Format;
    assert(index <= MAX_LEVEL_INDEX);
    return &table[index];
}    

// Extract a level resource. Uses the version to look up the appropriate formats
// table and the index to figure out the actual format.
void extract_level_resource(Id id_num, int index, uint32_t version, void *ptr) {
    // I believe these are the only formats extant.
    ResExtract(id_num + index, format_from_idx_version(index, version), ptr);
}

#define FIRST_CSPACE_LEVEL 14

#define SAVE_AUTOMAP_STRINGS

char saveload_string[30];
uchar display_saveload_checkpoints = FALSE;
uchar saveload_static = FALSE;
uint dynmem_mask = DYNMEM_ALL;

//-------------------------------------------------------
void store_objects(char **buf, ObjID *obj_array, char obj_count) {
    char *s = (char *)malloc(obj_count * sizeof(Obj) * 3);
    int i;
    if (s == NULL)
        critical_error(CRITERR_MEM | 3);
    *buf = s;
    for (i = 0; i < obj_count; i++) {
    check_null:
        if (obj_array[i] != OBJ_NULL) {
            ObjID id = obj_array[i];
            ObjSpecHeader *sh = &objSpecHeaders[objs[id].obclass];
            // dumb hack to avoid page fault for now, are not we cool
            if (!objs[id].active) {
                obj_array[i] = OBJ_NULL;
                goto check_null;
            }
            *(Obj *)s = objs[id];
            s += sizeof(Obj);
            memcpy(s, sh->data + sh->struct_size * objs[id].specID, sh->struct_size);
            s += sh->struct_size;
            ObjDel(id);
        } else {

            ((Obj *)s)->active = FALSE;
            s += sizeof(Obj);
        }
    }
}

//-------------------------------------------------------
void restore_objects(char *buf, ObjID *obj_array, char obj_count) {
    char *s = buf;
    int i;
    for (i = 0; i < obj_count; i++) {
        Obj *next = (Obj *)s;
        if (!next->active) {
            s += sizeof(Obj);
            obj_array[i] = OBJ_NULL;
        } else {
            ObjID id = obj_create_base(MAKETRIP(next->obclass, next->subclass, next->info.type));
            ObjSpecHeader *sh = &objSpecHeaders[next->obclass];
            char *spec;
            s += sizeof(Obj);
            if (id != OBJ_NULL) {
                objs[id].info = next->info;
                spec = (sh->data + sh->struct_size * objs[id].specID);
                memcpy(spec + sizeof(ObjSpec), s + sizeof(ObjSpec), sh->struct_size - sizeof(ObjSpec));
                obj_array[i] = id;
            }
            s += sh->struct_size;
        }
    }
    free(buf);
}

//-------------------------------------------------------
#define SECRET_VOODOO_ENDGAME_CSPACE 10

uchar go_to_different_level(int targlevel) {
    State player_state;
    char *buf;
    uchar in_cyber = global_fullmap->cyber;
    uchar retval = FALSE;
    errtype rv;

    dynmem_mask = DYNMEM_ALL;
    if ((targlevel >= FIRST_CSPACE_LEVEL) || (targlevel == SECRET_VOODOO_ENDGAME_CSPACE)) {
        if (!in_cyber) {
            // indicate static
            saveload_static = TRUE;

            // force to fullscreen

            enter_cyberspace_stuff(targlevel);
            retval = TRUE;
        }
    } else if (in_cyber) {
        saveload_static = TRUE;
    }
    if (saveload_static) {
        uchar old_music;
        physics_zero_all_controls();
        fr_global_mod_flag(FR_SOLIDFR_STATIC, FR_SOLIDFR_MASK);
        render_run();
        play_digi_fx(SFX_ENTER_CSPACE, -1);
        old_music = music_on;
        if (music_on)
            stop_music();
        music_on = old_music;
        // KLC      start_asynch_digi_fx();
        dynmem_mask = DYNMEM_PARTIAL;
    }
    //   else if (music_on)                                                                     // Don't play music
    //   while
    //      MacTuneShutdown();                                                          // elevator switches levels

    // KLC - if changing levels via the elevator, we need to make sure the elevator music
    //                    continues playing.  We'll do this by queueing up 3 additional chunks of music.
    else if (music_on) {
        for (int t = 0; t < 3; t++) {
            MacTuneQueueTune(mlimbs_boredom);
            mlimbs_boredom++;
            if (mlimbs_boredom >= 8)
                mlimbs_boredom = 0;
        }
    }

    // make a note of when we left;
    level_gamedata.exit_time = player_struct.game_time;

    // Zap the player to there...
    store_objects(&buf, player_struct.inventory, NUM_GENERAL_SLOTS);
    if (!in_cyber) {
        EDMS_get_state(objs[PLAYER_OBJ].info.ph, &player_state);
        memcpy(player_struct.edms_state, &player_state, sizeof(fix) * 12);
    } else {
        early_exit_cyberspace_stuff();
    }

    std::filesystem::path current_game_filename = ShockPlus::Options::getSavesFolder() / CURRENT_GAME_FNAME;
    rv = write_level_to_disk(current_game_filename.c_str(), ResIdFromLevel(player_struct.level), TRUE);
    if (rv)
        critical_error(CRITERR_FILE | 4);

    rv = load_level_from_file(targlevel);
    // Hmm, should we criterr out on this case?
    restore_objects(buf, player_struct.inventory, NUM_GENERAL_SLOTS);
    obj_load_art(FALSE);

    // reset renderer data
    game_fr_reparam(-1, -1, -1);

    // move the level to the present
    if (!in_cyber && !global_fullmap->cyber)
        update_level_gametime();

    // exit cyberspace
    if (in_cyber) {
        exit_cyberspace_stuff();
    }

    // Now do any level-entry triggers
    do_level_entry_triggers();

    // Set the current target to null.
    player_struct.curr_target = OBJ_NULL;

    // Undo static if we did it before
    if (saveload_static) {
        saveload_static = FALSE;
        fr_global_mod_flag(0, FR_SOLIDFR_MASK);
        if (music_on)
            start_music();
        // KLC      stop_asynch_digi_fx();
        clear_digi_fx();
    }
    /*  if no music while elevator moves
       else if (music_on)
       {
                    if (MacTuneInit() == 0)
                    {
                            mlimbs_on = TRUE;
                            mlimbs_AI_init();
                            mlimbs_boredom = TickCount() % 8;
                            load_score_guts(7);
                            MacTuneStartCurrentTheme();
                    }
       }
    */
    /*KLC
       else if (music_on)
          mlimbs_return_to_synch();
    */
    dynmem_mask = DYNMEM_ALL;
    mfd_force_update();
    return (retval);
}

#define ANOTHER_DEFINE_FOR_NUM_LEVELS 16

errtype write_id(Id id_num, short index, uint32_t version, void *ptr, long sz, int fd, short flags) {
    ResMake(id_num + index, ptr, sz, RTYPE_APP, fd, flags,
	    format_from_idx_version(index, version));
    if (ResWrite(id_num + index) == -1)
        critical_error(CRITERR_FILE | 6);
    ResUnmake(id_num + index);
    return (OK);
}

#define REF_WRITE(id_num, index, x) write_id(id_num, index, map_version, &(x), sizeof(x), fd, 0)
#define REF_WRITE_LZW(id_num, index, x)	write_id(id_num, index, map_version, &(x), sizeof(x), fd, RDF_LZW)
#define REF_WRITE_RAW(id_num, index, ptr, sz) write_id(id_num, index, map_version, ptr, sz, fd, RDF_LZW)

errtype save_current_map(const char *fname, Id id_num, uchar flush_mem, uchar pack) {
    int i, goof;
    int idx = 0;
    int fd;
    int map_version = MAP_VERSION_NUMBER;
    int ovnum = OBJECT_VERSION_NUMBER;
    int mvnum = MISC_SAVELOAD_VERSION_NUMBER;
    ObjLoc plr_loc;
    uchar make_player = FALSE;
    State player_edms;
    uint32_t verify_cookie = 0;

    INFO("Save current map: %s", fname);

    begin_wait();

    // make pathfinding state stable by fulfilling PF requests
    check_requests(FALSE);

    /* KLC - not needed for game
       if (id_num - LEVEL_ID_NUM < ANOTHER_DEFINE_FOR_NUM_LEVELS)
       {  // were in the editor, so clear out game state hack stupid i suck kill me
          fr_compile_rect(global_fullmap,0,0,MAP_XSIZE,MAP_YSIZE,TRUE);
       }
    */

    // do not ecology while player is being destroyed and created.
    trigger_check = FALSE;

    // save off physics stuff
    EDMS_get_state(objs[PLAYER_OBJ].info.ph, &player_edms);
    if (PLAYER_OBJ != OBJ_NULL) {
        plr_loc = objs[PLAYER_OBJ].loc;
        obj_destroy(PLAYER_OBJ);
        make_player = TRUE;
    }

    // Read appropriate state modifiers
    //   if (flush_mem)
    //      free_dynamic_memory(DYNMEM_PARTIAL);

    // Open the file we're going to save into.
    fd = ResEditFile(fname, TRUE);
    if (fd < 0) {
        ERROR("No file!");
        end_wait();
        return ERR_FOPEN;
    }

    REF_WRITE(SAVELOAD_VERIFICATION_ID, 0, verify_cookie);

    // xx02 Map version number.
    REF_WRITE(id_num, idx++, map_version);
    // xx03 Object version number.
    REF_WRITE(id_num, idx++, ovnum);
    // xx04 Fullmap.
    REF_WRITE(id_num, idx++, *global_fullmap);
    // xx05 Tile map.
    REF_WRITE_RAW(id_num, idx++, MAP_MAP, sizeof(MapElem) * 64 * 64);

    // Here we are writing out the schedules.  It's only a teeny tiny rep exposure.
    for (i = 0; i < NUM_MAP_SCHEDULES; i++) {
        int sz = lg_min(global_fullmap->sched[i].queue.fullness + 1, global_fullmap->sched[i].queue.size);
        REF_WRITE_RAW(id_num, idx++, global_fullmap->sched[i].queue.vec, sizeof(SchedEvent) * sz);
    }
    // xx07 Textures.
    REF_WRITE(id_num, idx++, loved_textures);

    obj_zero_unused();
    // xx08 Main object list. Always EASYSAVES, so no conversion needed.
    REF_WRITE_LZW(id_num, idx++, objs);
    // xx09 Object refs.
    REF_WRITE_LZW(id_num, idx++, objRefs);
    // xx10-xx24 Object specific stuff. All EASYSAVES again.
    REF_WRITE(id_num, idx++, objGuns);
    REF_WRITE(id_num, idx++, objAmmos);
    REF_WRITE_LZW(id_num, idx++, objPhysicss);
    REF_WRITE(id_num, idx++, objGrenades);
    REF_WRITE(id_num, idx++, objDrugs);
    REF_WRITE(id_num, idx++, objHardwares);
    REF_WRITE(id_num, idx++, objSoftwares);
    REF_WRITE_LZW(id_num, idx++, objBigstuffs);
    REF_WRITE_LZW(id_num, idx++, objSmallstuffs);
    REF_WRITE_LZW(id_num, idx++, objFixtures);
    REF_WRITE_LZW(id_num, idx++, objDoors);
    REF_WRITE(id_num, idx++, objAnimatings);
    REF_WRITE_LZW(id_num, idx++, objTraps);
    REF_WRITE_LZW(id_num, idx++, objContainers);
    REF_WRITE_LZW(id_num, idx++, objCritters);

    // xx25-xx29 Default objects
    REF_WRITE(id_num, idx++, default_gun);
    REF_WRITE(id_num, idx++, default_ammo);
    REF_WRITE(id_num, idx++, default_physics);
    REF_WRITE(id_num, idx++, default_grenade);
    REF_WRITE(id_num, idx++, default_drug);
    REF_WRITE(id_num, idx++, default_hardware);
    REF_WRITE(id_num, idx++, default_software);
    REF_WRITE(id_num, idx++, default_bigstuff);
    REF_WRITE(id_num, idx++, default_smallstuff);
    REF_WRITE(id_num, idx++, default_fixture);
    REF_WRITE(id_num, idx++, default_door);
    REF_WRITE(id_num, idx++, default_animating);
    REF_WRITE(id_num, idx++, default_trap);
    REF_WRITE(id_num, idx++, default_container);
    REF_WRITE(id_num, idx++, default_critter);

    idx++; // KLC - not used   REF_WRITE(id_num,idx++,mvnum);

    //   idx++; // where flickers once lived
    idx++; // KLC - not used   REF_WRITE(id_num,idx++,filler);
    // xx42 Texture animation.
    REF_WRITE(id_num, idx++, animtextures);
    // xx43-xx44 Surveillance
    REF_WRITE(id_num, idx++, hack_cam_objs);
    REF_WRITE(id_num, idx++, hack_cam_surrogates);

    // xx45 Other level data -- at resource id right after maps
    level_gamedata.size = sizeof(level_gamedata);
    REF_WRITE(id_num, idx++, level_gamedata);
#ifdef SAVE_AUTOMAP_STRINGS
    //   REF_WRITE(id_num, idx++, amap_str_reref(0));
    // LZW later   ResMake(id_num + (idx++), &(amap_str_reref(0)), AMAP_STRING_SIZE, RTYPE_APP, fd,  RDF_LZW);
    ResMake(id_num + (idx++), (amap_str_reref(0)), AMAP_STRING_SIZE, RTYPE_APP, fd, 0, FORMAT_RAW);
    ResWrite(id_num + (idx - 1));
    ResUnmake(id_num + (idx - 1));
    goof = amap_str_deref(amap_str_next());
    REF_WRITE(id_num, idx++, goof);
#endif
    idx++; // KLC - no need to be saved.   REF_WRITE(id_num, idx++, player_edms);
    // xx49-xx50 Paths
    REF_WRITE(id_num, idx++, paths);
    REF_WRITE(id_num, idx++, used_paths);
    REF_WRITE(id_num, idx++, animlist);
    REF_WRITE(id_num, idx++, anim_counter);
    REF_WRITE(id_num, idx++, h_sems);

    /* KLC - not used
       if (pack)
       {
          int reclaim;
          reclaim = ResPack(fd);
          if (reclaim == 0)
             Warning(("%d bytes reclaimed from ResPack!\n",reclaim));
       }
    */
    verify_cookie = VERIFY_COOKIE_VALID;
    REF_WRITE(SAVELOAD_VERIFICATION_ID, 0, verify_cookie);
    ResCloseFile(fd);

    // FlushVol(nil, fSpec->vRefNum);                   // Make sure everything is saved.

    if (make_player)
        obj_create_player(&plr_loc);
    trigger_check = TRUE;
    //   if (flush_mem)
    //      load_dynamic_memory(DYNMEM_PARTIAL);
    EDMS_holistic_teleport(objs[PLAYER_OBJ].info.ph, &player_edms);

    end_wait();
    {
        // extern void spoof_mouse_event();
        // what does this do???      spoof_mouse_event();
    }

    INFO("Saved level.");

    return OK;
}

/*KLC - no map conversion needed in Mac version.

extern uchar init_done;
extern int loadcount;

#ifdef SUPPORT_9_TO_10
void convert_map_element_9_10(oMapElem *ome, MapElem *me, int x, int y)
{
   me->tiletype=ome->tiletype;
   if (ome->param && (tile_floors[ome->tiletype].flags&FRFLRFLG_USEPR))
   {  // cool, erik has params and flags which mean NOTHING... neat... now need to detect
      int tmp=(ome->flags&MAP_MIRROR_MASK)>>MAP_MIRROR_SHF;

      if ((tmp==MAP_MATCH)||(tmp==MAP_FFLAT))
      {
              if (ome->ceil_height<ome->param)
              {
                 me->ceil_height=0;
                 Warning(("Bad input Map Format at %d,%d, param %d and ceil %d, mirror
%d\n",x,y,ome->param,ome->ceil_height,tmp));
              }
              else
                           me->ceil_height=ome->ceil_height-ome->param; // since this is negative
      }
      else me->ceil_height=ome->ceil_height;

      me->flr_height=ome->flr_height;
           me->param=ome->param;
   }
   else
   {
           me->flr_height=ome->flr_height;
           me->ceil_height=ome->ceil_height;
      me->param=0;
   }
   me->templight=ome->templight;
   me->space=ome->space;
   me->flags=ome->flags;
   me->objRef=ome->objRef;
   // new stuff
   // wow... wild internal secret gnosis
   me->rinfo.sub_clip=SUBCLIP_OUT_OF_CONE;
   me->rinfo.clear=0;
   me->rinfo.rotflr=0;
   me->rinfo.rotceil=0;
   me->rinfo.flicker=0;
}
#endif

void convert_map_element_10_11(oMapElem *ome, MapElem *me, int x, int y)
{
   me_tiletype_set(me,ome_tiletype(ome));
   me_height_flr_set(me,ome_height_flr(ome));
   me_height_ceil_set(me,ome_height_ceil(ome));
   me_param_set(me,ome_param(ome));
   me_templight_flr_set(me,ome_templight_flr(ome));
   me_templight_ceil_set(me,ome_templight_ceil(ome));
   me_tmap_flr_set(me,ome_tmap_flr(ome));
   me_tmap_ceil_set(me,ome_tmap_ceil(ome));
   me_tmap_wall_set(me,ome_tmap_wall(ome));
   me_objref_set(me,ome_objref(ome));

   // flags nightmare
   me->flag1=ome->flags&0xff;    // low word is easy
   me->flag2=(ome_bits_friend(ome)|(ome_bits_deconst(ome)<<1)|
        (ome_bits_mirror(ome)<<2)|(ome_bits_peril(ome)<<4)|(ome_bits_music(ome)<<5));
   me->flag3=(ome_light_flr(ome)|(ome_bits_rend4(ome)<<4));
   me->flag4=(ome_light_ceil(ome)|(ome_bits_rend3(ome)<<4));   // seen is 0 at start

   // new stuff
   // wow... wild internal secret gnosis
   me->sub_clip=SUBCLIP_OUT_OF_CONE;
   me->clearsolid=0;
   me_rotflr_set(me,ome->rinfo.rotflr);
   me_rotceil_set(me,ome->rinfo.rotceil);
   me_hazard_bio_set(me,0);
   me_hazard_rad_set(me,0);
   me->flick_qclip=0;
}

void convert_cit_map(oFullMap *omp, FullMap **mp)
{
   int i, j, ibase;
   if ((*mp)!=NULL)
    { Free((*mp)->map); Free(*mp); }
   *mp=Malloc(sizeof(FullMap));
   memcpy(*mp,omp,sizeof(oFullMap));
   (*mp)->map=Malloc(sizeof(MapElem)<<(omp->x_shft+omp->y_shft));
   for (ibase=0,i=0; i<(1<<omp->y_shft); i++, ibase+=(1<<(omp->x_shft)))
      for (j=0; j<(1<<omp->x_shft); j++)
         convert_map_element_10_11(omp->map+ibase+j,(*mp)->map+ibase+j,j,i);
}

#ifdef COMPRESS_OBJSPECS
extern uchar HeaderObjSpecFree (ObjClass obclass, ObjSpecID id, ObjSpecHeader *head);
extern ObjSpecID HeaderObjSpecGrab (ObjClass obclass, ObjSpecHeader *head);
extern uchar HeaderObjSpecCopy (ObjClass cls, ObjSpecID old, ObjSpecID new, ObjSpecHeader *head);
extern const ObjSpecHeader old_objSpecHeaders[NUM_CLASSES];

errtype fix_free_chain(char cl, short limit)
{
   ObjSpec *osp, *next_item, *next_next_item;
   short ss;
   char *data;
   uchar cont = TRUE;

   ss = old_objSpecHeaders[cl].struct_size;
   data = old_objSpecHeaders[cl].data;
   osp = (ObjSpec *)data;  // Get the first elem

   // iterate through free chain, leapfrogging anything which is too high.
   while (cont)
   {
      if (osp->next >= limit)
      {
         next_item = (ObjSpec *)(data + (osp->next * ss));
         osp->next = next_item->next;
         if (osp->next == OBJ_SPEC_NULL)
            cont = FALSE;
         else
         {
            next_next_item = (ObjSpec *)(data + (osp->next * ss));
            next_next_item->prev = next_item->prev;
         }
      }
      else if (osp->next == OBJ_SPEC_NULL)
         cont = FALSE;
      else
         osp = (ObjSpec *)(data + (osp->next * ss));
   }
   return(OK);
}

uchar one_compression_pass(char cl,short start)
{
   ObjSpec *osp;
   short ss;
   char *data;
   ObjID fugitive = OBJ_NULL;
   ObjSpec *p1;
   ObjSpecID new_objspec, old_specid;
   uchar cont = TRUE;

   ss = old_objSpecHeaders[cl].struct_size;
   data = old_objSpecHeaders[cl].data;
   osp = (ObjSpec *)data;  // Get the first elem
   osp = (ObjSpec *)(data + (osp->id * ss)); // Extract secret gnosis
   while(cont)
   {
      if (objs[osp->id].specID >= start)
      {
         fugitive = osp->id;
         cont = FALSE;
      }
      else if (osp->next == OBJ_SPEC_NULL)
      {
         cont = FALSE;
      }
      else
         osp = (ObjSpec *)(data + (osp->next * ss));
   }
   if (fugitive == OBJ_NULL)
      return(FALSE);

   // Now that we have a guy who ought to be moved, lets move him
   // Really, I guess we have no particular reason to believe that our new location
   // is towards the low end of stuff, but hey, it usually is....
   new_objspec = HeaderObjSpecGrab(cl,&old_objSpecHeaders[cl]);
   p1 = (ObjSpec *)(data + new_objspec * ss);
   if (new_objspec == OBJ_SPEC_NULL)
   {
      Warning(("we have a problem here folks.  Can't complete HeaderObjSpecGrab!\n"));
      critical_error(22);
   }
   // copy in the data...
   HeaderObjSpecCopy(cl,objs[osp->id].specID, new_objspec, &old_objSpecHeaders[cl]);

   p1->id = osp->id;
   old_specid = objs[osp->id].specID;
   objs[osp->id].specID = new_objspec;
   HeaderObjSpecFree(cl,old_specid, &old_objSpecHeaders[cl]);
   fix_free_chain(cl,start);
   return(TRUE);
}

errtype compress_old_class(char cl)
{
   uchar cont = TRUE;
   fix_free_chain(cl, objSpecHeaders[cl].size);
   while (cont)
      cont = one_compression_pass(cl,objSpecHeaders[cl].size);
   return(OK);
}
#endif

errtype expand_old_class(char cl, short new_start)
{
   ObjSpec *osp, *next_item;
   ObjSpecID osid;
   short ss;
   char *data;
   uchar cont = TRUE;

   ss = objSpecHeaders[cl].struct_size;
   data = objSpecHeaders[cl].data;
   osp = (ObjSpec *)data;  // Get the first elem
   osid = 0;

   // iterate through free chain until we find the old end, then extend it a bit further
   while (cont)
   {
      if (osp->next == OBJ_SPEC_NULL)
      {
         osp->next = new_start;
         Warning(("setting osid %d to %d\n",osid,new_start));
         next_item = (ObjSpec *)(data + osp->next * ss);
         next_item->prev = osid;
         cont = FALSE;
      }
      else
      {
         osid = osp->next;
         osp = (ObjSpec *)(data + (osp->next * ss));
      }
   }
   return(OK);
}
*/

void load_level_data() {
    // KLC-removed from here    obj_load_art(FALSE);
    load_small_texturemaps();
}

void SwapLongBytes(void *pval4);
void SwapShortBytes(void *pval2);
#define MAKE4(c0, c1, c2, c3) ((((ulong)c0) << 24) | (((ulong)c1) << 16) | (((ulong)c2) << 8) | ((ulong)c3))

//      ---------------------------------------------------------
// Â¥ Put this in some more appropriate, global place.
void SwapLongBytes(void *pval4) {
    long *temp = (long *)pval4;
    *temp = MAKE4(*temp & 0xFF, (*temp >> 8) & 0xFF, (*temp >> 16) & 0xFF, *temp >> 24);
}

void SwapShortBytes(void *pval2) {
    short *temp = (short *)pval2;
    *temp = ((*temp & 0xFF) << 8) | ((*temp >> 8) & 0xFF);
}

//---------------------------------------------------------------------------------
//  Loads in the map for a level, and all the other related resources (2+ MB worth).
//---------------------------------------------------------------------------------
// errtype load_current_map(char* fn, Id id_num, Datapath* dpath)
errtype load_current_map(Id id_num) {

    int i, idx = 0, fd;
    uint32_t map_version;
    uint32_t object_version;
    LGRect bounds;
    errtype retval = OK;
    bool make_player = FALSE;
    ObjLoc plr_loc;
    char *schedvec; // KLC - don't need an array.  Only one in map.
    // State          player_edms;
    curAMap saveAMaps[NUM_O_AMAP];
    uchar savedMaps;
    bool do_anims = FALSE;

    //   _MARK_("load_current_map:Start");

    INFO("Loading map %x", id_num);

    begin_wait();
    free_dynamic_memory(dynmem_mask);
    trigger_check = FALSE;
    if (PLAYER_OBJ != OBJ_NULL) {
        plr_loc = objs[PLAYER_OBJ].loc;
        obj_destroy(PLAYER_OBJ);
        make_player = TRUE;
    }
    if (input_cursor_mode == INPUT_OBJECT_CURSOR) {
        pop_cursor_object();
    }

    // Open the saved-game (or archive) file.
    std::filesystem::path current_game_filename = ShockPlus::Options::getSavesFolder() / CURRENT_GAME_FNAME;
    fd = ResOpenFile(current_game_filename.c_str());
    if (fd < 0) {
        // Warning(("Could not load map file %s (%s) , rv = %d!\n",dpath_fn,fn,retval));
        ERROR("Could not load map file %d", retval);
        if (make_player)
            obj_create_player(&plr_loc);
        trigger_check = TRUE;
        load_dynamic_memory(dynmem_mask);
        end_wait();

        return ERR_FOPEN;
    }

    if (ResInUse(SAVELOAD_VERIFICATION_ID)) {
        uint32_t verify_cookie;
        ResExtract(SAVELOAD_VERIFICATION_ID, FORMAT_U32, &verify_cookie);
        if ((verify_cookie != VERIFY_COOKIE_VALID) && (verify_cookie != OLD_VERIFY_COOKIE_VALID))
            critical_error(CRITERR_FILE | 5);
    }

    // Resource xx02: map version.
    ResExtract(id_num + idx++, FORMAT_U32, &map_version);

    // Check the version number of the map for this level.
    if (map_version < MAP_VERSION_NUMBER) {
        INFO("OLD MAP FORMAT!");
    }

#define REF_READ(id,index,x) extract_level_resource(id, index, map_version, &(x))
    
    //  object version number!
    REF_READ(id_num, idx++, object_version);
    // SwapLongBytes(&version);                        // Mac

    // Clear out old physics data and object data
    ObjsInit();
    physics_init();

    // Read in the global fullmap (without disrupting schedule vec ptr)
    schedvec = global_fullmap->sched[0].queue.vec; // KLC - Only one schedule, so just save it.
    // Preserve the old schedule size in case the one being loaded is different
    int schedsize = global_fullmap->sched[0].queue.size;

    // convert_from is the version we are coming from.
    // for now, this is only defined for coming from version 9
    REF_READ(id_num, idx++, *global_fullmap);

    MAP_MAP = (MapElem *)static_map;
    REF_READ(id_num, idx++, *static_map);

    // Load schedules, performing some voodoo.
    global_fullmap->sched[0].queue.vec = schedvec;
    global_fullmap->sched[0].queue.comp = compare_events;

    // Might have to allocate more memory for the queue
    if (global_fullmap->sched[0].queue.size > schedsize) {
	schedule_free(&global_fullmap->sched[0]);
	schedule_init(&global_fullmap->sched[0], global_fullmap->sched[0].queue.size, FALSE);
    } else {
	// Preserve the existing size.
	global_fullmap->sched[0].queue.size = schedsize;
    }

    char *dst_ptr = global_fullmap->sched[0].queue.vec;
    ResExtract(id_num + idx++, FORMAT_RAW, dst_ptr);

    // KLC��� Big hack!  Force the schedule to growable.
    global_fullmap->sched[0].queue.grow = TRUE;

    REF_READ(id_num, idx++, loved_textures);
    /*
       for (i = 0; i < NUM_LOADED_TEXTURES; i++)
       {
          SwapShortBytes(&loved_textures[i]);
       }
    */
    map_set_default(global_fullmap);

    /*���  Leave conversion from old objects out for now

       // Now set up for object conversion if necessary
       convert_from = -1;

       if (version != OBJECT_VERSION_NUMBER)
       {
          retval = ERR_NOEFFECT;
          Warning(("Old Object Version Number (%d)!!  Current V. Num = %d\n",version,OBJECT_VERSION_NUMBER));
          if (version >= 17)
          {
             Warning(("Auto-converting objects to v. %d from %d\n", OBJECT_VERSION_NUMBER,version));
             convert_from = version;
          }
          else
          {
             for (x=0; x<global_fullmap->x_size; x++)
             {
                for (y=0; y<global_fullmap->y_size; y++)
                {
                   MAP_GET_XY(x,y)->objRef = 0;
                }
             }
             goto obj_out;
          }
       }
    #ifdef SUPPORT_VERSION_26_OBJS
       if ((convert_from < 27) && (convert_from != -1))
       {
          extern old_Obj old_objs[NUM_OBJECTS];
          REF_READ(id_num,idx++,old_objs);
          for (x=0; x < NUM_OBJECTS; x++)
          {
             objs[x].active = old_objs[x].active;
             objs[x].obclass = old_objs[x].obclass;
             objs[x].subclass = old_objs[x].subclass;
             objs[x].specID = old_objs[x].specID;
             objs[x].ref = old_objs[x].ref;
             objs[x].next = old_objs[x].next;
             objs[x].prev = old_objs[x].prev;
             objs[x].loc = old_objs[x].loc;
             objs[x].info.ph = (char)(old_objs[x].info.ph);
             objs[x].info.type = old_objs[x].info.type;
             objs[x].info.current_hp = old_objs[x].info.current_hp;
             objs[x].info.make_info = old_objs[x].info.make_info;
             objs[x].info.current_frame = old_objs[x].info.current_frame;
             objs[x].info.time_remainder = old_objs[x].info.time_remainder;
             objs[x].info.inst_flags = old_objs[x].info.inst_flags;
          }
       }
       else
    #endif
    */

    // Read in object information.
    REF_READ(id_num, idx++, objs);

    // Read in and convert the object refs.
    REF_READ(id_num, idx++, objRefs);
    /* for (i=0; i < NUM_REF_OBJECTS; i++)
       {
          SwapShortBytes(&objRefs[i].state.bin.sq.x);
          SwapShortBytes(&objRefs[i].state.bin.sq.y);
          SwapShortBytes(&objRefs[i].obj);
          SwapShortBytes(&objRefs[i].next);
          SwapShortBytes(&objRefs[i].nextref);
       } */

    // Read in and convert the gun objects.
    REF_READ(id_num, idx++, objGuns);
    /* for (i=0; i < NUM_OBJECTS_GUN; i++)
       {
          SwapShortBytes(&objGuns[i].id);
          SwapShortBytes(&objGuns[i].next);
          SwapShortBytes(&objGuns[i].prev);
       }*/

    // Read in and convert the ammo objects.
    REF_READ(id_num, idx++, objAmmos);
    /* for (i=0; i < NUM_OBJECTS_AMMO; i++)
       {
          SwapShortBytes(&objAmmos[i].id);
          SwapShortBytes(&objAmmos[i].next);
          SwapShortBytes(&objAmmos[i].prev);
       }*/

    // Read in and convert the physics objects.
    REF_READ(id_num, idx++, objPhysicss);
    /* for (i=0; i < NUM_OBJECTS_PHYSICS; i++)
       {
          SwapShortBytes(&objPhysicss[i].id);
          SwapShortBytes(&objPhysicss[i].next);
          SwapShortBytes(&objPhysicss[i].prev);
          SwapShortBytes(&objPhysicss[i].owner);
          SwapLongBytes(&objPhysicss[i].bullet_triple);
          SwapLongBytes(&objPhysicss[i].duration);
          SwapShortBytes(&objPhysicss[i].p1.x);
          SwapShortBytes(&objPhysicss[i].p1.y);
          SwapShortBytes(&objPhysicss[i].p2.x);
          SwapShortBytes(&objPhysicss[i].p2.y);
          SwapShortBytes(&objPhysicss[i].p3.x);
          SwapShortBytes(&objPhysicss[i].p3.y);
       }*/

    // Read in and convert the grenades.
    REF_READ(id_num, idx++, objGrenades);
    /* for (i=0; i < NUM_OBJECTS_GRENADE; i++)
       {
          SwapShortBytes(&objGrenades[i].id);
          SwapShortBytes(&objGrenades[i].next);
          SwapShortBytes(&objGrenades[i].prev);
          SwapShortBytes(&objGrenades[i].flags);
          SwapShortBytes(&objGrenades[i].timestamp);
       }*/

    // Read in and convert the drugs.
    REF_READ(id_num, idx++, objDrugs);
    /* for (i=0; i < NUM_OBJECTS_DRUG; i++)
       {
          SwapShortBytes(&objDrugs[i].id);
          SwapShortBytes(&objDrugs[i].next);
          SwapShortBytes(&objDrugs[i].prev);
       }*/

    // Read in and convert the hardwares. 
    REF_READ(id_num, idx++, objHardwares);

    // Read in and convert the softwares.
    REF_READ(id_num, idx++, objSoftwares);

    // Read in and convert the big stuff.
    REF_READ(id_num, idx++, objBigstuffs);
    /* for (i=0; i < NUM_OBJECTS_BIGSTUFF; i++)
       {
          SwapShortBytes(&objBigstuffs[i].id);
          SwapShortBytes(&objBigstuffs[i].next);
          SwapShortBytes(&objBigstuffs[i].prev);
          SwapShortBytes(&objBigstuffs[i].cosmetic_value);
          SwapLongBytes(&objBigstuffs[i].data1);
          SwapLongBytes(&objBigstuffs[i].data2);
       }*/

    // Read in and convert the small stuff.
    REF_READ(id_num, idx++, objSmallstuffs);
    /* for (i=0; i < NUM_OBJECTS_SMALLSTUFF; i++)
       {
          SwapShortBytes(&objSmallstuffs[i].id);
          SwapShortBytes(&objSmallstuffs[i].next);
          SwapShortBytes(&objSmallstuffs[i].prev);
          SwapShortBytes(&objSmallstuffs[i].cosmetic_value);
          SwapLongBytes(&objSmallstuffs[i].data1);
          SwapLongBytes(&objSmallstuffs[i].data2);
       }*/

    // Read in and convert the fixtures.
    REF_READ(id_num, idx++, objFixtures);
    /* for (i=0; i < NUM_OBJECTS_FIXTURE; i++)
       {
          SwapShortBytes(&objFixtures[i].id);
          SwapShortBytes(&objFixtures[i].next);
          SwapShortBytes(&objFixtures[i].prev);
          SwapLongBytes(&objFixtures[i].comparator);
          SwapLongBytes(&objFixtures[i].p1);
          SwapLongBytes(&objFixtures[i].p2);
          SwapLongBytes(&objFixtures[i].p3);
          SwapLongBytes(&objFixtures[i].p4);
          SwapShortBytes(&objFixtures[i].access_level);
       }*/

    // Read in and convert the doors.
    REF_READ(id_num, idx++, objDoors);
    /* for (i=0; i < NUM_OBJECTS_DOOR; i++)
       {
          SwapShortBytes(&objDoors[i].id);
          SwapShortBytes(&objDoors[i].next);
          SwapShortBytes(&objDoors[i].prev);
          SwapShortBytes(&objDoors[i].locked);
          SwapShortBytes(&objDoors[i].other_half);
       }*/

    // Read in and convert the animating objects.
    REF_READ(id_num, idx++, objAnimatings);
    /* for (i=0; i < NUM_OBJECTS_ANIMATING; i++)
       {
          SwapShortBytes(&objAnimatings[i].id);
          SwapShortBytes(&objAnimatings[i].next);
          SwapShortBytes(&objAnimatings[i].prev);
          SwapShortBytes(&objAnimatings[i].owner);
       }*/

    // Read in and convert the traps.
    REF_READ(id_num, idx++, objTraps);
    /* for (i=0; i < NUM_OBJECTS_TRAP; i++)
       {
          SwapShortBytes(&objTraps[i].id);
          SwapShortBytes(&objTraps[i].next);
          SwapShortBytes(&objTraps[i].prev);
          SwapLongBytes(&objTraps[i].comparator);
          SwapLongBytes(&objTraps[i].p1);
          SwapLongBytes(&objTraps[i].p2);
          SwapLongBytes(&objTraps[i].p3);
          SwapLongBytes(&objTraps[i].p4);
       }  */

    // Read in and convert the containers.
    REF_READ(id_num, idx++, objContainers);

    // Read in and convert the critters.
    REF_READ(id_num, idx++, objCritters);
    /* for (i=0; i < NUM_OBJECTS_CRITTER; i++)
       {
          SwapShortBytes(&objCritters[i].id);
          SwapShortBytes(&objCritters[i].next);
          SwapShortBytes(&objCritters[i].prev);
          SwapLongBytes(&objCritters[i].des_heading);
          SwapLongBytes(&objCritters[i].des_speed);
          SwapLongBytes(&objCritters[i].urgency);
          SwapShortBytes(&objCritters[i].wait_frames);
          SwapShortBytes(&objCritters[i].flags);
          SwapLongBytes(&objCritters[i].attack_count);
          SwapShortBytes(&objCritters[i].loot1);
          SwapShortBytes(&objCritters[i].loot2);
          SwapLongBytes(&objCritters[i].sidestep);
       }  */

    //-------------------------------
    //  Read in the default objects.
    //-------------------------------

    // Convert the default gun.
    REF_READ(id_num, idx++, default_gun);
    /* SwapShortBytes(&default_gun.id);
       SwapShortBytes(&default_gun.next);
       SwapShortBytes(&default_gun.prev);*/

    // Convert the default ammo.
    REF_READ(id_num, idx++, default_ammo);
    /* SwapShortBytes(&default_ammo.id);
       SwapShortBytes(&default_ammo.next);
       SwapShortBytes(&default_ammo.prev);*/

    // Read in and convert the physics objects.
    REF_READ(id_num, idx++, default_physics);
    /* SwapShortBytes(&default_physics.id);
       SwapShortBytes(&default_physics.next);
       SwapShortBytes(&default_physics.prev);
       SwapShortBytes(&default_physics.owner);
       SwapLongBytes(&default_physics.bullet_triple);
       SwapLongBytes(&default_physics.duration);
       SwapShortBytes(&default_physics.p1.x);
       SwapShortBytes(&default_physics.p1.y);
       SwapShortBytes(&default_physics.p2.x);
       SwapShortBytes(&default_physics.p2.y);
       SwapShortBytes(&default_physics.p3.x);
       SwapShortBytes(&default_physics.p3.y);*/

    // Convert the default grenade.
    REF_READ(id_num, idx++, default_grenade);
    /* SwapShortBytes(&default_grenade.id);
       SwapShortBytes(&default_grenade.next);
       SwapShortBytes(&default_grenade.prev);
       SwapShortBytes(&default_grenade.flags);
       SwapShortBytes(&default_grenade.timestamp);*/

    // Convert the default drug.
    REF_READ(id_num, idx++, default_drug);
    /* SwapShortBytes(&default_drug.id);
       SwapShortBytes(&default_drug.next);
       SwapShortBytes(&default_drug.prev);*/

    // Convert the default hardware.
    REF_READ(id_num, idx++, default_hardware);

    // Convert the default software.
    REF_READ(id_num, idx++, default_software);

    // Convert the default big stuff.
    REF_READ(id_num, idx++, default_bigstuff);
    /* SwapShortBytes(&default_bigstuff.id);
       SwapShortBytes(&default_bigstuff.next);
       SwapShortBytes(&default_bigstuff.prev);
       SwapShortBytes(&default_bigstuff.cosmetic_value);
       SwapLongBytes(&default_bigstuff.data1);
       SwapLongBytes(&default_bigstuff.data2);*/

    // Convert the default small stuff.
    REF_READ(id_num, idx++, default_smallstuff);
    /* SwapShortBytes(&default_smallstuff.id);
       SwapShortBytes(&default_smallstuff.next);
       SwapShortBytes(&default_smallstuff.prev);
       SwapShortBytes(&default_smallstuff.cosmetic_value);
       SwapLongBytes(&default_smallstuff.data1);
       SwapLongBytes(&default_smallstuff.data2);*/

    // Convert the fixture.
    REF_READ(id_num, idx++, default_fixture);
    /* SwapShortBytes(&default_fixture.id);
       SwapShortBytes(&default_fixture.next);
       SwapShortBytes(&default_fixture.prev);
       SwapLongBytes(&default_fixture.comparator);
       SwapLongBytes(&default_fixture.p1);
       SwapLongBytes(&default_fixture.p2);
       SwapLongBytes(&default_fixture.p3);
       SwapLongBytes(&default_fixture.p4);
       SwapShortBytes(&default_fixture.access_level);*/

    // Convert the default door.
    REF_READ(id_num, idx++, default_door);
    /* SwapShortBytes(&default_door.id);
       SwapShortBytes(&default_door.next);
       SwapShortBytes(&default_door.prev);
       SwapShortBytes(&default_door.locked);
       SwapShortBytes(&default_door.other_half);*/

    // Convert the default animating object.
    REF_READ(id_num, idx++, default_animating);
    /* SwapShortBytes(&default_animating.id);
       SwapShortBytes(&default_animating.next);
       SwapShortBytes(&default_animating.prev);
       SwapShortBytes(&default_animating.owner);*/

    // Read in and convert the traps.
    REF_READ(id_num, idx++, default_trap);
    /* SwapShortBytes(&default_trap.id);
       SwapShortBytes(&default_trap.next);
       SwapShortBytes(&default_trap.prev);
       SwapLongBytes(&default_trap.comparator);
       SwapLongBytes(&default_trap.p1);
       SwapLongBytes(&default_trap.p2);
       SwapLongBytes(&default_trap.p3);
       SwapLongBytes(&default_trap.p4);*/

    // Convert the default container.
    REF_READ(id_num, idx++, default_container);

    // Convert the default critter.
    REF_READ(id_num, idx++, default_critter);
    /* SwapShortBytes(&default_critter.id);
       SwapShortBytes(&default_critter.next);
       SwapShortBytes(&default_critter.prev);
       SwapLongBytes(&default_critter.des_heading);
       SwapLongBytes(&default_critter.des_speed);
       SwapLongBytes(&default_critter.urgency);
       SwapShortBytes(&default_critter.wait_frames);
       SwapShortBytes(&default_critter.flags);
       SwapLongBytes(&default_critter.attack_count);
       SwapShortBytes(&default_critter.loot1);
       SwapShortBytes(&default_critter.loot2);
       SwapLongBytes(&default_critter.sidestep);*/

    idx++;
    /* KLC - don't need this any more.

       REF_READ(id_num, idx++, version);
       SwapLongBytes(&version);                        // Mac
       if (version != MISC_SAVELOAD_VERSION_NUMBER && version < 5)
       {
          retval = ERR_NOEFFECT;
          anim_counter = 0;
          goto obj_out;
       }
    */
    idx++; // skip over resource where flickers once lived

    // Convert the anim textures.
    REF_READ(id_num, idx++, animtextures);

    // Read in and convert the hack camera objects.
    REF_READ(id_num, idx++, hack_cam_objs);
    REF_READ(id_num, idx++, hack_cam_surrogates);
    /* for (i = 0; i < NUM_HACK_CAMERAS; i++)
       {
          SwapShortBytes(&hack_cam_objs[i]);
          SwapShortBytes(&hack_cam_surrogates[i]);
       }*/

    savedMaps = 0;
    for (i = 0; i < NUM_O_AMAP; i++) {
        if (oAMap(i)->init) {
            savedMaps |= (1 << i);
            amap_settings_copy(oAMap(i), &saveAMaps[i]);
            amap_invalidate(i);
        }
    }

    // Get other level data at next id
    REF_READ(id_num, idx++, level_gamedata);

#ifdef SAVE_AUTOMAP_STRINGS
    {
        int amap_magic_num;
        char *cp = amap_str_reref(0);
	REF_READ(id_num, idx++, *cp);
        //    REF_READ(id_num, idx++, amap_str_reref(0));     old way
        REF_READ(id_num, idx++, amap_magic_num);
        //    SwapLongBytes(&amap_magic_num);
        amap_str_startup(amap_magic_num);
    }
#endif

    idx++; // Doesn't appear that this does anything
           /*
              REF_READ(id_num, idx++, player_edms);
              SwapLongBytes(&player_edms.X);
              SwapLongBytes(&player_edms.Y);
              SwapLongBytes(&player_edms.Z);
              SwapLongBytes(&player_edms.alpha);
              SwapLongBytes(&player_edms.beta);
              SwapLongBytes(&player_edms.gamma);
              SwapLongBytes(&player_edms.X_dot);
              SwapLongBytes(&player_edms.Y_dot);
              SwapLongBytes(&player_edms.Z_dot);
              SwapLongBytes(&player_edms.alpha_dot);
              SwapLongBytes(&player_edms.beta_dot);
              SwapLongBytes(&player_edms.gamma_dot);
           */

    REF_READ(id_num, idx++, paths);
    /* for(i=0; i < MAX_PATHS; i++)
       {
          SwapShortBytes(&paths[i].source.x);
          SwapShortBytes(&paths[i].source.y);
          SwapShortBytes(&paths[i].dest.x);
          SwapShortBytes(&paths[i].dest.y);
       }*/
    REF_READ(id_num, idx++, used_paths);
    // SwapShortBytes(&used_paths);

    REF_READ(id_num, idx++, animlist);

    REF_READ(id_num, idx++, anim_counter);
    // SwapShortBytes(&anim_counter);

    REF_READ(id_num, idx++, h_sems); // Unbelievably, no conversion needed.

obj_out:
    bounds.ul.x = bounds.ul.y = 0;
    bounds.lr.x = global_fullmap->x_size;
    bounds.lr.y = global_fullmap->y_size;

    rendedit_process_tilemap(global_fullmap, &bounds, TRUE);

    for (i = 0; i < MAX_OBJ; i++)
        physics_handle_id[i] = OBJ_NULL;
    physics_handle_max = -1;

    if (anim_counter == 0)
        do_anims = TRUE;

    for (ObjID oid = (objs[OBJ_NULL]).headused; oid != OBJ_NULL; oid = objs[oid].next) {
        switch (objs[oid].obclass) {
        case CLASS_DOOR:
            set_door_data(oid);
            break;
        }

        if (do_anims && ANIM_3D(ObjProps[OPNUM(oid)].bitmap_3d)) {
            switch (TRIP2CL(ID2TRIP(oid))) {
            case CLASS_BIGSTUFF:
            case CLASS_SMALLSTUFF:
                obj_screen_animate(oid);
                break;
            default:
                add_obj_to_animlist(oid, REPEAT_3D(ObjProps[OPNUM(oid)].bitmap_3d), FALSE, FALSE, 0, 0, 0, 0);
                break;
            }
        }

        objs[oid].info.ph = -1;
        if (objs[oid].loc.x != 0xFFFF) {
            obj_move_to(oid, &objs[oid].loc, TRUE);
        }

        // sleep the object (this may become "settle" the object)
        if (objs[oid].info.ph != -1) {
            cit_sleeper_callback(objs[oid].info.ph);
            edms_delete_go();
        }
    }

    // DO NOT call this from here.  We haven't necessarily yet set
    // player_struct.level, which means the wrong shodometer quest
    // variable gets set!!!
    //
    // compute_shodometer_value(FALSE);

    if (make_player) {
        obj_create_player(&plr_loc);
        if (object_version > 9) {
            // Regenerate physics state from player_state here
        }
        //���      if (music_on && (score_playing != ELEVATOR_ZONE))
        //���        load_score_for_location(PLAYER_BIN_X,PLAYER_BIN_Y);
    }

out:
    ResCloseFile(fd);

    reset_pathfinding();
    old_bits = -1;

    trigger_check = TRUE;

    load_dynamic_memory(dynmem_mask);
    load_level_data();

    for (i = 0; i < NUM_O_AMAP; i++) {
        if (!oAMap(i)->init && (savedMaps & (1 << i))) {
            automap_init(player_struct.hardwarez[CPTRIP(NAV_HARD_TRIPLE)], i);
            amap_settings_copy(&saveAMaps[i], oAMap(i));
        }
    }
    reload_motion_cursors(global_fullmap->cyber);

    // Debug print the map
#ifdef DEBUG_MAP_PRINT
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            uchar tiletype = global_fullmap->map[x + y * 64].tiletype;
            if (tiletype == 0)
                printf("  ");
            else
                printf(" %i", tiletype);
        }
        printf("\n");
    }
#endif

    // KLC   physics_warmup();

    end_wait();
    /*���   {
          extern void spoof_mouse_event();
          spoof_mouse_event();
       }
       _MARK_("load_current_map:End");
    */

    return retval;
}
