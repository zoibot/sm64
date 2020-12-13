#include "game/bingo.h"
#include "game/bingo_tracking_collectables.h"


void bhv_signpost_init(void) {
    o->oBingoId = get_unique_id(BINGO_UPDATE_READ_SIGNPOST, o->oPosX, o->oPosY, o->oPosZ);
}