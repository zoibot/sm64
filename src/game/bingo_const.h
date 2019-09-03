#ifndef _BINGO_CONST_H
#define _BINGO_CONST_H

#include <ultra64.h>

extern char *courseAbbreviations[15];
extern s32 course_1ups[15];
extern s32 possibleABC[32][2];
extern s32 starTimes[90][3];

s32 get_1ups_in_level(enum CourseNum course);
s32 get_time_for_star(enum CourseNum course, s32 star);

#endif /* _BINGO_CONST_H */
