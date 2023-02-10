// koopa_shell_underwater.c.inc

struct ObjectHitbox sKoopaShellUnderwaterHitbox = {
    /* interactType: */ INTERACT_GRABBABLE,
    /* downOffset: */ 0,
    /* damageOrCoinValue: */ 0,
    /* health: */ 1,
    /* numLootCoins: */ 0,
    /* radius: */ 80,
    /* height: */ 50,
    /* hurtboxRadius: */ 0,
    /* hurtboxHeight: */ 0,
};

void func_802AA0D4(void) {
    set_object_hitbox(o, &sKoopaShellUnderwaterHitbox);
}


void bhv_koopa_shell_underwater_loop(void) {
    if(o->oTimer == 0) {
        o->oBingoId = get_unique_id(BINGO_UPDATE_RODE_SHELL, o->oPosX, o->oPosY, o->oPosZ);
    }
    switch (o->oHeldState) {
        case HELD_FREE:
            func_802AA0D4();
            break;
        case HELD_HELD:
            // TODO is this checking every frame?
            if (is_new_kill(BINGO_UPDATE_RODE_SHELL, o->oBingoId)) {
                bingo_update(BINGO_UPDATE_RODE_SHELL);
            }
            func_8029FA5C(-1, 0);
            break;
        case HELD_THROWN:
        case HELD_DROPPED:
            mark_object_for_deletion(o);
            func_802A3004();
            break;
    }
    if (o->oInteractStatus & INT_STATUS_STOP_RIDING) {
        mark_object_for_deletion(o);
        func_802A3004();
    }
    o->oInteractStatus = 0;
}
