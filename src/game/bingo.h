#ifndef _BINGO_H
#define _BINGO_H

#include "course_table.h"
#include "level_table.h"

// Global bingo state information
extern s32 gBingoInitialized;
extern u32 gBingoInitialSeed;
extern s64 gbGlobalBingoTimer;
extern s32 gbBingoTimerDisabled;
extern s32 gbBingoTarget;
extern s32 gbBingosCompleted;
extern s32 gbBingoShowCongratsCounter;
extern s32 gbBingoShowCongratsLimit;
extern s32 gbBingoShowTimer;

extern s16 gbStarIndex;
extern s32 gbCoinsJustGotten;
extern s32 gBingoReverseJoystickActive;
extern s32 gBingoClickGameActive;
extern s32 gBingoClickCounter;
extern s16 gBingoClickGamePrevCameraSettings;
extern s32 gBingoClickGamePrevCameraIndex;
extern s32 gBingoDaredevilActive;
extern s32 gBingoDaredevilPrevHealth;
extern s32 gStarSelectScreenActive;

enum BingoModifier {
    BINGO_MODIFIER_NONE = 0,
    BINGO_MODIFIER_GREEN_DEMON,
    BINGO_MODIFIER_REVERSE_JOYSTICK,
    BINGO_MODIFIER_CLICK_GAME,
    BINGO_MODIFIER_DAREDEVIL,
    BINGO_MODIFIER_MAX = BINGO_MODIFIER_DAREDEVIL,
    BINGO_STARS_TOTAL_AMOUNT = BINGO_MODIFIER_MAX + 1
};

extern enum BingoModifier gBingoStarSelected;

enum BingoObjectiveState
{
    BINGO_STATE_NONE,
    BINGO_STATE_COMPLETE,
    BINGO_STATE_FAILED_IN_THIS_COURSE
};

enum BingoObjectiveClass
{
    // Difficulty-based classes:
    BINGO_CLASS_EASY,
    BINGO_CLASS_MEDIUM,
    BINGO_CLASS_HARD,
    // Positional classes:
    BINGO_CLASS_CENTER
};

enum BingoObjectiveType
{
    BINGO_OBJECTIVE_TYPE_MIN,
    // Begin

    // Single stars:
    BINGO_OBJECTIVE_STAR_MIN = BINGO_OBJECTIVE_TYPE_MIN,
    BINGO_OBJECTIVE_STAR = BINGO_OBJECTIVE_STAR_MIN,
    BINGO_OBJECTIVE_STAR_TIMED,
    BINGO_OBJECTIVE_STAR_A_BUTTON_CHALLENGE,
    BINGO_OBJECTIVE_STAR_B_BUTTON_CHALLENGE,
    BINGO_OBJECTIVE_STAR_Z_BUTTON_CHALLENGE,
    // Single stars, game-modifying:
    BINGO_OBJECTIVE_STAR_CLICK_GAME,
    BINGO_OBJECTIVE_STAR_REVERSE_JOYSTICK,
    BINGO_OBJECTIVE_STAR_GREEN_DEMON,
    BINGO_OBJECTIVE_STAR_DAREDEVIL,
    BINGO_OBJECTIVE_STAR_MAX = BINGO_OBJECTIVE_STAR_DAREDEVIL,
    // Per level:
    BINGO_OBJECTIVE_COIN,
    BINGO_OBJECTIVE_1UPS_IN_LEVEL,
    BINGO_OBJECTIVE_STARS_IN_LEVEL,
    // One-offs (for now):
    BINGO_OBJECTIVE_LOSE_MARIO_HAT,
    BINGO_OBJECTIVE_BLJ,
    // Things that don't fit in any other category...
    BINGO_OBJECTIVE_BOWSER,
    // Collectables:
    BINGO_OBJECTIVE_COLLECTABLE_MIN,
    BINGO_OBJECTIVE_MULTICOIN = BINGO_OBJECTIVE_COLLECTABLE_MIN,
    BINGO_OBJECTIVE_MULTISTAR,
    BINGO_OBJECTIVE_RED_COIN,
    BINGO_OBJECTIVE_EXCLAMATION_MARK_BOX,
    BINGO_OBJECTIVE_KILL_GOOMBAS,
    BINGO_OBJECTIVE_KILL_BOBOMBS,
    BINGO_OBJECTIVE_KILL_SPINDRIFTS,
    BINGO_OBJECTIVE_KILL_MR_IS,
    BINGO_OBJECTIVE_COLLECTABLE_MAX = BINGO_OBJECTIVE_KILL_MR_IS,
    // End
    BINGO_OBJECTIVE_TYPE_MAX = BINGO_OBJECTIVE_COLLECTABLE_MAX,
    BINGO_OBJECTIVE_TOTAL_AMOUNT
};

enum BingoObjectiveIcon {
    // Don't mess with the order here.
    // It needs to be kept in sync with
    // bingo_lut in segment2.c.
    BINGO_ICON_FAILED,
    BINGO_ICON_SUCCESS,
    BINGO_ICON_STAR,
    BINGO_ICON_STAR_TIMED,
    BINGO_ICON_STAR_A_BUTTON_CHALLENGE,
    BINGO_ICON_STAR_B_BUTTON_CHALLENGE,
    BINGO_ICON_STAR_Z_BUTTON_CHALLENGE,
    BINGO_ICON_STAR_CLICK_GAME,
    BINGO_ICON_STAR_REVERSE_JOYSTICK,
    BINGO_ICON_STAR_GREEN_DEMON,
    BINGO_ICON_STAR_DAREDEVIL,
    BINGO_ICON_COIN,
    BINGO_ICON_MULTICOIN,
    BINGO_ICON_MULTISTAR,
    BINGO_ICON_1UPS_IN_LEVEL,
    BINGO_ICON_STARS_IN_LEVEL,
    BINGO_ICON_MARIO_HAT,
    BINGO_ICON_BLJ,
    BINGO_ICON_BOWSER,
    BINGO_ICON_RED_COIN,
    BINGO_ICON_EXCLAMATION_MARK_BOX,
    BINGO_ICON_KILL_GOOMBAS,
    BINGO_ICON_KILL_BOBOMBS,
    BINGO_ICON_KILL_SPINDRIFTS,
    BINGO_ICON_KILL_MR_IS
};

enum BingoObjectiveUpdate
{
    BINGO_UPDATE_STAR,
    BINGO_UPDATE_COIN,
    // Unique collectables (used by bingo_tracking_collectables)
    BINGO_COLLECTABLES_MIN,

    BINGO_UPDATE_KILLED_GOOMBA = BINGO_COLLECTABLES_MIN,
    BINGO_UPDATE_KILLED_BOBOMB,
    BINGO_UPDATE_KILLED_SPINDRIFT,
    BINGO_UPDATE_KILLED_MR_I,
    BINGO_UPDATE_RED_COIN,
    BINGO_UPDATE_EXCLAMATION_MARK_BOX,

    BINGO_COLLECTABLES_MAX = BINGO_UPDATE_EXCLAMATION_MARK_BOX,
    // End collectables (TODO: migrate away from Update enum?)
    BINGO_UPDATE_COURSE_CHANGED,
    BINGO_UPDATE_A_PRESSED,
    BINGO_UPDATE_B_PRESSED,
    BINGO_UPDATE_Z_PRESSED,
    BINGO_UPDATE_CAMERA_CLICK,
    BINGO_UPDATE_TIMER_FRAME,
    BINGO_UPDATE_GOT_1UP,
    BINGO_UPDATE_LOST_HAT,
    BINGO_UPDATE_BLJ,
    BINGO_UPDATE_BOWSER_KILLED
};


struct StarObjectiveData
{
    enum CourseNum course;
    s32 starIndex;
};

struct ABCStarObjectiveData
{
    enum CourseNum course;
    s32 starIndex;
    char *hint;
};

struct StarTimerObjectiveData
{
    enum CourseNum course;
    s32 starIndex;
    s32 maxTime;
    s32 timer;
};

struct StarClickCounterData // TODO: merge with timer objective
{
    enum CourseNum course;
    s32 starIndex;
    s32 maxClicks;
    s32 clicks;
};

struct CourseCollectableData
{
    enum CourseNum course;
    s32 toGet;
    s32 gotten;
};

struct CollectableData
{
    s32 toGet;
    s32 gotten;
};

struct LevelObjectiveData
{
    enum LevelNum level;
};

struct BingoObjective
{
    u8 initialized;  // marker for if this objective is all zeros
    enum BingoObjectiveType type;
    enum BingoObjectiveState state;
    enum BingoObjectiveIcon icon;
    enum BingoObjectiveClass class;  // useful during board setup only
    char title[30];
    union {
        struct StarObjectiveData starObjective;
        struct ABCStarObjectiveData abcStarObjective;
        struct StarTimerObjectiveData starTimerObjective;
        struct StarClickCounterData starClicksObjective;
        struct CourseCollectableData courseCollectableData;
        struct CollectableData collectableData;
        struct LevelObjectiveData levelData;
    } data;
};

extern struct BingoObjective gBingoObjectives[25];
extern u8 gBingoObjectivesDisabled[BINGO_OBJECTIVE_TOTAL_AMOUNT];

void set_objective_state(struct BingoObjective *objective, enum BingoObjectiveState state);
void bingo_update(enum BingoObjectiveUpdate update);

#endif /* _BINGO_H */
