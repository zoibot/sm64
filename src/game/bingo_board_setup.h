#ifndef _BINGO_BOARD_SETUP_H
#define _BINGO_BOARD_SETUP_H

#include <ultra64.h>
#include "area.h"

enum CourseNum random_course();
void random_abc(enum CourseNum *course, s32 *star);
void setup_bingo_objectives(u32);

#endif /* _BINGO_BOARD_SETUP_H */