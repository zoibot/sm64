#include <ultra64.h>
#include "bingo.h"

struct BingoObjectiveInfo {
    enum BingoObjectiveType type;
    enum BingoObjectiveIcon icon;
};

struct BingoObjectiveInfo *get_objective_info(enum BingoObjectiveType type);