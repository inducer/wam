// ----------------------------------------------------------------------------
//  Description      : WAM game constants
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_GAME_NAMES
#define WAM_GAME_NAMES




#include "engine/gameject.hh"




// file name strings ----------------------------------------------------------
#define IMG_EXTENSION   ".png"




// gameject features ----------------------------------------------------------
#define GJFEATURE_MPACK_WAM  "pWAM"
#define GJFEATURE_GAME_PHYSICS  "Phys"
#define GJFEATURE_HERO   "Hero"
#define GJFEATURE_WEAPON  "Weap"
#define GJFEATURE_ENVIRONMENT  "EnvM"
#define GJFEATURE_SUPERVISOR  "Supv"
#define GJFEATURE_SOLID  "Soli"




// gameject feature sets ------------------------------------------------------
#define GJFEATURESET_HERO \
  GJFEATURE_MPACK_WAM GJFEATURE_GAME_PHYSICS GJFEATURE_HERO \
  GJFEATURE_GAMECOORD GJFEATURE_DISPLAYED GJFEATURE_SOLID
#define GJFEATURESET_HERO_HEALTH \
  GJFEATURE_MPACK_WAM GJFEATURE_GAMECOORD GJFEATURE_DISPLAYED \
  "Hlth"
#define GJFEATURESET_HERO_GRAVE \
  GJFEATURE_MPACK_WAM GJFEATURE_GAMECOORD GJFEATURE_DISPLAYED \
  GJFEATURE_SOLID "Grve"
#define GJFEATURESET_BAZOOKA \
  GJFEATURE_MPACK_WAM GJFEATURE_WEAPON \
  GJFEATURE_DISPLAYED "Bazo"
#define GJFEATURESET_ROCKET \
  GJFEATURE_MPACK_WAM GJFEATURE_GAME_PHYSICS GJFEATURE_GAMECOORD \
  GJFEATURE_DISPLAYED GJFEATURE_SOLID "Rckt"
#define GJFEATURESET_EXPLOSION \
  GJFEATURE_MPACK_WAM GJFEATURE_GAMECOORD GJFEATURE_DISPLAYED "Boom"
#define GJFEATURESET_ENVIRONMENTMGR \
  GJFEATURE_MPACK_WAM GJFEATURE_DISPLAYED GJFEATURE_MANAGER \
  GJFEATURE_ENVIRONMENT GJFEATURE_SOLID 
#define GJFEATURESET_ROUNDSGAME_SUPERVISOR \
  GJFEATURE_MPACK_WAM GJFEATURE_SUPERVISOR "Roun"
#define GJFEATURESET_SIMULTANEOUSGAME_SUPERVISOR \
  GJFEATURE_MPACK_WAM GJFEATURE_SUPERVISOR "Simu"



// game message names ---------------------------------------------------------
// internal

// user interaction, prefixed by +/- (!)
#define MSG_STEER_JUMP   "jump"
#define MSG_STEER_LEFT   "left"
#define MSG_STEER_RIGHT   "right"
#define MSG_STEER_CROUCH  "crouch"
#define MSG_STEER_UP      "up"
#define MSG_STEER_DOWN    "down"
#define MSG_FIRE          "fire"

#define MSG_WAM_PREFIX   "wam/"
#define MSG_WEAPON_ENUMERATE  MSG_WAM_PREFIX "weapons-enumerate"
#define MSG_WEAPON_LOOKUP  MSG_WAM_PREFIX "weapon-lookup"
#define MSG_WEAPON_MENU   MSG_WAM_PREFIX"weapon_menu"
#define MSG_CYCLE_FOCUS   MSG_WAM_PREFIX"cycle_focus"

#define MSG_EXPLOSION   MSG_WAM_PREFIX"explosion"
  // par2 is pointer to wExplosion

#define MSG_SUPERVISOR_PREFIX  MSG_WAM_PREFIX "supervisor/"
#define MSG_SUPERVISOR_ACKNOWLEDGE MSG_SUPERVISOR_PREFIX "ack" // (request_id)
#define MSG_SUPERVISOR_REJECT  MSG_SUPERVISOR_PREFIX "rej" // (request_id:reason)
#define MSG_SUPERVISOR_GET_FOCUS MSG_SUPERVISOR_PREFIX "get_focus"
#define MSG_SUPERVISOR_LOSE_FOCUS MSG_SUPERVISOR_PREFIX "lose_focus"
#define MSG_SUPERVISOR_RETURN_FOCUS MSG_SUPERVISOR_PREFIX "return_focus"
#define MSG_SUPERVISOR_AVATAR_BEGIN_DEATH MSG_SUPERVISOR_PREFIX "begin_death" // (Identifier)
#define MSG_SUPERVISOR_AVATAR_REQUEST_DEATH MSG_SUPERVISOR_PREFIX "request_death" // (Identifier)
#define MSG_SUPERVISOR_AVATAR_DIE MSG_SUPERVISOR_PREFIX "die"




// weapon classes -------------------------------------------------------------
#define WC_SPECIAL  0
#define WC_GUN   1
#define WC_ROCKET  2
#define WC_RAY   3
#define WC_MOVEMENT  7
#define WC_GAMERELATED  8
#define WC_LAST   8




#endif
