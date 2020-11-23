#ifndef _BINGO_CONST_H
#define _BINGO_CONST_H

#include <ultra64.h>

#include "course_table.h"

struct ABCData {
    enum CourseNum course;
    s32 star;
    char hint[100];
};

extern char *courseAbbreviations[24];
extern struct ABCData possibleABC[];
extern s32 numPossibleABC;

s32 get_1ups_in_level(enum CourseNum course);
s32 get_time_for_star(enum CourseNum course, s32 star);

#endif /* _BINGO_CONST_H */
