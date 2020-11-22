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

void random_abc(enum CourseNum *course, s32 *star, char **hint) {
    u32 num_stars = sizeof(possibleABC) / sizeof(possibleABC[0]);
    u32 the_star = RandomU16() % num_stars;
    *course = possibleABC[the_star].course;
    *star = possibleABC[the_star].star - 1; // star is 0-indexed everywhere else :(
    *hint = possibleABC[the_star].hint;
}

enum CourseNum random_main_course() {
    return (RandomU16() % 15) + 1;
}

void random_star_main_course(enum CourseNum *course, s32 *star) {
    *course = random_main_course();
    *star = (RandomU16() % 7); // zero-indexed
}

void random_star_main_course_except_100c(enum CourseNum *course, s32 *star) {
    *course = random_main_course();
    *star = (RandomU16() % 6); // zero-indexed
}

void random_star_except_mips_toad(enum CourseNum *course, s32 *star) {
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

void random_bbc(enum CourseNum *course, s32 *star) {
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

void random_zbc(enum CourseNum *course, s32 *star) {
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

void ahhh_ahhhhh_oh_no_not_implemented(struct BingoObjective *objective) {
    // TODO: fill this in somehow
}

void bingo_objective_star_init(struct BingoObjective *objective, enum BingoObjectiveClass class) {
    enum CourseNum course;
    s32 star;

    switch (class) {
        case BINGO_CLASS_EASY:
        case BINGO_CLASS_MEDIUM:
            random_star_except_mips_toad(&course, &star);
            break;
        default:
            ahhh_ahhhhh_oh_no_not_implemented(objective);
            return;
    }
    strcpy(objective->icon, ICON_STAR);
    objective->data.starObjective.course = course;
    objective->data.starObjective.starIndex = star;
    get_objective_title(objective);
}

void bingo_objective_star_a_button_challenge_init(struct BingoObjective *objective,
                                                  enum BingoObjectiveClass class) {
    enum CourseNum course;
    s32 star;
    char *hint;

    switch (class) {
        case BINGO_CLASS_HARD:
            random_abc(&course, &star, &hint);
            break;
        default:
            ahhh_ahhhhh_oh_no_not_implemented(objective);
            return;
    }
    strcpy(objective->icon, ICON_A_BUTTON);
    objective->data.abcStarObjective.course = course;
    objective->data.abcStarObjective.starIndex = star;
    objective->data.abcStarObjective.hint = hint;
    get_objective_title(objective);
}

void bingo_objective_star_b_button_challenge_init(struct BingoObjective *objective,
                                                  enum BingoObjectiveClass class) {
    enum CourseNum course;
    s32 star;

    switch (class) {
        case BINGO_CLASS_MEDIUM:
            random_bbc(&course, &star);
            break;
        default:
            ahhh_ahhhhh_oh_no_not_implemented(objective);
            return;
    }
    strcpy(objective->icon, ICON_B_BUTTON);
    objective->data.starObjective.course = course;
    objective->data.starObjective.starIndex = star;
    get_objective_title(objective);
}

void bingo_objective_star_z_button_challenge_init(struct BingoObjective *objective,
                                                  enum BingoObjectiveClass class) {
    enum CourseNum course;
    s32 star;

    switch (class) {
        case BINGO_CLASS_MEDIUM:
            random_zbc(&course, &star);
            break;
        default:
            ahhh_ahhhhh_oh_no_not_implemented(objective);
            return;
    }

    strcpy(objective->icon, ICON_Z_BUTTON);
    objective->data.starObjective.course = course;
    objective->data.starObjective.starIndex = star;
    get_objective_title(objective);
}

void bingo_objective_star_timed_init(struct BingoObjective *objective, enum BingoObjectiveClass class) {
    enum CourseNum course;
    s32 star, maxTime;

    switch (class) {
        case BINGO_CLASS_MEDIUM:
            random_star_main_course_except_100c(&course, &star);
            maxTime = get_time_for_star(course, star) * 30 + (20 * 30) + // add 20 seconds to be nice
                      (RandomU16() % (15 * 30));                         // up to 15 extra seconds
            break;
        case BINGO_CLASS_HARD:
            random_star_main_course_except_100c(&course, &star);
            maxTime = (get_time_for_star(course, star) * 30)
                      + (RandomU16() % (15 * 30)); // up to 15 extra seconds
            break;
        default:
            ahhh_ahhhhh_oh_no_not_implemented(objective);
            return;
    }

    strcpy(objective->icon, ICON_TIMER);
    objective->data.starTimerObjective.course = course;
    objective->data.starTimerObjective.starIndex = star;
    objective->data.starTimerObjective.timer = 0;
    objective->data.starTimerObjective.maxTime = maxTime;
    get_objective_title(objective);
}

void bingo_objective_star_reverse_joystick_init(struct BingoObjective *objective,
                                                enum BingoObjectiveClass class) {
    enum CourseNum course;
    s32 star;

    switch (class) {
        case BINGO_CLASS_HARD:
            random_star_main_course_except_100c(&course, &star);
            break;
        default:
            ahhh_ahhhhh_oh_no_not_implemented(objective);
            return;
    }

    strcpy(objective->icon, ICON_JOYSTICK);
    objective->data.starObjective.course = course;
    objective->data.starObjective.starIndex = star;
    get_objective_title(objective);
}

void bingo_objective_star_green_demon_init(struct BingoObjective *objective,
                                           enum BingoObjectiveClass class) {
    enum CourseNum course;
    s32 star;

    switch (class) {
        case BINGO_CLASS_HARD:
            random_star_main_course_except_100c(&course, &star);
            break;
        default:
            ahhh_ahhhhh_oh_no_not_implemented(objective);
            return;
    }

    strcpy(objective->icon, ICON_GREENDEMON);
    objective->data.starObjective.course = course;
    objective->data.starObjective.starIndex = star;
    get_objective_title(objective);
}

void bingo_objective_coin_init(struct BingoObjective *objective, enum BingoObjectiveClass class) {
    enum CourseNum course;
    s32 coins;

    switch (class) {
        case BINGO_CLASS_EASY:
            course = random_main_course();
            coins = ((RandomU16() % 30) + 30); // between 30 and 60
            break;
        case BINGO_CLASS_CENTER:
            course = random_main_course();
            coins = ((RandomU16() % 20) + 80); // between 80 and 100
            break;
        case BINGO_CLASS_MEDIUM:
            course = random_main_course();
            coins = ((RandomU16() % 90) + 10); // between 10 and 100
            break;
        default:
            ahhh_ahhhhh_oh_no_not_implemented(objective);
            return;
    }
    strcpy(objective->icon, ICON_COIN);
    objective->data.courseCollectableData.course = course;
    objective->data.courseCollectableData.toGet = coins;
    objective->data.courseCollectableData.gotten = 0;
    get_objective_title(objective);
}

void bingo_objective_multicoin_init(struct BingoObjective *objective, enum BingoObjectiveClass class) {
    s32 numCoins;
    switch (class) {
        case BINGO_CLASS_HARD:
            numCoins = 200 + ((RandomU16() % 15) * 10); // [200:350:10]
            break;
        case BINGO_CLASS_CENTER:
            numCoins = 300 + ((RandomU16() % 25) * 10); // [300:550:10]
            break;
        default:
            ahhh_ahhhhh_oh_no_not_implemented(objective);
            return;
    }
    strcpy(objective->icon, ICON_MULTICOIN);
    objective->data.collectableData.toGet = numCoins;
    objective->data.collectableData.gotten = 0;
    get_objective_title(objective);
}

void bingo_objective_1ups_in_level_init(struct BingoObjective *objective,
                                        enum BingoObjectiveClass class) {
    enum CourseNum course;
    s32 _1ups;
    switch (class) {
        case BINGO_CLASS_HARD:
            course = random_main_course();
            _1ups = get_1ups_in_level(course);
            break;
        default:
            ahhh_ahhhhh_oh_no_not_implemented(objective);
            return;
    }

    strcpy(objective->icon, ICON_1UP);
    objective->data.courseCollectableData.course = course;
    objective->data.courseCollectableData.toGet = _1ups;
    objective->data.courseCollectableData.gotten = 0;
    get_objective_title(objective);
}

void bingo_objective_stars_in_level_init(struct BingoObjective *objective,
                                         enum BingoObjectiveClass class) {
    enum CourseNum course;

    switch (class) {
        case BINGO_CLASS_HARD:
            course = random_main_course();
            break;
        default:
            ahhh_ahhhhh_oh_no_not_implemented(objective);
            return;
    }

    strcpy(objective->icon, ICON_PURPLESTAR);
    objective->data.courseCollectableData.course = course;
    objective->data.courseCollectableData.toGet = 6;
    objective->data.courseCollectableData.gotten = 0;
    get_objective_title(objective);
}

void bingo_objective_exclamation_mark_box_init(struct BingoObjective *objective,
                                               enum BingoObjectiveClass class) {
    s32 boxes;
    switch (class) {
        case BINGO_CLASS_MEDIUM:
            boxes = 6 + (RandomU16() % 8); // between 6 and 14
            break;
        default:
            ahhh_ahhhhh_oh_no_not_implemented(objective);
            return;
    }

    strcpy(objective->icon, ICON_YELLOW_EXCLAMATION_MARK_BOX);
    objective->data.collectableData.toGet = boxes;
    objective->data.collectableData.gotten = 0;
    get_objective_title(objective);
}

void bingo_objective_kill_goombas_init(struct BingoObjective *objective,
                                       enum BingoObjectiveClass class) {
    s32 enemiesToKill;

    switch (class) {
        case BINGO_CLASS_CENTER:
            enemiesToKill = (RandomU16() % 5) + 15;
            break;
        case BINGO_CLASS_MEDIUM:
            enemiesToKill = (RandomU16() % 10) + 5;
            break;
        default:
            ahhh_ahhhhh_oh_no_not_implemented(objective);
            return;
    }

    strcpy(objective->icon, ICON_GOOMBA);
    objective->data.killEnemyObjective.enemiesToKill = enemiesToKill;
    objective->data.killEnemyObjective.enemiesKilled = 0;
    get_objective_title(objective);
}

void bingo_objective_kill_bobombs_init(struct BingoObjective *objective,
                                       enum BingoObjectiveClass class) {
    s32 enemiesToKill;

    switch (class) {
        case BINGO_CLASS_CENTER:
            enemiesToKill = (RandomU16() % 5) + 15;
            break;
        case BINGO_CLASS_MEDIUM:
            enemiesToKill = (RandomU16() % 10) + 5;
            break;
        default:
            ahhh_ahhhhh_oh_no_not_implemented(objective);
            return;
    }

    strcpy(objective->icon, ICON_BOBOMB);
    objective->data.killEnemyObjective.enemiesToKill = enemiesToKill;
    objective->data.killEnemyObjective.enemiesKilled = 0;
    get_objective_title(objective);
}

////////////////////////////////////////////////////////////////////////////////

void bingo_objective_init(struct BingoObjective *objective, enum BingoObjectiveClass class,
                          enum BingoObjectiveType type) {
    // Write down the type. We use it a lot now.
    objective->type = type;

    // Figure out everything else about it.
    switch (type) {
        case BINGO_OBJECTIVE_STAR:
            bingo_objective_star_init(objective, class);
            break;
        case BINGO_OBJECTIVE_STAR_A_BUTTON_CHALLENGE:
            bingo_objective_star_a_button_challenge_init(objective, class);
            break;
        case BINGO_OBJECTIVE_STAR_B_BUTTON_CHALLENGE:
            bingo_objective_star_b_button_challenge_init(objective, class);
            break;
        case BINGO_OBJECTIVE_STAR_Z_BUTTON_CHALLENGE:
            bingo_objective_star_z_button_challenge_init(objective, class);
            break;
        case BINGO_OBJECTIVE_STAR_TIMED:
            bingo_objective_star_timed_init(objective, class);
            break;
        case BINGO_OBJECTIVE_STAR_REVERSE_JOYSTICK:
            bingo_objective_star_reverse_joystick_init(objective, class);
            break;
        case BINGO_OBJECTIVE_STAR_GREEN_DEMON:
            bingo_objective_star_green_demon_init(objective, class);
            break;
        case BINGO_OBJECTIVE_COIN:
            bingo_objective_coin_init(objective, class);
            break;
        case BINGO_OBJECTIVE_MULTICOIN:
            bingo_objective_multicoin_init(objective, class);
            break;
        case BINGO_OBJECTIVE_1UPS_IN_LEVEL:
            bingo_objective_1ups_in_level_init(objective, class);
            break;
        case BINGO_OBJECTIVE_STARS_IN_LEVEL:
            bingo_objective_stars_in_level_init(objective, class);
            break;
        case BINGO_OBJECTIVE_EXCLAMATION_MARK_BOX:
            bingo_objective_exclamation_mark_box_init(objective, class);
            break;
        case BINGO_OBJECTIVE_KILL_GOOMBAS:
            bingo_objective_kill_goombas_init(objective, class);
            break;
        case BINGO_OBJECTIVE_KILL_BOBOMBS:
            bingo_objective_kill_bobombs_init(objective, class);
            break;
    }
}
