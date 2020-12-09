#include <ultra64.h>
#include "area.h"
#include "bingo.h"

#include "engine/behavior_script.h"
#include "bingo_const.h"
#include "bingo_descriptions.h"
#include "bingo_objective_func.h"
#include "bingo_objective_init.h"
#include "bingo_titles.h"
#include "strcpy.h"
#include "level_table.h"

s32 random_abc(enum CourseNum *course, s32 *star, char **hint) {
    u32 the_star = RandomU16() % numPossibleABC;
    *course = possibleABC[the_star].course;
    *star = possibleABC[the_star].star - 1; // star is 0-indexed everywhere else :(
    *hint = possibleABC[the_star].hint;
}

enum CourseNum random_main_course() {
    return (RandomU16() % 15) + 1;
}

s32 random_star_main_course(enum CourseNum *course, s32 *star) {
    *course = random_main_course();
    *star = (RandomU16() % 7); // zero-indexed
}

s32 random_star_main_course_except_100c(enum CourseNum *course, s32 *star) {
    *course = random_main_course();
    *star = (RandomU16() % 6); // zero-indexed
}

s32 random_star_except_mips_toad(enum CourseNum *course, s32 *star) {
    // This is gonna be a bit janky since I want every star to have equal prob.
    s32 starIndex = (RandomU16() % 115); // 120 - (mips * 2) - (toad * 3)
    if (starIndex < (15 * 7)) {
        *course = (starIndex % 15) + 1;
        *star = starIndex % 7;
        return;
    }

    *star = 0;

    switch (starIndex - (15 * 7)) {
        case 0:
        case 1:
            // PSS
            *course = COURSE_PSS;
            *star = starIndex - (15 * 7); // PSS_STAR_SLOW or PSS_STAR_FAST
            break;
        case 2:
            // SA
            *course = COURSE_SA;
            break;
        case 3:
            // TotWC
            *course = COURSE_TOTWC;
            break;
        case 4:
            // CotMC
            *course = COURSE_COTMC;
            break;
        case 5:
            // VCutM
            *course = COURSE_VCUTM;
            break;
        case 6:
            // WMotR
            *course = COURSE_WMOTR;
            break;
        case 7:
            // BitDW
            *course = COURSE_BITDW;
            break;
        case 8:
            // BitFS
            *course = COURSE_BITFS;
            break;
        case 9:
            // BitS
            *course = COURSE_BITS;
            break;
    }

    return;
}

s32 random_bbc(enum CourseNum *course, s32 *star) {
retry:
    random_star_except_mips_toad(course, star);
    if (*course == COURSE_BOB && *star == 0) {
        // King Bob-omb on the summit can't be collected because you
        // have to pick him up
        goto retry;
    }
    if (*course == COURSE_TTM && *star == 1) {
        // Mystery of the Monkey Cage can't be collected because you
        // have to pick Ukiki up
        goto retry;
    }
    if (*course == COURSE_CCM && *star == 1) {
        // Li'l Penguin Lost is possible, but it's very hard. You have
        // to lead the lil penguin down to the bottom platform which requires
        // many hard techniques
        goto retry;
    }
}

s32 random_zbc(enum CourseNum *course, s32 *star) {
retry:
    random_star_except_mips_toad(course, star);
    if (*course == COURSE_WF && *star == 0) {
        // chip off whomps block is impossible, have to ground pound
        goto retry;
    }
    if (*course == COURSE_JRB && *star == 6) {
        // 100c in JRB is impossible because only 104 coins and need to
        // ground pound the blue coin thing
        goto retry;
    }
    // should i outlaw chain chomps gate in BoB because it requires bomb clip?
    if (*course == COURSE_DDD && *star == 6) {
        // not that many coins, and have to use difficult shell cloning
        goto retry;
    }
    // should I outlaw SL (igloo stuff) because you have to clip through it?
    // should I outlaw THI wiggler cave stuff because you have to clip using Chuckya?
    if (*course == COURSE_TTC && *star == 6) {
        // have to clone coins until you get 100 without blue coins
        goto retry;
    }
    // RELEVANT FOR OTHER GAME MODES:
    // vcutm can't be accessed without ground pounding the pillars
    // although ofc you can use PU speed lol
}

////////////////////////////////////////////////////////////////////////////////

s32 bingo_objective_collectable_init(
    struct BingoObjective *obj, enum BingoObjectiveIcon icon, s32 toGet
) {
    obj->icon = icon;
    obj->data.collectableData.toGet = toGet;
    obj->data.collectableData.gotten = 0;
    get_objective_title(obj);
}

////////////////////////////////////////////////////////////////////////////////

s32 bingo_objective_star_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    enum CourseNum course;
    s32 star;

    switch (class) {
        default:
            random_star_except_mips_toad(&course, &star);
            break;
    }
    objective->icon = BINGO_ICON_STAR;
    objective->data.starObjective.course = course;
    objective->data.starObjective.starIndex = star;
    get_objective_title(objective);
}

s32 bingo_objective_star_a_button_challenge_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    enum CourseNum course;
    s32 star;
    char *hint;

    switch (class) {
        default:
            random_abc(&course, &star, &hint);
            break;
    }
    objective->icon = BINGO_ICON_STAR_A_BUTTON_CHALLENGE;
    objective->data.abcStarObjective.course = course;
    objective->data.abcStarObjective.starIndex = star;
    objective->data.abcStarObjective.hint = hint;
    get_objective_title(objective);
}

s32 bingo_objective_star_b_button_challenge_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    enum CourseNum course;
    s32 star;

    switch (class) {
        default:
            random_bbc(&course, &star);
            break;
    }
    objective->icon = BINGO_ICON_STAR_B_BUTTON_CHALLENGE;
    objective->data.starObjective.course = course;
    objective->data.starObjective.starIndex = star;
    get_objective_title(objective);
}

s32 bingo_objective_star_z_button_challenge_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    enum CourseNum course;
    s32 star;

    switch (class) {
        default:
            random_zbc(&course, &star);
            break;
    }

    objective->icon = BINGO_ICON_STAR_Z_BUTTON_CHALLENGE;
    objective->data.starObjective.course = course;
    objective->data.starObjective.starIndex = star;
    get_objective_title(objective);
}

s32 bingo_objective_star_timed_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    enum CourseNum course;
    s32 star, maxTime;

    switch (class) {
        case BINGO_CLASS_MEDIUM:
        default:
            random_star_main_course_except_100c(&course, &star);
            maxTime = get_time_for_star(course, star) * 30 + (20 * 30) + // add 20 seconds to be nice
                      (RandomU16() % (15 * 30));                         // up to 15 extra seconds
            break;
        case BINGO_CLASS_HARD:
        case BINGO_CLASS_CENTER:
            random_star_main_course_except_100c(&course, &star);
            maxTime = (get_time_for_star(course, star) * 30)
                      + (RandomU16() % (15 * 30)); // up to 15 extra seconds
            break;
    }

    objective->icon = BINGO_ICON_STAR_TIMED;
    objective->data.starTimerObjective.course = course;
    objective->data.starTimerObjective.starIndex = star;
    objective->data.starTimerObjective.timer = 0;
    objective->data.starTimerObjective.maxTime = maxTime;
    get_objective_title(objective);
}

s32 bingo_objective_star_reverse_joystick_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    enum CourseNum course;
    s32 star;

    switch (class) {
        default:
            random_star_main_course_except_100c(&course, &star);
            break;
    }

    objective->icon = BINGO_ICON_STAR_REVERSE_JOYSTICK;
    objective->data.starObjective.course = course;
    objective->data.starObjective.starIndex = star;
    get_objective_title(objective);
}

s32 bingo_objective_star_green_demon_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    enum CourseNum course;
    s32 star;

    switch (class) {
        default:
            random_star_main_course_except_100c(&course, &star);
            break;
    }

    objective->icon = BINGO_ICON_STAR_GREEN_DEMON;
    objective->data.starObjective.course = course;
    objective->data.starObjective.starIndex = star;
    get_objective_title(objective);
}

s32 bingo_objective_star_click_game_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    enum CourseNum course;
    s32 star;
    s32 clicks, clickDiff;
    struct ClickInfo *clickInfo;

    do {
        random_star_main_course(&course, &star);
        clickInfo = get_click_info_for_star(course, star);
    } while (clickInfo->minClicks == CLICK_GAME_STAR_BANNED);

    if (clickInfo->maxClicks == CLICK_GAME_MAX_IS_MIN) {
        clicks = clickInfo->minClicks;
    } else {
        switch (class) {
            default:
                clickDiff = clickInfo->maxClicks - clickInfo->minClicks;
                clicks = clickInfo->minClicks + (RandomU16() % clickDiff) + 1;
                break;
            case BINGO_CLASS_HARD:
                clicks = clickInfo->minClicks;
                break;
        }
    }

    objective->icon = BINGO_ICON_STAR_CLICK_GAME;
    objective->data.starClicksObjective.course = course;
    objective->data.starClicksObjective.starIndex = star;
    objective->data.starClicksObjective.maxClicks = clicks;
    // not 0 due to bug where level entry counts as a click:
    objective->data.starClicksObjective.clicks = -1;
    get_objective_title(objective);
}

s32 bingo_objective_star_daredevil_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    enum CourseNum course;
    s32 star;

    // Some thoughts on Daredevil:
    //   - All stars are not much harder, except for...
    //   - DDD: It might be possible to get to the sub area,
    //          but I haven't managed to do it. Close, though.
    //          The Manta Ray heals you, so it's possible.
    //          Coins make Chests in the Current possible.
    //   - WDW: Getting to downtown seems impossible without
    //          glitches. This forbids 8 red coins and Quick Race.
    //          100 coins is possible (and fun).
    //   - JRB: Using the metal cap it's actually possible,
    //          though difficult, to get to the ocean cove.
    //          However, I haven't managed to get from the
    //          cove back to the surface (though close), which
    //          means only Treasure of the Ocean Cove, Blast to
    //          the Stone Pillar, and Through the Jet Stream seem
    //          possible. Can the Eel Come Out to Play might be
    //          possible - haven't tried yet - but I bet it'd be
    //          hard. Same with 100 coins. To be determined.
    switch (class) {
        default:
        case BINGO_CLASS_MEDIUM:
            do {
                random_star_main_course_except_100c(&course, &star);
            } while (
                (
                    course == COURSE_JRB
                    || (course == COURSE_DDD && (star != 1 || star != 4))
                    || (course == COURSE_WDW && (star == 4 || star == 5))
                )
            );
            break;
        case BINGO_CLASS_HARD:
            do {
                course = random_main_course();
            } while (course == COURSE_DDD);
            if (course == COURSE_JRB) {
                switch (RandomU16() % 3) {
                    case 0:
                        star = 2;
                    case 1:
                        star = 4;
                    case 2:
                        star = 5;
                }
            } else {
                star = 6;  // 100 coins
            }
            break;
    }

    objective->icon = BINGO_ICON_STAR_DAREDEVIL;
    objective->data.starObjective.course = course;
    objective->data.starObjective.starIndex = star;
    get_objective_title(objective);
}

s32 bingo_objective_coin_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    enum CourseNum course;
    s32 coins;

    switch (class) {
        default:
        case BINGO_CLASS_EASY:
            course = random_main_course();
            coins = ((RandomU16() % 30) + 30); // between 30 and 60
            break;
        case BINGO_CLASS_MEDIUM:
            course = random_main_course();
            coins = ((RandomU16() % 40) + 50); // between 50 and 99
            break;
        case BINGO_CLASS_CENTER:
        case BINGO_CLASS_HARD:
            course = random_main_course();
            coins = ((RandomU16() % 20) + 80); // between 80 and 99
            break;
    }
    objective->icon = BINGO_ICON_COIN;
    objective->data.courseCollectableData.course = course;
    objective->data.courseCollectableData.toGet = coins;
    objective->data.courseCollectableData.gotten = 0;
    get_objective_title(objective);
}

s32 bingo_objective_multicoin_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    s32 numCoins;
    switch (class) {
        default:
            numCoins = 100 + (RandomU16() % 151); // [100:250]
            break;
    }
    bingo_objective_collectable_init(objective, BINGO_ICON_MULTICOIN, numCoins);
}

s32 bingo_objective_multistar_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) { s32 numStars;
    switch (class) {
        case BINGO_CLASS_EASY:
            numStars = 3 + (RandomU16() % 2);    // 3 to 4
            break;
        case BINGO_CLASS_MEDIUM:
            numStars = 5 + (RandomU16() % 4);    // 5 to 8
            break;
        default:
        case BINGO_CLASS_HARD:
            numStars = 8 + (RandomU16() % 5);   // 8 to 12
            break;
    }
    bingo_objective_collectable_init(objective, BINGO_ICON_MULTISTAR, numStars);
}

s32 bingo_objective_1ups_in_level_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    enum CourseNum course;
    s32 _1ups;
    s32 _1upsMin;
    switch (class) {
        case BINGO_CLASS_MEDIUM:
        default:
            do {
                course = random_main_course();
                _1upsMin = (s32) get_1ups_in_level(course) * 0.6f;
                _1ups = (RandomU16() % (get_1ups_in_level(course) - _1upsMin)) + _1upsMin;
            } while (_1ups < 3);
            break;
        case BINGO_CLASS_HARD:
            course = random_main_course();
            _1ups = get_1ups_in_level(course);
            break;
    }

    objective->icon = BINGO_ICON_1UPS_IN_LEVEL;
    objective->data.courseCollectableData.course = course;
    objective->data.courseCollectableData.toGet = _1ups;
    objective->data.courseCollectableData.gotten = 0;
    get_objective_title(objective);
}

s32 bingo_objective_stars_in_level_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    enum CourseNum course;

    switch (class) {
        default:
            course = random_main_course();
            break;
    }

    objective->icon = BINGO_ICON_STARS_IN_LEVEL;
    objective->data.courseCollectableData.course = course;
    objective->data.courseCollectableData.toGet = 7;
    objective->data.courseCollectableData.gotten = 0;
    get_objective_title(objective);
}

s32 bingo_objective_lose_mario_hat_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    enum CourseNum course;

    switch (class) {
        default:
            break;
    }

    objective->icon = BINGO_ICON_MARIO_HAT;
    get_objective_title(objective);
}

s32 bingo_objective_blj_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    enum CourseNum course;

    switch (class) {
        default:
            break;
    }

    objective->icon = BINGO_ICON_BLJ;
    get_objective_title(objective);
}

s32 bingo_objective_bowser_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    enum LevelNum level;

    switch (class) {
        default:
            if (RandomU16() % 2 == 0) {
                level = LEVEL_BOWSER_1;
            } else {
                level = LEVEL_BOWSER_2;
            }
            break;
        case BINGO_CLASS_CENTER:
        case BINGO_CLASS_HARD:
            level = LEVEL_BOWSER_3;
            break;
    }

    objective->icon = BINGO_ICON_BOWSER;
    objective->data.levelData.level = level;
    get_objective_title(objective);
}

s32 bingo_objective_exclamation_mark_box_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    s32 boxes;
    switch (class) {
        default:
            boxes = 6 + (RandomU16() % 8); // between 6 and 14
            break;
    }

    bingo_objective_collectable_init(objective, BINGO_ICON_EXCLAMATION_MARK_BOX, boxes);
}

s32 bingo_objective_red_coin_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    s32 coins;
    switch (class) {
        default:
        case BINGO_CLASS_MEDIUM:
            coins = 12 + (RandomU16() % 7);   // 12 to 18
            break;
        case BINGO_CLASS_HARD:
            coins = 20 + (RandomU16() % 10);  // 20 to 29
            break;
    }

    bingo_objective_collectable_init(objective, BINGO_ICON_RED_COIN, coins);
}

s32 bingo_objective_kill_goombas_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    s32 enemiesToKill;

    switch (class) {
        case BINGO_CLASS_CENTER:
            enemiesToKill = (RandomU16() % 5) + 15;
            break;
        case BINGO_CLASS_MEDIUM:
        default:
            enemiesToKill = (RandomU16() % 10) + 5;
            break;
    }

    bingo_objective_collectable_init(objective, BINGO_ICON_KILL_GOOMBAS, enemiesToKill);
}

s32 bingo_objective_kill_bobombs_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    s32 enemiesToKill;

    switch (class) {
        case BINGO_CLASS_CENTER:
            enemiesToKill = (RandomU16() % 5) + 15;
            break;
        case BINGO_CLASS_MEDIUM:
        default:
            enemiesToKill = (RandomU16() % 10) + 5;
            break;
    }

    bingo_objective_collectable_init(objective, BINGO_ICON_KILL_BOBOMBS, enemiesToKill);
}

s32 bingo_objective_kill_spindrifts_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    s32 enemiesToKill;

    switch (class) {
        case BINGO_CLASS_MEDIUM:
        default:
            enemiesToKill = (RandomU16() % 8) + 9;  // between 9 and 16
            break;
    }

    bingo_objective_collectable_init(objective, BINGO_ICON_KILL_SPINDRIFTS, enemiesToKill);
}

s32 bingo_objective_kill_mr_is_init(
    struct BingoObjective *objective, enum BingoObjectiveClass class
) {
    s32 enemiesToKill;

    switch (class) {
        case BINGO_CLASS_MEDIUM:
        default:
            enemiesToKill = (RandomU16() % 4) + 3;  // between 3 and 6
            break;
    }

    bingo_objective_collectable_init(objective, BINGO_ICON_KILL_MR_IS, enemiesToKill);
}

////////////////////////////////////////////////////////////////////////////////

s32 bingo_objective_init(
    struct BingoObjective *objective,
    enum BingoObjectiveClass class,
    enum BingoObjectiveType type
) {
    objective->initialized = 1;
    objective->type = type;
    objective->class = class;
    switch (type) {
        case BINGO_OBJECTIVE_STAR:
            return bingo_objective_star_init(objective, class);
        case BINGO_OBJECTIVE_STAR_A_BUTTON_CHALLENGE:
            return bingo_objective_star_a_button_challenge_init(objective, class);
        case BINGO_OBJECTIVE_STAR_B_BUTTON_CHALLENGE:
            return bingo_objective_star_b_button_challenge_init(objective, class);
        case BINGO_OBJECTIVE_STAR_Z_BUTTON_CHALLENGE:
            return bingo_objective_star_z_button_challenge_init(objective, class);
        case BINGO_OBJECTIVE_STAR_TIMED:
            return bingo_objective_star_timed_init(objective, class);
        case BINGO_OBJECTIVE_STAR_REVERSE_JOYSTICK:
            return bingo_objective_star_reverse_joystick_init(objective, class);
        case BINGO_OBJECTIVE_STAR_GREEN_DEMON:
            return bingo_objective_star_green_demon_init(objective, class);
        case BINGO_OBJECTIVE_STAR_CLICK_GAME:
            return bingo_objective_star_click_game_init(objective, class);
        case BINGO_OBJECTIVE_STAR_DAREDEVIL:
            return bingo_objective_star_daredevil_init(objective, class);
        case BINGO_OBJECTIVE_COIN:
            return bingo_objective_coin_init(objective, class);
        case BINGO_OBJECTIVE_MULTICOIN:
            return bingo_objective_multicoin_init(objective, class);
        case BINGO_OBJECTIVE_MULTISTAR:
            return bingo_objective_multistar_init(objective, class);
        case BINGO_OBJECTIVE_1UPS_IN_LEVEL:
            return bingo_objective_1ups_in_level_init(objective, class);
        case BINGO_OBJECTIVE_STARS_IN_LEVEL:
            return bingo_objective_stars_in_level_init(objective, class);
        case BINGO_OBJECTIVE_LOSE_MARIO_HAT:
            return bingo_objective_lose_mario_hat_init(objective, class);
        case BINGO_OBJECTIVE_BLJ:
            return bingo_objective_blj_init(objective, class);
        case BINGO_OBJECTIVE_BOWSER:
            return bingo_objective_bowser_init(objective, class);
        case BINGO_OBJECTIVE_EXCLAMATION_MARK_BOX:
            return bingo_objective_exclamation_mark_box_init(objective, class);
        case BINGO_OBJECTIVE_RED_COIN:
            return bingo_objective_red_coin_init(objective, class);
        case BINGO_OBJECTIVE_KILL_GOOMBAS:
            return bingo_objective_kill_goombas_init(objective, class);
        case BINGO_OBJECTIVE_KILL_BOBOMBS:
            return bingo_objective_kill_bobombs_init(objective, class);
        case BINGO_OBJECTIVE_KILL_SPINDRIFTS:
            return bingo_objective_kill_spindrifts_init(objective, class);
        case BINGO_OBJECTIVE_KILL_MR_IS:
            return bingo_objective_kill_mr_is_init(objective, class);
    }
}
