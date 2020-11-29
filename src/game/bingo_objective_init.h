#ifndef _BINGO_OBJECTIVE_INIT_H
#define _BINGO_OBJECTIVE_INIT_H

#include <ultra64.h>
#include "area.h"
#include "bingo.h"

s32 bingo_objective_init(
    struct BingoObjective *objective,
    enum BingoObjectiveClass class,
    enum BingoObjectiveType type
);

#endif /* _BINGO_OBJECTIVE_INIT_H */
