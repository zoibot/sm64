#include <ultra64.h>
#include <PR/os_libc.h>

#include "area.h"
#include "bingo.h"

#include "bingo_descriptions.h"
#include "bingo_const.h"
#include "bingo_objective_func.h"
#include "bingo_objective_init.h"
#include "segment2.h"
#include "strcpy.h"

u32 gbCourseStars[25] = { 0 };
u32 gbSecretStarFlags = 0;

void bingo_set_star(s16 course, s16 star) {
    if (course == -1) {
        gbSecretStarFlags |= (1 << star);
    } else {
        gbCourseStars[course] |= (1 << star);
    }
}
s32 bingo_get_course_count(enum CourseNum course) {
    s32 i;
    u32 flag = 1;
    s32 courseCount = 0;

    for (i = 0; i < 32; i++) {
        if (gbCourseStars[course - 1] & flag) {
            courseCount++;
        }
        flag <<= 1;
    }
    return courseCount;
}

s32 bingo_get_star_count(void) {
    u32 flag = 1;
    s32 course;
    s32 count = 0;

    // Get standard course star count.
    for (course = COURSE_MIN - 1; course < COURSE_MAX; course++) {
        count += bingo_get_course_count(course);
    }

    // castle stars
    for (flag = 1; flag != (1 << 31); flag <<= 1) {
        if (gbSecretStarFlags & flag) {
            count++;
        }
    }

    return count;
}

s32 bingo_get_castle_secret_star_count(void) {
    u32 flag = 1;
    s32 count = 0;
    s32 course;

    // castle stars
    for (flag = 1; flag != (1 << 31); flag <<= 1) {
        if (gbSecretStarFlags & flag) {
            count++;
        }
    }
    
    // bonus stages
    for (course = COURSE_BONUS_STAGES - 1; course < COURSE_MAX; course++) {
        count += bingo_get_course_count(course);
    }
    
    return count;
}

u8 bingo_get_course_flags(s16 course) {
    // TODO(kempster): if course is -1, return secret star count
    return gbCourseStars[course];
}
