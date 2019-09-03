#include <ultra64.h>
#include <PR/os_libc.h>

#include "area.h"
#include "bingo.h"

#include "bingo_descriptions.h"
#include "bingo_const.h"
#include "bingo_objective_func.h"
#include "bingo_objective_init.h"
#include "bingo_star_tracking.h"
#include "segment2.h"
#include "strcpy.h"

s32 gDebugThing;

void objective_obtain_star(struct BingoObjective *objective) {
    s32 course;
    s32 star;

    if (sBingoCurrUpdate == BINGO_UPDATE_STAR) {
        course = objective->data.starObjective.course;
        star = objective->data.starObjective.starIndex;
        if (gCurrCourseNum == course && gbStarIndex == star) {
            set_objective_state(objective, BINGO_STATE_COMPLETE);
        }
    }
}

void objective_obtain_star_a_button_challenge(struct BingoObjective *objective) {
    if (sBingoCurrUpdate == BINGO_UPDATE_COURSE_CHANGED) {
        objective->state = BINGO_STATE_NONE;
    } else if (sBingoCurrUpdate == BINGO_UPDATE_A_PRESSED
               && gCurrCourseNum == objective->data.courseCollectableData.course) {
        set_objective_state(objective, BINGO_STATE_FAILED_IN_THIS_COURSE);
    } else if (objective->state != BINGO_STATE_FAILED_IN_THIS_COURSE) {
        // Regular star-obtaining stuff
        objective_obtain_star(objective);
    }
}

void objective_obtain_star_b_button_challenge(struct BingoObjective *objective) {
    if (sBingoCurrUpdate == BINGO_UPDATE_COURSE_CHANGED) {
        objective->state = BINGO_STATE_NONE;
    } else if (sBingoCurrUpdate == BINGO_UPDATE_B_PRESSED
               && gCurrCourseNum == objective->data.courseCollectableData.course) {
        set_objective_state(objective, BINGO_STATE_FAILED_IN_THIS_COURSE);
    } else if (objective->state != BINGO_STATE_FAILED_IN_THIS_COURSE) {
        // Regular star-obtaining stuff
        objective_obtain_star(objective);
    }
}

void objective_obtain_star_z_button_challenge(struct BingoObjective *objective) {
    if (sBingoCurrUpdate == BINGO_UPDATE_COURSE_CHANGED) {
        objective->state = BINGO_STATE_NONE;
    } else if (sBingoCurrUpdate == BINGO_UPDATE_Z_PRESSED
               && gCurrCourseNum == objective->data.courseCollectableData.course) {
        set_objective_state(objective, BINGO_STATE_FAILED_IN_THIS_COURSE);
    } else if (objective->state != BINGO_STATE_FAILED_IN_THIS_COURSE) {
        // Regular star-obtaining stuff
        objective_obtain_star(objective);
    }
}

void objective_obtain_star_timer(struct BingoObjective *objective) {
    if (sBingoCurrUpdate == BINGO_UPDATE_COURSE_CHANGED
        || sBingoCurrUpdate == BINGO_UPDATE_RESET_TIMER) {
        objective->state = BINGO_STATE_NONE;
        objective->data.starTimerObjective.timer = 0;
    } else if (objective->state == BINGO_STATE_FAILED_IN_THIS_COURSE) {
        return;
    } else if (sBingoCurrUpdate == BINGO_UPDATE_TIMER_FRAME
               && gCurrCourseNum == objective->data.starTimerObjective.course) {
        objective->data.starTimerObjective.timer++;
        if (objective->data.starTimerObjective.timer > objective->data.starTimerObjective.maxTime) {
            set_objective_state(objective, BINGO_STATE_FAILED_IN_THIS_COURSE);
        }
    } else if (objective->state != BINGO_STATE_FAILED_IN_THIS_COURSE) {
        objective_obtain_star(objective);
    }
}

void objective_obtain_coins(struct BingoObjective *objective) {
    if (sBingoCurrUpdate == BINGO_UPDATE_COURSE_CHANGED) {
        objective->state = BINGO_STATE_NONE;
        objective->data.courseCollectableData.gotten = 0;
    } else if (sBingoCurrUpdate == BINGO_UPDATE_COIN
               && gCurrCourseNum == objective->data.courseCollectableData.course) {
        objective->data.courseCollectableData.gotten += gbCoinsJustGotten;
        if (objective->data.courseCollectableData.gotten
            >= objective->data.courseCollectableData.toGet) {
            set_objective_state(objective, BINGO_STATE_COMPLETE);
        }
    }
}

void objective_obtain_multicoin(struct BingoObjective *objective) {
    if (sBingoCurrUpdate == BINGO_UPDATE_COIN) {
        objective->data.collectableData.gotten += gbCoinsJustGotten;
        if (objective->data.collectableData.gotten >= objective->data.collectableData.toGet) {
            set_objective_state(objective, BINGO_STATE_COMPLETE);
        }
    }
}

void objective_obtain_1ups_in_level(struct BingoObjective *objective) {
    if (sBingoCurrUpdate == BINGO_UPDATE_COURSE_CHANGED) {
        objective->state = BINGO_STATE_NONE;
        objective->data.courseCollectableData.gotten = 0;
    } else if (sBingoCurrUpdate == BINGO_UPDATE_GOT_1UP
               && gCurrCourseNum == objective->data.courseCollectableData.course) {
        objective->data.courseCollectableData.gotten++;
        if (objective->data.courseCollectableData.gotten
            == objective->data.courseCollectableData.toGet) {
            set_objective_state(objective, BINGO_STATE_COMPLETE);
        }
    }
}

void objective_obtain_stars_in_level(struct BingoObjective *objective) {
    enum CourseNum course = objective->data.courseCollectableData.course;
    if (sBingoCurrUpdate == BINGO_UPDATE_STAR && gCurrCourseNum == course) {
        if (bingo_get_course_count(course) >= 7) {
            set_objective_state(objective, BINGO_STATE_COMPLETE);
        }
    }
}

void objective_exclamation_mark_box(struct BingoObjective *objective) {
    if (sBingoCurrUpdate == BINGO_UPDATE_EXCLAMATION_MARK_BOX) {
        objective->data.collectableData.gotten++;
        if (objective->data.collectableData.gotten >= objective->data.collectableData.toGet) {
            set_objective_state(objective, BINGO_STATE_COMPLETE);
        }
    }
}

void objective_kill_enemy(struct BingoObjective *objective) {
    if (update_is_enemy_killing(sBingoCurrUpdate)
        && update_type_matches_enemy_type(sBingoCurrUpdate,
                                          objective->data.killEnemyObjective.enemyType)) {
        objective->data.killEnemyObjective.enemiesKilled++;
        if (objective->data.killEnemyObjective.enemiesToKill
                - objective->data.killEnemyObjective.enemiesKilled
            <= 0) {
            set_objective_state(objective, BINGO_STATE_COMPLETE);
        }
    }
}

void update_objective(struct BingoObjective *objective) {
    if (objective->state == BINGO_STATE_COMPLETE) {
        return;
    }

    switch (objective->type) {
        case BINGO_OBJECTIVE_STAR:
            objective_obtain_star(objective);
            break;
        case BINGO_OBJECTIVE_STAR_A_BUTTON_CHALLENGE:
            objective_obtain_star_a_button_challenge(objective);
            break;
        case BINGO_OBJECTIVE_STAR_B_BUTTON_CHALLENGE:
            objective_obtain_star_b_button_challenge(objective);
            break;
        case BINGO_OBJECTIVE_STAR_Z_BUTTON_CHALLENGE:
            objective_obtain_star_z_button_challenge(objective);
            break;
        case BINGO_OBJECTIVE_STAR_TIMED:
            objective_obtain_star_timer(objective);
            break;
        case BINGO_OBJECTIVE_COIN:
            objective_obtain_coins(objective);
            break;
        case BINGO_OBJECTIVE_MULTICOIN:
            objective_obtain_multicoin(objective);
            break;
        case BINGO_OBJECTIVE_1UPS_IN_LEVEL:
            objective_obtain_1ups_in_level(objective);
            break;
        case BINGO_OBJECTIVE_STARS_IN_LEVEL:
            objective_obtain_stars_in_level(objective);
            break;
        case BINGO_OBJECTIVE_EXCLAMATION_MARK_BOX:
            objective_exclamation_mark_box(objective);
            break;
        case BINGO_OBJECTIVE_KILL_ENEMIES:
            objective_kill_enemy(objective);
            break;
    }
}