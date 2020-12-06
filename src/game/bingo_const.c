#include <ultra64.h>
#include "area.h"
#include "bingo_const.h"

// TODO: This should be a hash-map or something
char *courseAbbreviations[24] = { "BOB",   "WF",   "JRB", "CCM",   "BBH",   "HMC",   "LLL",   "SSL",
                                  "DDD",   "SL",   "WDW", "TTM",   "THI",   "TTC",   "RR",    "BitDW",
                                  "BitFS", "BitS", "PSS", "CotMC", "TotWC", "VCutM", "WMotR", "SA" };

s32 course_1ups[15] = {
    3,  // COURSE_BOB
    4,  // COURSE_WF
    2,  // COURSE_JRB
    5,  // COURSE_CCM, without the impossible 1up
    2,  // COURSE_BBH
    4,  // COURSE_HMC
    8,  // COURSE_LLL
    9,  // COURSE_SSL
    1,  // COURSE_DDD
    4,  // COURSE_SL
    4,  // COURSE_WDW
    11, // COURSE_TTM
    10, // COURSE_THI
    4,  // COURSE_TTC
    9,  // COURSE_RR
};

s32 get_1ups_in_level(enum CourseNum course) {
    return course_1ups[course - 1];
}

// https://docs.google.com/spreadsheets/d/1MDgm0yj5IJeqYMJlOdbKyIz66aX1Y3dYm79_wIePxY0/edit#gid=0
// https://www.youtube.com/playlist?list=PLP0jCPw9IPWhdWX4Ae24hjdX2CH15GKTg

struct ABCData possibleABC[] = {
    { COURSE_BOB, 1, "" },
    { COURSE_BOB, 2, "" },
    // BOB6 is possible but 11 minutes long, requiring extensive cloning
    { COURSE_WF, 3, "Don't forget the warp." },
    { COURSE_WF, 4, "Don't forget the warp." },
    { COURSE_WF, 6, "Warp, then cannonless..." },
    // JRB1 is possible but 12 minutes long and very precise
    { COURSE_JRB, 2,
      "Use the B button underwater to go slightly faster." }, // pretty hard and quite boring...
    { COURSE_CCM, 1, "" },
    { COURSE_CCM, 2, "" },
    { COURSE_CCM, 3, "" },
    // CCM4 is possible but there is a hard bounce
    { COURSE_CCM, 5, "" },
    // BBH1 is possible but requires VSC
    { COURSE_HMC, 1, "Clip through a wall under an elevator with a zoomed-out camera." },
    { COURSE_LLL, 1, "" },
    { COURSE_LLL, 2, "" },
    { COURSE_LLL, 3, "" },
    { COURSE_LLL, 4, "" },
    // SSL1 is really precise. Gonna kill it. Requires Tweester
    { COURSE_SSL, 2, "" },
    // DDD5 is possible but super hard
    { COURSE_SL, 2, "" },
    // SL3 (In the Deep Freeze) is super hard actually, whoops
    { COURSE_SL, 4, "" },
    { COURSE_SL, 5, "" },
    // SL6 is possible but requires lots of Spindrift RNG
    { COURSE_WDW, 1, "Teleport to success." },
    // WDW3 is possible but totally RNG (for getting past Chuckya)
    // TTM1 is possible but 11 mins long and requires a HOLP placement
    { COURSE_TTM, 2, "You may have to framewalk or pause buffer to get past the rolling log." },
    { COURSE_TTM, 3, "Use a Fly Guy to get past the rolling log." },
    { COURSE_TTM, 4, "A precise dive can help you skip the slide." },
    { COURSE_TTM, 5, "You may have to framewalk or pause buffer to get past the rolling log." },
    { COURSE_TTM, 6, "Use the Crazy Box to get to the mushroom." },
    { COURSE_THI, 1, "Enter Huge Island and use the shell." },
    { COURSE_THI, 2, "Enter Huge Island and use the shell." },
    { COURSE_THI, 3, "Enter Huge Island and use the shell." },
    // THI4 is possible but borderline too hard (need fly guy bounce to get in pipe)
    { COURSE_THI, 6,
      "Enter Huge Island, use the shell, "
      "and use Chuckya to clip into the mountain." }
};

s32 numPossibleABC = sizeof(possibleABC) / sizeof(possibleABC[0]);

s32 starTimes[90][3] = {
    { COURSE_BOB, 1, 66 }, { COURSE_BOB, 2, 127 }, { COURSE_BOB, 3, 29 }, { COURSE_BOB, 4, 69 },
    { COURSE_BOB, 5, 46 }, { COURSE_BOB, 6, 19 },  { COURSE_WF, 1, 45 },  { COURSE_WF, 2, 21 },
    { COURSE_WF, 3, 14 },  { COURSE_WF, 4, 39 },   { COURSE_WF, 5, 17 },  { COURSE_WF, 6, 19 },
    { COURSE_JRB, 1, 63 }, { COURSE_JRB, 2, 50 },  { COURSE_JRB, 3, 56 }, { COURSE_JRB, 4, 80 },
    { COURSE_JRB, 5, 16 }, { COURSE_JRB, 6, 29 },  { COURSE_CCM, 1, 37 }, { COURSE_CCM, 2, 28 },
    { COURSE_CCM, 3, 69 }, { COURSE_CCM, 4, 56 },  { COURSE_CCM, 5, 55 }, { COURSE_CCM, 6, 13 },
    { COURSE_BBH, 1, 78 }, { COURSE_BBH, 2, 50 },  { COURSE_BBH, 3, 17 }, { COURSE_BBH, 4, 78 },
    { COURSE_BBH, 5, 39 }, { COURSE_BBH, 6, 40 },  { COURSE_HMC, 1, 23 }, { COURSE_HMC, 2, 68 },
    { COURSE_HMC, 3, 37 }, { COURSE_HMC, 4, 50 },  { COURSE_HMC, 5, 17 }, { COURSE_HMC, 6, 23 },
    { COURSE_LLL, 1, 28 }, { COURSE_LLL, 2, 37 },  { COURSE_LLL, 3, 21 }, { COURSE_LLL, 4, 17 },
    { COURSE_LLL, 5, 26 }, { COURSE_LLL, 6, 28 },  { COURSE_SSL, 1, 25 }, { COURSE_SSL, 2, 10 },
    { COURSE_SSL, 3, 120 }, { COURSE_SSL, 4, 160 },  { COURSE_SSL, 5, 125 }, { COURSE_SSL, 6, 240 },
    { COURSE_DDD, 1, 49 }, { COURSE_DDD, 2, 45 },  { COURSE_DDD, 3, 150 }, { COURSE_DDD, 4, 53 },
    { COURSE_DDD, 5, 21 }, { COURSE_DDD, 6, 63 },  { COURSE_SL, 1, 14 },  { COURSE_SL, 2, 19 },
    { COURSE_SL, 3, 8 },   { COURSE_SL, 4, 20 },   { COURSE_SL, 5, 46 },  { COURSE_SL, 6, 25 },
    { COURSE_WDW, 1, 20 }, { COURSE_WDW, 2, 22 },  { COURSE_WDW, 3, 49 }, { COURSE_WDW, 4, 28 },
    { COURSE_WDW, 5, 87 }, { COURSE_WDW, 6, 62 },  { COURSE_TTM, 1, 35 }, { COURSE_TTM, 2, 52 },
    { COURSE_TTM, 3, 35 }, { COURSE_TTM, 4, 11 },  { COURSE_TTM, 5, 29 }, { COURSE_TTM, 6, 16 },
    { COURSE_THI, 1, 35 }, { COURSE_THI, 2, 30 },  { COURSE_THI, 3, 71 }, { COURSE_THI, 4, 28 },
    { COURSE_THI, 5, 41 }, { COURSE_THI, 6, 63 },  { COURSE_TTC, 1, 20 }, { COURSE_TTC, 2, 25 },
    { COURSE_TTC, 3, 20 }, { COURSE_TTC, 4, 65 },  { COURSE_TTC, 5, 30 }, { COURSE_TTC, 6, 40 },
    { COURSE_RR, 1, 28 },  { COURSE_RR, 2, 170 },  { COURSE_RR, 3, 33 },  { COURSE_RR, 4, 20 },
    { COURSE_RR, 5, 19 },  { COURSE_RR, 6, 70 }
};

s32 get_time_for_star(enum CourseNum course, s32 star) {
    return starTimes[(course - 1) * 6 + star][2];
}
