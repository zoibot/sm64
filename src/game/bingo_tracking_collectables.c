#include <ultra64.h>
#include "types.h"
#include "area.h"
#include "bingo_tracking_collectables.h"

struct GlobalPosition {
    enum CourseNum course;
    Vec3f pos;
};

// There are something like 57 Goombas and 51 ! boxes.
// There are also 8 * 15 = 120 red coins. So I think
// it's very safe to make this table this size.
// The first dimension is what type of collectable it is.
// The second is the (x, y, z) position. The index into
// that second dimension is the globally unique ID of the
// object.
// I really hope nothing is actually at (0, 0, 0)....
// Also, I'm worried this might get pretty big. Maybe pointers to
// arrays that vary in size based on collectable?
#define MAX_UIDS 120
struct GlobalPosition sIDTable[3][MAX_UIDS] = { { 0 } };
u8 sKillTable[BINGO_COLLECTABLES_MAX - BINGO_COLLECTABLES_MIN + 1][MAX_UIDS] = { { 0 } };

s32 get_index(enum BingoObjectiveUpdate update) {
    // if (update != BINGO_UPDATE_KILLED_GOOMBA) {
    //     return -1;  // for now, just goombas
    // }
    return update - BINGO_COLLECTABLES_MIN;
}

u16 get_unique_id(enum BingoObjectiveUpdate type, f32 posX, f32 posY, f32 posZ) {
    s32 index = get_index(type);
    u16 j;
    struct GlobalPosition *global;
    if (index == -1) {
        return -1;
    }
    for (j = 0; j < MAX_UIDS; j++) {
        global = &sIDTable[index][j];
        if (
            global->course == gCurrCourseNum
            && global->pos[0] == posX && global->pos[1] == posY && global->pos[2] == posZ
        ) {
            return j;
        }

        if (global->pos[0] == 0 && global->pos[1] == 0 && global->pos[2] == 0) {
            global->course = gCurrCourseNum;
            global->pos[0] = posX;
            global->pos[1] = posY;
            global->pos[2] = posZ;
            return j;
        }
    }
    // No empty slots somehow?!
    return -1;
}

s32 is_new_kill(enum BingoObjectiveUpdate type, u16 uid) {
    u8 killed;
    s32 index = get_index(type);

    if (uid == -1 || index == -1) {
        return 1;
    }
    killed = sKillTable[index][uid];
    if (killed) {
        return 0;
    } else {
        sKillTable[index][uid] = 1;
        return 1;
    }
}