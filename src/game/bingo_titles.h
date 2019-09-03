#ifndef _BINGO_TITLES_H
#define _BINGO_TITLES_H

#include <ultra64.h>
#include "area.h"
#include "bingo.h"

void get_course_abbreviation(enum CourseNum course, char *abbrev);

void get_objective_title(struct BingoObjective *objective);

#endif /* _BINGO_TITLES_H */