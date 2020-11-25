#include <ultra64.h>
#include <PR/os_libc.h>

#include "area.h"
#include "bingo.h"

#include "bingo_descriptions.h"
#include "bingo_const.h"
#include "bingo_objective_func.h"
#include "bingo_objective_init.h"
#include "bingo_titles.h"
#include "segment2.h"
#include "strcpy.h"

#define TEXT_FILLED_STAR 0xFA

void get_course_abbreviation(enum CourseNum course, char *abbrev) {
    // TODO: Avoid buffer overflows
    strcpy(abbrev, courseAbbreviations[course - 1]); // offset since BOB = 1
}

////////////////////////////////////////////////////////////////////////////////

void get_star_objective_title(struct BingoObjective *objective) {
    enum CourseNum course = objective->data.starObjective.course;
    s32 star = objective->data.starObjective.starIndex;
    char abbrev[7] = { 0xFF };
    char PSSname[5] = { 0xFF };
    get_course_abbreviation(course, &abbrev);

    // We do "star + 1" here to make it human-readable.
    if (COURSE_IS_MAIN_COURSE(course)) {
        sprintf(objective->title, "%s%c%d", abbrev, TEXT_FILLED_STAR, star + 1);
    } else if (course == COURSE_PSS) {
        switch (star) {
            case PSS_STAR_FAST:
                strcpy(PSSname, "FAST");
                break;
            case PSS_STAR_SLOW:
                strcpy(PSSname, "SLOW");
                break;
        }
        sprintf(objective->title, "%s%c%s", abbrev, TEXT_FILLED_STAR, PSSname);
    } else {
        sprintf(objective->title, "%s%c", abbrev, TEXT_FILLED_STAR);
    }
}

void get_coin_objective_title(struct BingoObjective *objective) {
    enum CourseNum course = objective->data.courseCollectableData.course;
    s32 coins = objective->data.courseCollectableData.toGet;
    char abbrev[4];
    get_course_abbreviation(course, &abbrev);

    sprintf(objective->title, "%s %d", abbrev, coins);
}

void get_multicoin_objective_title(struct BingoObjective *objective) {
    sprintf(objective->title, "%d", objective->data.collectableData.toGet);
}

void get_in_level_objective_title(struct BingoObjective *objective) {
    char abbrev[4];
    get_course_abbreviation(objective->data.courseCollectableData.course, &abbrev);

    sprintf(objective->title, "%s", abbrev);
}

void get_lose_mario_hat_objective_title(struct BingoObjective *objective) {
    sprintf(objective->title, "x1");
}

void get_exclamation_mark_box_title(struct BingoObjective *objective) {
    sprintf(objective->title, "x%d", objective->data.collectableData.toGet);
}

void get_kill_enemy_title(struct BingoObjective *objective) {
    sprintf(objective->title, "x%d", objective->data.collectableData.toGet);
}

////////////////////////////////////////////////////////////////////////////////

void get_objective_title(struct BingoObjective *objective) {
    switch (objective->type) {
        case BINGO_OBJECTIVE_STAR:
        case BINGO_OBJECTIVE_STAR_A_BUTTON_CHALLENGE:
        case BINGO_OBJECTIVE_STAR_B_BUTTON_CHALLENGE:
        case BINGO_OBJECTIVE_STAR_Z_BUTTON_CHALLENGE:
        case BINGO_OBJECTIVE_STAR_TIMED:
        case BINGO_OBJECTIVE_STAR_REVERSE_JOYSTICK:
        case BINGO_OBJECTIVE_STAR_GREEN_DEMON:
            get_star_objective_title(objective);
            break;
        case BINGO_OBJECTIVE_COIN:
            get_coin_objective_title(objective);
            break;
        case BINGO_OBJECTIVE_MULTICOIN:
            get_multicoin_objective_title(objective);
            break;
        case BINGO_OBJECTIVE_1UPS_IN_LEVEL:
        case BINGO_OBJECTIVE_STARS_IN_LEVEL:
            get_in_level_objective_title(objective);
            break;
        case BINGO_OBJECTIVE_LOSE_MARIO_HAT:
        case BINGO_OBJECTIVE_BLJ:
            get_lose_mario_hat_objective_title(objective);
            break;
        case BINGO_OBJECTIVE_EXCLAMATION_MARK_BOX:
            get_exclamation_mark_box_title(objective);
            break;
        case BINGO_OBJECTIVE_KILL_GOOMBAS:
        case BINGO_OBJECTIVE_KILL_BOBOMBS:
            get_kill_enemy_title(objective);
            break;
    }
}