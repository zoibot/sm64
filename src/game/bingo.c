#include <ultra64.h>
#include <PR/os_cont.h>
#include <PR/os_libc.h>

#include "types.h"
#include "game.h"
#include "sm64.h"
#include "print.h"
#include "hud.h"
#include "area.h"
#include "save_file.h"
#include "bingo.h"
#include "ingame_menu.h"
#include "menu/file_select.h"
#include "engine/behavior_script.h"
#include "level_update.h"
#include "audio_defines.h"
#include "audio/external.h"
#include "bingo_objective_func.h"
#include "camera.h"

s32 gBingoInitialized = 0;
u32 gBingoInitialSeed = 0;

s64 gbGlobalBingoTimer = 0;
s32 gbBingoTarget = 1;
s32 gbBingosCompleted = 0;
s32 gbBingoShowCongratsCounter = 0;
s32 gbBingoShowCongratsLimit = 2;
s32 gbBingoTimerDisabled = 0;
s32 gbBingoShowTimer = 1;

s16 gbStarIndex = 0;

s32 gbCoinsJustGotten = 0;

s32 gBingoReverseJoystickActive = 0;
s32 gBingoClickGameActive = 0;
s32 gBingoClickCounter = 0;
s16 gBingoClickGamePrevCameraSettings = 0;
s32 gBingoClickGamePrevCameraIndex = 0;
s32 gBingoDaredevilActive = 0;
s32 gBingoDaredevilPrevHealth = 0;
s32 gStarSelectScreenActive = 0;

// Star Selector count models printed in the act selector menu.
enum BingoModifier gBingoStarSelected = BINGO_MODIFIER_NONE;

struct BingoObjective gBingoObjectives[25];
u8 gBingoObjectivesDisabled[BINGO_OBJECTIVE_TOTAL_AMOUNT] = { 0 };

static struct BingoObjectiveInfo sBingoObjectiveInfo[] = {
    { BINGO_OBJECTIVE_STAR, BINGO_ICON_STAR },
    { BINGO_OBJECTIVE_STAR_TIMED, BINGO_ICON_STAR_TIMED },
    { BINGO_OBJECTIVE_STAR_A_BUTTON_CHALLENGE, BINGO_ICON_STAR_A_BUTTON_CHALLENGE },
    { BINGO_OBJECTIVE_STAR_B_BUTTON_CHALLENGE, BINGO_ICON_STAR_B_BUTTON_CHALLENGE },
    { BINGO_OBJECTIVE_STAR_Z_BUTTON_CHALLENGE, BINGO_ICON_STAR_Z_BUTTON_CHALLENGE },
    { BINGO_OBJECTIVE_STAR_CLICK_GAME, BINGO_ICON_STAR_CLICK_GAME },
    { BINGO_OBJECTIVE_STAR_REVERSE_JOYSTICK, BINGO_ICON_STAR_REVERSE_JOYSTICK },
    { BINGO_OBJECTIVE_STAR_GREEN_DEMON, BINGO_ICON_STAR_GREEN_DEMON },
    { BINGO_OBJECTIVE_STAR_DAREDEVIL, BINGO_ICON_STAR_DAREDEVIL },
    { BINGO_OBJECTIVE_COIN, BINGO_ICON_COIN },
    { BINGO_OBJECTIVE_1UPS_IN_LEVEL, BINGO_ICON_1UPS_IN_LEVEL },
    { BINGO_OBJECTIVE_STARS_IN_LEVEL, BINGO_ICON_STARS_IN_LEVEL },
    { BINGO_OBJECTIVE_DANGEROUS_WALL_KICKS, BINGO_ICON_DANGEROUS_WALL_KICKS },
    { BINGO_OBJECTIVE_LOSE_MARIO_HAT, BINGO_ICON_MARIO_HAT },
    { BINGO_OBJECTIVE_BLJ, BINGO_ICON_BLJ },
    { BINGO_OBJECTIVE_BOWSER, BINGO_ICON_BOWSER },
    { BINGO_OBJECTIVE_MULTICOIN, BINGO_ICON_MULTICOIN },
    { BINGO_OBJECTIVE_MULTISTAR, BINGO_ICON_MULTISTAR },
    { BINGO_OBJECTIVE_SIGNPOST, BINGO_ICON_SIGNPOST },
    { BINGO_OBJECTIVE_RED_COIN, BINGO_ICON_RED_COIN },
    { BINGO_OBJECTIVE_EXCLAMATION_MARK_BOX, BINGO_ICON_EXCLAMATION_MARK_BOX },
    { BINGO_OBJECTIVE_AMPS, BINGO_ICON_AMP },
    { BINGO_OBJECTIVE_KILL_GOOMBAS, BINGO_ICON_KILL_GOOMBAS },
    { BINGO_OBJECTIVE_KILL_BOBOMBS, BINGO_ICON_KILL_BOBOMBS },
    { BINGO_OBJECTIVE_KILL_SPINDRIFTS, BINGO_ICON_KILL_SPINDRIFTS },
    { BINGO_OBJECTIVE_KILL_MR_IS, BINGO_ICON_KILL_MR_IS },
};

struct BingoObjectiveInfo *get_objective_info(enum BingoObjectiveType type) {
    s32 i;
    s32 size = sizeof(sBingoObjectiveInfo) / sizeof(sBingoObjectiveInfo[0]);
    for (i = 0; i < size; i++) {
        if (sBingoObjectiveInfo[i].type == type) {
            return &sBingoObjectiveInfo[i];
        }
    }
    return NULL;
}

void set_objective_state(struct BingoObjective *objective, enum BingoObjectiveState state) {
    // Only play the corresponding sound once
    if (objective->state == state) {
        return;
    }

    switch (state) {
        case BINGO_STATE_COMPLETE:
            play_sound(SOUND_GENERAL2_RIGHT_ANSWER, gDefaultSoundArgs);
            bingo_hud_update_state(objective->icon, BINGO_ICON_SUCCESS);
            break;
        case BINGO_STATE_FAILED_IN_THIS_COURSE:
            play_sound(SOUND_MENU_CAMERA_BUZZ, gDefaultSoundArgs);
            bingo_hud_update_state(objective->icon, BINGO_ICON_FAILED);
            break;
    }
    objective->state = state;
}

/**
 * Return number of bingos on the board.
 */
u8 bingo_check_win() {
    u8 i, j;
    u8 buckets[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    u8 bingos;

    for (i = 0; i < 5; i++) {
        for (j = 0; j < 5; j++) {
            if (gBingoObjectives[i + j * 5].state == BINGO_STATE_COMPLETE) {
                buckets[i]++;
                buckets[j + 5]++;
                if (i == j) {
                    buckets[10]++;
                }
                if (i == 4 - j) {
                    buckets[11]++;
                }
            }
        }
    }

    bingos = 0;
    for (i = 0; i < 12; i++) {
        if (buckets[i] == 5) {
            bingos += 1;
        }
    }

    return bingos;
}

void bingo_update(enum BingoObjectiveUpdate update) {
    s32 i;
    // This is to avoid a bug where the call to bingo_update() from area.c
    // (the once-a-frame call) crashes before setup_bingo_objectives() has
    // been called.
    if (!gBingoInitialized) {
        return;
    }

    for (i = 0; i < 25; i++) {
        update_objective(&gBingoObjectives[i], update);
    }

    if (update == BINGO_UPDATE_TIMER_FRAME_GLOBAL && gbBingosCompleted < gbBingoTarget) {
        // Should we not increment if game is paused?
        // We probably should. Just a thought.
        gbGlobalBingoTimer++;
    }
    if (update == BINGO_UPDATE_COURSE_CHANGED) {
        // This is crufty but I can't think of any place else to put this...
        gBingoReverseJoystickActive = 0;

        gBingoDaredevilActive = 0;

        if (gBingoClickGameActive) {
            sSelectionFlags = gBingoClickGamePrevCameraSettings;
            gDialogCameraAngleIndex = gBingoClickGamePrevCameraIndex;
        }
        gBingoClickGameActive = 0;
        gBingoClickCounter = -1;

        // Put this here because, long story short, the bingo modifier
        // string renders before the game renders the stars; the star-rendering
        // is what resets this back to NONE otherwise.
        // TODO: be intelligent about retries
        gBingoStarSelected = BINGO_MODIFIER_NONE;
    }

    // Timer updates can never result in bingo being won
    if (update != BINGO_UPDATE_TIMER_FRAME_GLOBAL && update != BINGO_UPDATE_TIMER_FRAME_STAR) {
        gbBingosCompleted = bingo_check_win();
    }
}
