#include <ultra64.h>
#include <PR/os_libc.h>

#include "area.h"
#include "bingo.h"

#include "bingo_descriptions.h"
#include "bingo_const.h"
#include "bingo_objective_func.h"
#include "bingo_objective_init.h"
#include "bingo_tracking_star.h"
#include "segment2.h"
#include "strcpy.h"
#include "camera.h"
#include "ingame_menu.h"
#include "print.h"


s32 objective_obtain_star(struct BingoObjective *objective, enum BingoObjectiveUpdate update) {
    s32 course;
    s32 star;

    if (update == BINGO_UPDATE_STAR) {
        course = objective->data.starObjective.course;
        star = objective->data.starObjective.starIndex;
        if (gCurrCourseNum == course && gbStarIndex == star) {
            set_objective_state(objective, BINGO_STATE_COMPLETE);
        }
    }
}

s32 objective_obtain_abz_button_challenges(
    struct BingoObjective *objective,
    enum BingoObjectiveUpdate update,
    enum BingoObjectiveUpdate buttonUpdate
) {
    if (update == BINGO_UPDATE_COURSE_CHANGED) {
        objective->state = BINGO_STATE_NONE;
    } else if (
        update == buttonUpdate
        && gCurrCourseNum == objective->data.courseCollectableData.course
    ) {
        set_objective_state(objective, BINGO_STATE_FAILED_IN_THIS_COURSE);
    } else if (objective->state != BINGO_STATE_FAILED_IN_THIS_COURSE) {
        objective_obtain_star(objective, update);
    }
}

s32 objective_obtain_star_timer(struct BingoObjective *objective, enum BingoObjectiveUpdate update) {
    if (update == BINGO_UPDATE_COURSE_CHANGED
        || update == BINGO_UPDATE_RESET_TIMER) {
        objective->state = BINGO_STATE_NONE;
        objective->data.starTimerObjective.timer = 0;
    } else if (objective->state == BINGO_STATE_FAILED_IN_THIS_COURSE) {
        return;
    } else if (
        update == BINGO_UPDATE_TIMER_FRAME
        && gCurrCourseNum == objective->data.starTimerObjective.course
        && !gbBingoTimerDisabled
    ) {
        objective->data.starTimerObjective.timer++;
        if (objective->data.starTimerObjective.timer > objective->data.starTimerObjective.maxTime) {
            set_objective_state(objective, BINGO_STATE_FAILED_IN_THIS_COURSE);
        }
    } else if (objective->state != BINGO_STATE_FAILED_IN_THIS_COURSE) {
        objective_obtain_star(objective, update);
    }
}

s32 objective_obtain_star_reverse_joystick(struct BingoObjective *objective, enum BingoObjectiveUpdate update) {
    s32 course;
    s32 star;

    if (update == BINGO_UPDATE_COURSE_CHANGED) {
        gBingoReverseJoystickActive = 0;
    } else if (update == BINGO_UPDATE_STAR && gBingoReverseJoystickActive) {
        course = objective->data.starObjective.course;
        star = objective->data.starObjective.starIndex;
        if (gCurrCourseNum == course && gbStarIndex == star) {
            set_objective_state(objective, BINGO_STATE_COMPLETE);
        }
    }
}

s32 objective_obtain_star_greendemon(struct BingoObjective *objective, enum BingoObjectiveUpdate update) {
    s32 course;
    s32 star;

    if (
        update == BINGO_UPDATE_STAR
        && gBingoStarSelected == BINGO_MODIFIER_GREEN_DEMON
    ) {
        course = objective->data.starObjective.course;
        star = objective->data.starObjective.starIndex;
        if (gCurrCourseNum == course && gbStarIndex == star) {
            set_objective_state(objective, BINGO_STATE_COMPLETE);
        }
    }
}

s32 objective_obtain_star_click_game(struct BingoObjective *objective, enum BingoObjectiveUpdate update) {
    s32 course;
    s32 star;

    if (update == BINGO_UPDATE_COURSE_CHANGED) {
        if (gBingoClickGameActive) {
            sSelectionFlags = gBingoClickGamePrevCameraSettings;
            gDialogCameraAngleIndex = gBingoClickGamePrevCameraIndex;
        }
        gBingoClickGameActive = 0;
        objective->data.starClicksObjective.clicks = -1;  // to as32 bug where entering a level is a click
    } else if (objective->state == BINGO_STATE_FAILED_IN_THIS_COURSE) {
        return;
    } else if (
        update == BINGO_UPDATE_CAMERA_CLICK
        && gCurrCourseNum == objective->data.starClicksObjective.course
    ) {
        objective->data.starClicksObjective.clicks++;
        // gBingoClickCounter = objective->data.starClicksObjective.clicks;
        if (objective->data.starClicksObjective.clicks > objective->data.starClicksObjective.maxClicks) {
            // objective->state = BINGO_STATE_FAILED_IN_THIS_COURSE;
        }
    } else if (update == BINGO_UPDATE_STAR && gBingoClickGameActive) {
        course = objective->data.starObjective.course;
        star = objective->data.starObjective.starIndex;
        if (gCurrCourseNum == course && gbStarIndex == star) {
            set_objective_state(objective, BINGO_STATE_COMPLETE);
        }
    }
}

s32 objective_obtain_star_daredevil(struct BingoObjective *objective, enum BingoObjectiveUpdate update) {
    s32 course;
    s32 star;

    if (update == BINGO_UPDATE_COURSE_CHANGED) {
        gBingoDaredevilActive = 0;
    } else if (update == BINGO_UPDATE_STAR && gBingoDaredevilActive) {
        course = objective->data.starObjective.course;
        star = objective->data.starObjective.starIndex;
        if (gCurrCourseNum == course && gbStarIndex == star) {
            // gBingoDaredevilActive = 0;
            set_objective_state(objective, BINGO_STATE_COMPLETE);
        }
    }
}

s32 objective_obtain_coins(struct BingoObjective *objective, enum BingoObjectiveUpdate update) {
    if (update == BINGO_UPDATE_COURSE_CHANGED) {
        objective->state = BINGO_STATE_NONE;
        objective->data.courseCollectableData.gotten = 0;
    } else if (
        update == BINGO_UPDATE_COIN
        && gCurrCourseNum == objective->data.courseCollectableData.course
    ) {
        objective->data.courseCollectableData.gotten += gbCoinsJustGotten;
        if (
            objective->data.courseCollectableData.gotten
            >= objective->data.courseCollectableData.toGet
        ) {
            set_objective_state(objective, BINGO_STATE_COMPLETE);
        }
    }
}

s32 objective_obtain_multicoin(struct BingoObjective *objective, enum BingoObjectiveUpdate update) {
    if (update == BINGO_UPDATE_COIN) {
        objective->data.collectableData.gotten += gbCoinsJustGotten;
        if (objective->data.collectableData.gotten >= objective->data.collectableData.toGet) {
            set_objective_state(objective, BINGO_STATE_COMPLETE);
        }
    }
}

s32 objective_obtain_multistar(struct BingoObjective *objective, enum BingoObjectiveUpdate update) {
    if (update == BINGO_UPDATE_STAR) {
        objective->data.collectableData.gotten = bingo_get_star_count();
        if (bingo_get_star_count() == objective->data.collectableData.toGet) {
            set_objective_state(objective, BINGO_STATE_COMPLETE);
        }
    }
}

s32 objective_obtain_1ups_in_level(struct BingoObjective *objective, enum BingoObjectiveUpdate update) {
    if (update == BINGO_UPDATE_COURSE_CHANGED) {
        objective->state = BINGO_STATE_NONE;
        objective->data.courseCollectableData.gotten = 0;
    } else if (
        update == BINGO_UPDATE_GOT_1UP
        && gCurrCourseNum == objective->data.courseCollectableData.course
    ) {
        objective->data.courseCollectableData.gotten++;
        if (objective->data.courseCollectableData.gotten
            == objective->data.courseCollectableData.toGet) {
            set_objective_state(objective, BINGO_STATE_COMPLETE);
        }
    }
}

s32 objective_obtain_stars_in_level(struct BingoObjective *objective, enum BingoObjectiveUpdate update) {
    enum CourseNum course = objective->data.courseCollectableData.course;
    if (update == BINGO_UPDATE_STAR && gCurrCourseNum == course) {
        if (bingo_get_course_count(course) >= 7) {
            set_objective_state(objective, BINGO_STATE_COMPLETE);
        }
    }
}

s32 objective_lose_mario_hat(struct BingoObjective *objective, enum BingoObjectiveUpdate update) {
    if (update == BINGO_UPDATE_LOST_HAT) {
        set_objective_state(objective, BINGO_STATE_COMPLETE);
    }
}

s32 objective_blj(struct BingoObjective *objective, enum BingoObjectiveUpdate update) {
    if (update == BINGO_UPDATE_BLJ) {
        set_objective_state(objective, BINGO_STATE_COMPLETE);
    }
}

s32 objective_generic_collectable(
    struct BingoObjective *objective,
    enum BingoObjectiveUpdate update,
    enum BingoObjectiveUpdate desiredUpdate
) {
    if (update == desiredUpdate) {
        objective->data.collectableData.gotten++;
        if (objective->data.collectableData.gotten >= objective->data.collectableData.toGet) {
            set_objective_state(objective, BINGO_STATE_COMPLETE);
        }
    }
}

s32 update_objective(struct BingoObjective *objective, enum BingoObjectiveUpdate update) {
    if (objective->state == BINGO_STATE_COMPLETE) {
        return;
    }

    switch (objective->type) {
        case BINGO_OBJECTIVE_STAR:
            return objective_obtain_star(objective, update);
        case BINGO_OBJECTIVE_STAR_A_BUTTON_CHALLENGE:
            return objective_obtain_abz_button_challenges(objective, update, BINGO_UPDATE_A_PRESSED);
        case BINGO_OBJECTIVE_STAR_B_BUTTON_CHALLENGE:
            return objective_obtain_abz_button_challenges(objective, update, BINGO_UPDATE_B_PRESSED);
        case BINGO_OBJECTIVE_STAR_Z_BUTTON_CHALLENGE:
            return objective_obtain_abz_button_challenges(objective, update, BINGO_UPDATE_Z_PRESSED);
        case BINGO_OBJECTIVE_STAR_TIMED:
            return objective_obtain_star_timer(objective, update);
        case BINGO_OBJECTIVE_STAR_REVERSE_JOYSTICK:
            return objective_obtain_star_reverse_joystick(objective, update);
        case BINGO_OBJECTIVE_STAR_GREEN_DEMON:
            return objective_obtain_star_greendemon(objective, update);
        case BINGO_OBJECTIVE_STAR_CLICK_GAME:
            return objective_obtain_star_click_game(objective, update);
        case BINGO_OBJECTIVE_STAR_DAREDEVIL:
            return objective_obtain_star_daredevil(objective, update);
        case BINGO_OBJECTIVE_COIN:
            return objective_obtain_coins(objective, update);
        case BINGO_OBJECTIVE_MULTICOIN:
            return objective_obtain_multicoin(objective, update);
        case BINGO_OBJECTIVE_MULTISTAR:
            return objective_obtain_multistar(objective, update);
        case BINGO_OBJECTIVE_1UPS_IN_LEVEL:
            return objective_obtain_1ups_in_level(objective, update);
        case BINGO_OBJECTIVE_STARS_IN_LEVEL:
            return objective_obtain_stars_in_level(objective, update);
        case BINGO_OBJECTIVE_LOSE_MARIO_HAT:
            return objective_lose_mario_hat(objective, update);
        case BINGO_OBJECTIVE_BLJ:
            return objective_blj(objective, update);
        case BINGO_OBJECTIVE_RED_COIN:
            return objective_generic_collectable(objective, update, BINGO_UPDATE_RED_COIN);
        case BINGO_OBJECTIVE_EXCLAMATION_MARK_BOX:
            return objective_generic_collectable(objective, update, BINGO_UPDATE_EXCLAMATION_MARK_BOX);
        case BINGO_OBJECTIVE_KILL_GOOMBAS:
            return objective_generic_collectable(objective, update, BINGO_UPDATE_KILLED_GOOMBA);
        case BINGO_OBJECTIVE_KILL_BOBOMBS:
            return objective_generic_collectable(objective, update, BINGO_UPDATE_KILLED_BOBOMB);
    }
}