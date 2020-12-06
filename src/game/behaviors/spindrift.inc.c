// spindrift.c.inc
#include "game/bingo.h"
#include "game/bingo_tracking_collectables.h"

struct ObjectHitbox sSpindriftHitbox = {
    /* interactType: */ INTERACT_BOUNCE_TOP,
    /* downOffset: */ 0,
    /* damageOrCoinValue: */ 2,
    /* health: */ 1,
    /* numLootCoins: */ 3,
    /* radius: */ 90,
    /* height: */ 80,
    /* hurtboxRadius: */ 80,
    /* hurtboxHeight: */ 70,
};

void bhv_spindrift_loop(void) {
    o->activeFlags |= 0x400;
    if (obj_set_hitbox_and_die_if_attacked_bingo(
        &sSpindriftHitbox, SOUND_OBJ_DYING_ENEMY1, 0, BINGO_UPDATE_KILLED_SPINDRIFT
    ))
        obj_change_action(2);
    obj_update_floor_and_walls();
    switch (o->oAction) {
        case 0:
            o->oBingoId = get_unique_id(BINGO_UPDATE_KILLED_SPINDRIFT, o->oPosX, o->oPosY, o->oPosZ);
            o->oAction = 1;
            break;
        case 1:
            func_802A9050(&o->oForwardVel, 4.0f, 1.0f);
            if (obj_lateral_dist_from_mario_to_home() > 1000.0f)
                o->oAngleToMario = obj_angle_to_home();
            else if (o->oDistanceToMario > 300.0f)
                o->oAngleToMario = angle_to_object(o, gMarioObject);
            obj_rotate_yaw_toward(o->oAngleToMario, 0x400);
            break;
        case 2:
            o->oFlags &= ~8;
            o->oForwardVel = -10.0f;
            if (o->oTimer > 20) {
                o->oAction = 1;
                o->oInteractStatus = 0;
                o->oFlags |= 8;
            }
            break;
    }
    obj_move_standard(-60);
}
