#ifndef _BINGO_DESCRIPTIONS_H
#define _BINGO_DESCRIPTIONS_H

#include <ultra64.h>
#include "bingo.h"

char reverse_encode(u8 c);
void reverse_encode_str(u8 *str, char *buf);
void get_act_name(char *buf, enum CourseNum course, s32 star);
void get_level_name(char *buf, enum CourseNum course);
void getTimeFmtTiny(char *buf, int timestamp);
void getTimeFmt(char *buf, s32 timestamp);
void getTimeFmtPrecise(char *buf, s32 timestamp);

void describe_objective(struct BingoObjective *obj, char*);

#endif /* _BINGO_DESCRIPTIONS_H */
