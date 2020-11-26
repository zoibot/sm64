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

struct ObjectiveWeight {
    enum BingoObjectiveType objective;
    s32 weight;
};

struct ObjectiveWeight sWeightsEasy[] = {
    { BINGO_OBJECTIVE_COIN, 12 },
    { BINGO_OBJECTIVE_STAR, 12 },
    { BINGO_OBJECTIVE_LOSE_MARIO_HAT, 12 },
    { BINGO_OBJECTIVE_BLJ, 12 },
    { BINGO_OBJECTIVE_MULTISTAR, 12 },
};
s32 sWeightsSizeEasy = sizeof(sWeightsEasy) / sizeof(struct ObjectiveWeight);

struct ObjectiveWeight sWeightsMedium[] = {
    { BINGO_OBJECTIVE_COIN, 12 },
    { BINGO_OBJECTIVE_STAR, 12 },
    { BINGO_OBJECTIVE_KILL_GOOMBAS, 6 },
    { BINGO_OBJECTIVE_KILL_BOBOMBS, 6 },
    { BINGO_OBJECTIVE_STAR_TIMED, 12 },
    { BINGO_OBJECTIVE_STAR_B_BUTTON_CHALLENGE, 3 },
    { BINGO_OBJECTIVE_STAR_Z_BUTTON_CHALLENGE, 3 },
    { BINGO_OBJECTIVE_EXCLAMATION_MARK_BOX, 12 },
    { BINGO_OBJECTIVE_MULTISTAR, 12 },
};
s32 sWeightsSizeMedium = sizeof(sWeightsMedium) / sizeof(struct ObjectiveWeight);

struct ObjectiveWeight sWeightsHard[] = {
    { BINGO_OBJECTIVE_STAR_TIMED, 12 },
    { BINGO_OBJECTIVE_STAR_A_BUTTON_CHALLENGE, 12 },
    { BINGO_OBJECTIVE_1UPS_IN_LEVEL, 12 },
    { BINGO_OBJECTIVE_STARS_IN_LEVEL, 12 },
    { BINGO_OBJECTIVE_MULTICOIN, 3 },
    { BINGO_OBJECTIVE_STAR_REVERSE_JOYSTICK, 12 },
    { BINGO_OBJECTIVE_STAR_GREEN_DEMON, 12 },
    { BINGO_OBJECTIVE_MULTISTAR, 12 },
};
s32 sWeightsSizeHard = sizeof(sWeightsHard) / sizeof(struct ObjectiveWeight);

struct ObjectiveWeight sWeightsCenter[] = {
    { BINGO_OBJECTIVE_COIN, 12 },
    { BINGO_OBJECTIVE_KILL_GOOMBAS, 6 },
    { BINGO_OBJECTIVE_KILL_BOBOMBS, 6 },
    { BINGO_OBJECTIVE_MULTICOIN, 12 },
    { BINGO_OBJECTIVE_MULTISTAR, 12 },
};
s32 sWeightsSizeCenter = sizeof(sWeightsCenter) / sizeof(struct ObjectiveWeight);

enum BingoObjectiveType get_random_objective_type(enum BingoObjectiveClass class) {
    struct ObjectiveWeight *weights;
    s32 size;
	s32 sum;
	s32 want_sum;
	s32 i;

    switch (class) {
        case BINGO_CLASS_EASY:
            weights = sWeightsEasy;
            size = sWeightsSizeEasy;
            break;
        case BINGO_CLASS_MEDIUM:
            weights = sWeightsMedium;
            size = sWeightsSizeMedium;
            break;
        case BINGO_CLASS_HARD:
            weights = sWeightsHard;
            size = sWeightsSizeHard;
            break;
        case BINGO_CLASS_CENTER:
            weights = sWeightsCenter;
            size = sWeightsSizeCenter;
            break;
    }
    sum = 0.0f;
    for (i = 0; i < size; i++) {
        sum += weights[i].weight;
    }
    want_sum = RandomU16() % sum;

    i = 0;
    sum = 0;
    while ((sum += weights[i].weight) < want_sum) {
        i++;
    }
    return weights[i].objective;
}

enum BingoObjectiveType get_random_enabled_objective_type(enum BingoObjectiveClass class) {
    u32 attempts = 50;
    enum BingoObjectiveType candidate;
    enum BingoObjectiveType i;
    s32 enabledSum = 0;
    u32 randomIndex;
    u32 enabledCounter = 0;

    while (attempts > 0) {
        candidate = get_random_objective_type(class);
        if (!gBingoObjectivesDisabled[candidate]) {
            return candidate;
        }
        attempts--;
    }
    // Tried a few times; just give up and get a completely random objective
    for (i = BINGO_OBJECTIVE_TYPE_MIN; i < BINGO_OBJECTIVE_TOTAL_AMOUNT; i++) {
        if (!gBingoObjectivesDisabled[i]) {
            enabledSum++;
        }
    }
    if (enabledSum == 0) {
        // All objectives are disabled. I guess just allow free play?
        return;
    }
    randomIndex = (RandomU16() % enabledSum) + 1;
    for (i = BINGO_OBJECTIVE_TYPE_MIN; i < BINGO_OBJECTIVE_TOTAL_AMOUNT; i++) {
        if (!gBingoObjectivesDisabled[i]) {
            enabledCounter++;
        }
        if (enabledCounter == randomIndex) {
            return i;
        }
    }
    // We shouldn't get here.
}

s32 switch_to(s32 exclude) {
    s32 otherOne, otherTwo;
    s32 switchTo;
    switch (exclude) {
        case 0:
            otherOne = 1;
            otherTwo = 2;
            break;
        case 1:
            otherOne = 0;
            otherTwo = 2;
            break;
        case 2:
            otherOne = 1;
            otherTwo = 2;
            break;
        case 3:
            otherOne = 3;  // remain hard
            otherTwo = 1;  // easy or harder
    }
    switch (RandomU16() % 5) {
        case 0:
        case 1:
            switchTo = otherOne;
            break;
        case 2:
        case 3:
            switchTo = otherTwo;
            break;
        case 4:
            switchTo = 3;
            break;
    }
    return switchTo;
}

void shuffle(s32 *array, s32 length) {
    s32 i, j, choice;
    for (i = 0; i < length - 1; i++)
    {
        j = i + RandomU16() / (65535 / (length - i) + 1);
        choice = array[j];
        array[j] = array[i];
        array[i] = choice;
    }
}

////////////////////////////////////////////////////////////////////////////////

// Bingo setup and update hooks
void setup_bingo_objectives(u32 seed) {
    int row, col;
    s32 objectiveClasses[5][5] = {
        { 1, 0, 0, 0, 2 }, { 0, 2, 0, 1, 0 }, { 0, 0, 3, 0, 0 }, { 0, 1, 0, 2, 0 }, { 2, 0, 0, 0, 1 }
    };
    s32 indexOrder[25] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24
    };
    s32 harderClass, easierClass, class;
    enum BingoObjectiveClass classType;
    s32 i, index;
    enum BingoObjectiveType type;
    struct BingoObjective *objective;
    s32 prevDisabledSettings[BINGO_OBJECTIVE_TOTAL_AMOUNT];
    s32 objectiveCounts[BINGO_OBJECTIVE_TOTAL_AMOUNT] = { 0 };

    // Initialize random number subsystem
    init_genrand(seed);

    gBingoInitialized = 1;
    gBingoInitialSeed = seed;

    harderClass = (RandomU16() % 2) + 1; // either 1 or 2
    easierClass = (2 - harderClass) + 1; // either 2 or 1, opposite harderClass

    // To shake it up a bit, mutate between 0 and 2 of the cells randomly.
    for (i = 0; i < RandomU16() % 3; i++) {
        row = RandomU16() % 5;
        col = RandomU16() % 5;
        objectiveClasses[row][col] = switch_to(objectiveClasses[row][col]);
    }

    // Save the options settings (see below for why).
    for (i = 0; i < BINGO_OBJECTIVE_TOTAL_AMOUNT; i++) {
        prevDisabledSettings[i] = gBingoObjectivesDisabled[i];
    }

    // NOTE!
    // We populate the bingo grid in random order because of restrictions on
    // amounts of duplicate objective types. If we were to populate the grid
    // in a fixed order every time, while also restricting that an objective
    // e.g. were to only appear once, then that objective would appear more
    // often in particular parts of the grid. We want "as random as possible"
    // hence this design choice.
    shuffle(&indexOrder, 25);

    for (i = 0; i < 25; i++) {
        index = indexOrder[i];
        class = objectiveClasses[index % 5][index / 5];
        objective = &gBingoObjectives[index];

        if (class == harderClass) {
            classType = BINGO_CLASS_HARD;
        } else if (class == easierClass) {
            classType = BINGO_CLASS_EASY;
        } else if (class == 3) {
            classType = BINGO_CLASS_CENTER;
        } else if (class == 0) {
            classType = BINGO_CLASS_MEDIUM;
        }
        // TODO: avoid duplicate single stars, and having
        // multiple multistars/multicoins in one row/col.
        type = get_random_enabled_objective_type(classType);
        bingo_objective_init(objective, classType, type);
        objectiveCounts[type]++;
        // TODO: Make {class, type} a unique pair per board for certain
        // objectives like coins, bob-ombs, etc. Having these done individually
        // is a little lame.
        if (
            (
                (objectiveCounts[type] == 1)
                && (
                    type == BINGO_OBJECTIVE_LOSE_MARIO_HAT
                    || type == BINGO_OBJECTIVE_BLJ
                )
            )
            || (
                (objectiveCounts[type] == 2)
                && (
                    type == BINGO_OBJECTIVE_EXCLAMATION_MARK_BOX
                    || type == BINGO_OBJECTIVE_STAR_A_BUTTON_CHALLENGE
                )
            )
            || (
                (objectiveCounts[type] == 3)
                && (
                    type == BINGO_OBJECTIVE_KILL_GOOMBAS
                    || type == BINGO_OBJECTIVE_KILL_BOBOMBS
                    || type == BINGO_OBJECTIVE_STAR_B_BUTTON_CHALLENGE
                    || type == BINGO_OBJECTIVE_STAR_Z_BUTTON_CHALLENGE
                    || type == BINGO_OBJECTIVE_MULTISTAR
                )
            )
        ) {
            gBingoObjectivesDisabled[type] = 1;
        }
    }
    // Note - if a player Saves & Quits, and we didn't un-disable these
    // objectives, they would be disabled for future games. So, revert
    // it below.
    for (i = 0; i < BINGO_OBJECTIVE_TOTAL_AMOUNT; i++) {
        gBingoObjectivesDisabled[i] = prevDisabledSettings[i];
    }
}