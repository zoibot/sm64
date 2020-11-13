#include <ultra64.h>
#include <PR/os_libc.h>

#include "area.h"
#include "bingo.h"

#include "engine/behavior_script.h"
#include "engine/rand.h"
#include "bingo_descriptions.h"
#include "bingo_const.h"
#include "bingo_objective_func.h"
#include "bingo_objective_init.h"
#include "segment2.h"
#include "strcpy.h"
#include "print.h"

enum BingoObjectiveType get_random_objective_type(enum BingoObjectiveClass class) {
    switch (class) {
        case BINGO_CLASS_HARD:
        retry_hard:
            switch (RandomU16() % 7) {
                case 0:
                    return BINGO_OBJECTIVE_STAR_TIMED;
                case 1:
                    return BINGO_OBJECTIVE_STAR_A_BUTTON_CHALLENGE;
                case 2:
                    return BINGO_OBJECTIVE_1UPS_IN_LEVEL;
                case 3:
                    return BINGO_OBJECTIVE_STARS_IN_LEVEL;
                case 4:
                    if (RandomU16() % 4 != 0) {
                        goto retry_hard;
                    }
                    return BINGO_OBJECTIVE_MULTICOIN;
                case 5:
                    return BINGO_OBJECTIVE_STAR_REVERSE_JOYSTICK;
                case 6:
                    return BINGO_OBJECTIVE_STAR_GREEN_DEMON;
            }
            break;

        case BINGO_CLASS_EASY:
            switch (RandomU16() % 2) {
                case 0:
                    return BINGO_OBJECTIVE_COIN;
                case 1:
                    return BINGO_OBJECTIVE_STAR;
            }
            break;

        case BINGO_CLASS_CENTER:
            switch (RandomU16() % 3) {
                case 0:
                    return BINGO_OBJECTIVE_COIN;
                case 1:
                    return BINGO_OBJECTIVE_KILL_ENEMIES;
                case 2:
                    return BINGO_OBJECTIVE_MULTICOIN;
            }
            break;

        case BINGO_CLASS_MEDIUM:
        retry_medium:
            switch (RandomU16() % 7) {
                case 0:
                    return BINGO_OBJECTIVE_COIN;
                case 1:
                    return BINGO_OBJECTIVE_STAR;
                case 2:
                    return BINGO_OBJECTIVE_KILL_ENEMIES;
                case 3:
                    return BINGO_OBJECTIVE_STAR_TIMED;
                case 4:
                    if (RandomU16() % 4 != 0) {
                        // If you ever wanted proof that I am lazy:
                        goto retry_medium;
                    }
                    return BINGO_OBJECTIVE_STAR_B_BUTTON_CHALLENGE;
                case 5:
                    if (RandomU16() % 4 != 0) {
                        goto retry_medium;
                    }
                    return BINGO_OBJECTIVE_STAR_Z_BUTTON_CHALLENGE;
                case 6:
                    return BINGO_OBJECTIVE_EXCLAMATION_MARK_BOX;
            }
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////

// Bingo setup and update hooks
void setup_bingo_objectives(u32 seed) {
    int row, col;
    s32 objectiveClasses[5][5] = {
        { 1, 0, 0, 0, 2 }, { 0, 2, 0, 1, 0 }, { 0, 0, 3, 0, 0 }, { 0, 1, 0, 2, 0 }, { 2, 0, 0, 0, 1 }
    };
    s32 harderClass, easierClass, class;
    enum BingoObjectiveType type;
    struct BingoObjective *objective;

    // Initialize random number subsystem
    init_genrand(seed);

    gBingoInitialized = 1;
    gBingoInitialSeed = seed;

    harderClass = (RandomU16() % 2) + 1; // either 1 or 2
    easierClass = (2 - harderClass) + 1; // either 2 or 1, opposite harderClass

    // NOTE!
    // If we start throwing in restrictions about maximum numbers of
    // a particular objective type on a given board, then we have to
    // rotate it a random multiple of 90 degrees, otherwise the concentration
    // in the upper left will be different from that in the lower right.
    for (row = 0; row < 5; row++) {
        for (col = 0; col < 5; col++) {
            class = objectiveClasses[row][col];
            objective = &gBingoObjectives[row * 5 + col];

            if (class == harderClass) {
                type = get_random_objective_type(BINGO_CLASS_HARD);
                bingo_objective_init(objective, BINGO_CLASS_HARD, type);
            } else if (class == easierClass) {
                type = get_random_objective_type(BINGO_CLASS_EASY);
                bingo_objective_init(objective, BINGO_CLASS_EASY, type);
            } else if (class == 3) {
                type = get_random_objective_type(BINGO_CLASS_CENTER);
                bingo_objective_init(objective, BINGO_CLASS_CENTER, type);
            } else if (class == 0) {
                type = get_random_objective_type(BINGO_CLASS_MEDIUM);
                bingo_objective_init(objective, BINGO_CLASS_MEDIUM, type);
            }
        }
    }
}