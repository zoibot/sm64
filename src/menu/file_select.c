#include <ultra64.h>
#include <PR/os_libc.h>

#include "sm64.h"
#include "audio/external.h"
#include "game/game.h"
#include "game/ingame_menu.h"
#include "game/object_helpers.h"
#include "game/area.h"
#include "game/save_file.h"
#include "game/spawn_object.h"
#include "game/object_list_processor.h"
#include "game/segment2.h"
#include "game/segment7.h"
#include "game/print.h"
#include "engine/behavior_script.h"
#include "engine/graph_node.h"
#include "engine/math_util.h"
#include "behavior_data.h"
#include "text_strings.h"
#include "file_select.h"
#include "dialog_ids.h"
#include "game/bingo.h"
#include "game/bingo_board_setup.h"
#include "game/strcpy.h"

/**
 * @file file_select.c
 * This file implements how the file select and it's menus render and function.
 * That includes button IDs rendered as object models, strings, hand cursor,
 * special menu messages and phases, button states and button clicked checks.
 */

// The current sound mode is automatically centered on US due to
// the large length difference between options.
#ifndef VERSION_JP
static s16 sSoundTextX;
#endif

// Amount of main menu buttons defined in the code called by spawn_object_rel_with_rot.
// See file_select.h for the names in MenuButtonTypes.
static struct Object *sMainMenuButtons[35];

// The button that is selected when it is clicked.
static s8 sSelectedButtonID = MENU_BUTTON_NONE;

// Whether we are on the main menu or one of the submenus.
static s8 sCurrentMenuLevel = MENU_LAYER_MAIN;

// Used for text opacifying. If it is below 250, it is constantly incremented.
static u8 sTextBaseAlpha = 0;

// 2D position of the cursor on the screen.
// sCursorPos[0]: X | sCursorPos[1]: Y
static f32 sCursorPos[] = {0, 0};

// Determines which graphic to use for the cursor.
static s16 sCursorClickingTimer = 0;

// Equal to sCursorPos if the cursor gets clicked, {-10000, -10000} otherwise.
static s16 sClickPos[] = {-10000, -10000};

// Whether to fade out text or not.
static s8 sFadeOutText = FALSE;

// Used for text fading. The alpha value of text is calculated as
// sTextBaseAlpha - sTextFadeAlpha.
static u8 sTextFadeAlpha = 0;

// File select timer that keeps counting until it reaches 1000.
// Used to prevent buttons from being clickable as soon as a menu loads.
// Gets reset when you click an empty save, existing saves in copy and erase menus
// and when you click yes/no in the erase confirmation prompt.
static s16 sMainMenuTimer = 0;

// Sound mode menu buttonID, has different values compared to gSoundMode in audio.
// 0: gSoundMode = 0 (Stereo) | 1: gSoundMode = 3 (Mono) | 2: gSoundMode = 1 (Headset)
static s8 sSoundMode = 0;

// Defines the value of the save slot selected in the menu.
// Mario A: 1 | Mario B: 2 | Mario C: 3 | Mario D: 4
static s8 sSelectedFileNum = 0;

// Which coin score mode to use when scoring files. 0 for local
// coin high score, 1 for high score across all files.
static s8 sScoreFileCoinScoreMode = 0;

static unsigned char textReturn[] = { TEXT_RETURN };
static unsigned char textMarioA[] = { TEXT_FILE_MARIO_A };
static unsigned char textMarioB[] = { TEXT_FILE_MARIO_B };
static unsigned char textMarioC[] = { TEXT_FILE_MARIO_C };
static unsigned char textMarioD[] = { TEXT_FILE_MARIO_D };
static unsigned char textNew[] = { TEXT_NEW };
static unsigned char starIcon[] = { GLYPH_STAR, GLYPH_SPACE };
static unsigned char xIcon[] = { GLYPH_MULTIPLY, GLYPH_SPACE };
static unsigned char textSelectFile[] = { TEXT_SELECT_FILE };
static unsigned char textSeeds[] = { TEXT_SEEDS };
static unsigned char textReset[] = { TEXT_RESET };
static unsigned char text123[] = { TEXT_123 };
static unsigned char text456[] = { TEXT_456 };
static unsigned char text789[] = { TEXT_789 };
static unsigned char text_0_[] = { TEXT__0_ };
static unsigned char textRandom[] = { TEXT_RANDOM };
static unsigned char textOption[] = { TEXT_OPTION };
static unsigned char textOff[] = { TEXT_OFF };
static unsigned char textOn[] = { TEXT_ON };


s32 gBingoSeedIsSet = 0;
// We can support seeds up to 4,294,967,295, but since this is a weird number,
// we cap it at 999,999,999, which is 9 digits long. "RANDOM" is 6 characters
// long, so:
u8 gBingoSeedRandomText[] = { TEXT_RANDOM, 0xFF, 0xFF, 0xFF };
u8 gBingoSeedText[] = { TEXT_RANDOM, 0xFF, 0xFF, 0xFF };

s32 sBingoOptionSelection = 0;
#define BINGO_OPTIONS_PER_PAGE 11
s32 sBingoOptionSelectTimer = 0;
#define BINGO_OPTION_TIMER_FRAMES 3
s32 sToggleCurrentOption = 0;


void seed_reset(void) {
    s32 i;
    gBingoSeedIsSet = 0;
    for (i = 0; i < 10; i++) {
        gBingoSeedText[i] = gBingoSeedRandomText[i];
    }
}

/**
 * Yellow Background Menu Initial Action
 * Rotates the background at 180 grades and it's scale.
 * Although the scale is properly applied in the loop function.
 */
void beh_yellow_background_menu_init(void) {
    gCurrentObject->oFaceAngleYaw = 0x8000;
    gCurrentObject->oMenuButtonScale = 9.0f;
}

/**
 * Yellow Background Menu Loop Action
 * Properly scales the background in the main menu.
 */
void beh_yellow_background_menu_loop(void) {
    obj_scale(9.0f);
}

/**
 * Check if a button was clicked.
 * depth = 200.0 for main menu, 22.0 for submenus.
 */
static s32 check_clicked_button(s16 x, s16 y, f32 depth) {
    f32 a = 52.4213;
    f32 newX = ((f32) x * 160.0) / (a * depth);
    f32 newY = ((f32) y * 120.0) / (a * 3.0f / 4.0f * depth);
    s16 maxX = newX + 25.0f;
    s16 minX = newX - 25.0f;
    s16 maxY = newY + 21.0f;
    s16 minY = newY - 21.0f;

    if (sClickPos[0] < maxX && minX < sClickPos[0] && sClickPos[1] < maxY && minY < sClickPos[1]) {
        return TRUE;
    }
    return FALSE;
}

/**
 * Grow from main menu, used by selecting files and menus.
 */
static void bhv_menu_button_growing_from_main_menu(struct Object *button) {
    if (button->oMenuButtonTimer < 16) {
        button->oFaceAngleYaw += 0x800;
    }
    if (button->oMenuButtonTimer < 8) {
        button->oFaceAnglePitch += 0x800;
    }
    if (button->oMenuButtonTimer >= 8 && button->oMenuButtonTimer < 16) {
        button->oFaceAnglePitch -= 0x800;
    }
    button->oParentRelativePosX -= button->oMenuButtonOrigPosX / 16.0;
    button->oParentRelativePosY -= button->oMenuButtonOrigPosY / 16.0;
    if (button->oPosZ < button->oMenuButtonOrigPosZ + 17800.0) {
        button->oParentRelativePosZ += 1112.5;
    }
    button->oMenuButtonTimer++;
    if (button->oMenuButtonTimer == 16) {
        button->oParentRelativePosX = 0.0f;
        button->oParentRelativePosY = 0.0f;
        button->oMenuButtonState = MENU_BUTTON_STATE_FULLSCREEN;
        button->oMenuButtonTimer = 0;
    }
}

/**
 * Shrink back to main menu, used to return back while inside menus.
 */
static void bhv_menu_button_shrinking_to_main_menu(struct Object *button) {
    if (button->oMenuButtonTimer < 16) {
        button->oFaceAngleYaw -= 0x800;
    }
    if (button->oMenuButtonTimer < 8) {
        button->oFaceAnglePitch -= 0x800;
    }
    if (button->oMenuButtonTimer >= 8 && button->oMenuButtonTimer < 16) {
        button->oFaceAnglePitch += 0x800;
    }
    button->oParentRelativePosX += button->oMenuButtonOrigPosX / 16.0;
    button->oParentRelativePosY += button->oMenuButtonOrigPosY / 16.0;
    if (button->oPosZ > button->oMenuButtonOrigPosZ) {
        button->oParentRelativePosZ -= 1112.5;
    }
    button->oMenuButtonTimer++;
    if (button->oMenuButtonTimer == 16) {
        button->oParentRelativePosX = button->oMenuButtonOrigPosX;
        button->oParentRelativePosY = button->oMenuButtonOrigPosY;
        button->oMenuButtonState = MENU_BUTTON_STATE_DEFAULT;
        button->oMenuButtonTimer = 0;
    }
}

/**
 * Grow from submenu, used by selecting a file in the score menu.
 */
static void bhv_menu_button_growing_from_submenu(struct Object *button) {
    if (button->oMenuButtonTimer < 16) {
        button->oFaceAngleYaw += 0x800;
    }
    if (button->oMenuButtonTimer < 8) {
        button->oFaceAnglePitch += 0x800;
    }
    if (button->oMenuButtonTimer >= 8 && button->oMenuButtonTimer < 16) {
        button->oFaceAnglePitch -= 0x800;
    }
    button->oParentRelativePosX -= button->oMenuButtonOrigPosX / 16.0;
    button->oParentRelativePosY -= button->oMenuButtonOrigPosY / 16.0;
    button->oParentRelativePosZ -= 116.25;
    button->oMenuButtonTimer++;
    if (button->oMenuButtonTimer == 16) {
        button->oParentRelativePosX = 0.0f;
        button->oParentRelativePosY = 0.0f;
        button->oMenuButtonState = MENU_BUTTON_STATE_FULLSCREEN;
        button->oMenuButtonTimer = 0;
    }
}

/**
 * Shrink back to submenu, used to return back while inside a score save menu.
 */
static void bhv_menu_button_shrinking_to_submenu(struct Object *button) {
    if (button->oMenuButtonTimer < 16) {
        button->oFaceAngleYaw -= 0x800;
    }
    if (button->oMenuButtonTimer < 8) {
        button->oFaceAnglePitch -= 0x800;
    }
    if (button->oMenuButtonTimer >= 8 && button->oMenuButtonTimer < 16) {
        button->oFaceAnglePitch += 0x800;
    }
    button->oParentRelativePosX += button->oMenuButtonOrigPosX / 16.0;
    button->oParentRelativePosY += button->oMenuButtonOrigPosY / 16.0;
    if (button->oPosZ > button->oMenuButtonOrigPosZ) {
        button->oParentRelativePosZ += 116.25;
    }
    button->oMenuButtonTimer++;
    if (button->oMenuButtonTimer == 16) {
        button->oParentRelativePosX = button->oMenuButtonOrigPosX;
        button->oParentRelativePosY = button->oMenuButtonOrigPosY;
        button->oMenuButtonState = MENU_BUTTON_STATE_DEFAULT;
        button->oMenuButtonTimer = 0;
    }
}

/**
 * A small increase and decrease in size.
 * Used by failed copy/erase/score operations and sound mode select.
 */
static void bhv_menu_button_zoom_in_out(struct Object *button) {
    if (sCurrentMenuLevel == MENU_LAYER_MAIN) {
        if (button->oMenuButtonTimer < 4) {
            button->oParentRelativePosZ -= 20.0f;
        }
        if (button->oMenuButtonTimer >= 4) {
            button->oParentRelativePosZ += 20.0f;
        }
    } else {
        if (button->oMenuButtonTimer < 4) {
            button->oParentRelativePosZ += 20.0f;
        }
        if (button->oMenuButtonTimer >= 4) {
            button->oParentRelativePosZ -= 20.0f;
        }
    }
    button->oMenuButtonTimer++;
    if (button->oMenuButtonTimer == 8) {
        button->oMenuButtonState = MENU_BUTTON_STATE_DEFAULT;
        button->oMenuButtonTimer = 0;
    }
}

/**
 * A small temporary increase in size.
 * Used while selecting a target copy/erase file or yes/no erase confirmation prompt.
 */
static void bhv_menu_button_zoom_in(struct Object *button) {
    button->oMenuButtonScale += 0.0022;
    button->oMenuButtonTimer++;
    if (button->oMenuButtonTimer == 10) {
        button->oMenuButtonState = MENU_BUTTON_STATE_DEFAULT;
        button->oMenuButtonTimer = 0;
    }
}

/**
 * A small temporary decrease in size.
 * Used after selecting a target copy/erase file or
 * yes/no erase confirmation prompt to undo the zoom in.
 */
static void bhv_menu_button_zoom_out(struct Object *button) {
    button->oMenuButtonScale -= 0.0022;
    button->oMenuButtonTimer++;
    if (button->oMenuButtonTimer == 10) {
        button->oMenuButtonState = MENU_BUTTON_STATE_DEFAULT;
        button->oMenuButtonTimer = 0;
    }
}

/**
 * Menu Buttons Menu Initial Action
 * Aligns menu buttons so they can stay in their original
 * positions when you choose a button.
 */
void bhv_menu_button_init(void) {
    gCurrentObject->oMenuButtonOrigPosX = gCurrentObject->oParentRelativePosX;
    gCurrentObject->oMenuButtonOrigPosY = gCurrentObject->oParentRelativePosY;
}

/**
 * Menu Buttons Menu Loop Action
 * Handles the functions of the button states and
 * object scale for each button.
 */
void bhv_menu_button_loop(void) {
    switch (gCurrentObject->oMenuButtonState) {
        case MENU_BUTTON_STATE_DEFAULT: // Button state
            gCurrentObject->oMenuButtonOrigPosZ = gCurrentObject->oPosZ;
            break;
        case MENU_BUTTON_STATE_GROWING: // Switching from button to menu state
            if (sCurrentMenuLevel == MENU_LAYER_MAIN) {
                bhv_menu_button_growing_from_main_menu(gCurrentObject);
            }
            if (sCurrentMenuLevel == MENU_LAYER_SUBMENU) {
                bhv_menu_button_growing_from_submenu(gCurrentObject); // Only used for score files
            }
            sTextBaseAlpha = 0;
            sCursorClickingTimer = 4;
            break;
        case MENU_BUTTON_STATE_FULLSCREEN: // Menu state
            break;
        case MENU_BUTTON_STATE_SHRINKING: // Switching from menu to button state
            if (sCurrentMenuLevel == MENU_LAYER_MAIN) {
                bhv_menu_button_shrinking_to_main_menu(gCurrentObject);
            }
            if (sCurrentMenuLevel == MENU_LAYER_SUBMENU) {
                bhv_menu_button_shrinking_to_submenu(gCurrentObject); // Only used for score files
            }
            sTextBaseAlpha = 0;
            sCursorClickingTimer = 4;
            break;
        case MENU_BUTTON_STATE_ZOOM_IN_OUT:
            bhv_menu_button_zoom_in_out(gCurrentObject);
            sCursorClickingTimer = 4;
            break;
        case MENU_BUTTON_STATE_ZOOM_IN:
            bhv_menu_button_zoom_in(gCurrentObject);
            sCursorClickingTimer = 4;
            break;
        case MENU_BUTTON_STATE_ZOOM_OUT:
            bhv_menu_button_zoom_out(gCurrentObject);
            sCursorClickingTimer = 4;
            break;
    }
    obj_scale(gCurrentObject->oMenuButtonScale);
}

/**
 * Handles how to exit the score file menu using button states.
 */
static void exit_score_file_to_score_menu(struct Object *scoreFileButton, s8 scoreButtonID) {
    // Begin exit
    if (scoreFileButton->oMenuButtonState == MENU_BUTTON_STATE_FULLSCREEN
        && sCursorClickingTimer == 2) {
        play_sound(SOUND_MENU_CAMERA_ZOOM_OUT, gDefaultSoundArgs);
        scoreFileButton->oMenuButtonState = MENU_BUTTON_STATE_SHRINKING;
    }
    // End exit
    if (scoreFileButton->oMenuButtonState == MENU_BUTTON_STATE_DEFAULT) {
        sSelectedButtonID = scoreButtonID;
        if (sCurrentMenuLevel == MENU_LAYER_SUBMENU) {
            sCurrentMenuLevel = MENU_LAYER_MAIN;
        }
    }
}

static s32 seed_keypad_get_number(void) {
    s32 x = sClickPos[0];
    s32 y = sClickPos[1];
    s32 col = -1;
    s32 row = -1;
    s32 keypad[3][3] = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };

    if (x >= -33 && x <= -15) {
        col = 0;
    } else if (x >= -6 && x <= 12) {
        col = 1;
    } else if (x >= 21 && x <= 39) {
        col = 2;
    }

    if (col == -1) {
        return -1;
    }

    if (y <= 25 && y >= 7) {
        row = 0;
    } else if (y <= -2 && y >= -20) {
        row = 1;
    } else if (y <= -38 && y >= -47) {
        row = 2;
    } else if (y <= -68 && y >= -81) {
        if (col == 1) {
            return 0;
        } else {
            return -1;
        }
    }

    if (row == -1) {
        return -1;
    }

    return keypad[row][col];
}

static void seed_push_key(s32 key) {
    s32 i;
    if (!gBingoSeedIsSet) {
        // First keypress
        for (i = 0; i < 9; i++) {
            gBingoSeedText[i] = 0x00;
        }
        gBingoSeedIsSet = 1;
    }
    // Shift everything by 1 and add key to back
    for (i = 0; i < (9 - 1); i++) {
        gBingoSeedText[i] = gBingoSeedText[i + 1];
    }
    gBingoSeedText[8] = key;
}

static void seed_menu_check_clicked_buttons() {
    int buttonId;
    s32 keyEntered;

    // if (seedButton->oMenuButtonState == MENU_BUTTON_STATE_FULLSCREEN) {
        // code that's half copy-pasted, half ad-hoc fixed makes you look like
        // a MANIAC tbh
        for (buttonId = MENU_BUTTON_SEED_MIN; buttonId < MENU_BUTTON_SEED_MAX; buttonId++) {
            s16 buttonX = sMainMenuButtons[buttonId]->oPosX;
            s16 buttonY = sMainMenuButtons[buttonId]->oPosY;

            if (check_clicked_button(buttonX, buttonY, 200.0f) == TRUE) {
                if (buttonId == MENU_BUTTON_SEED_RESET) {
                    seed_reset();
                } else if (buttonId == MENU_BUTTON_SEED_OPTION) {
                    // sCurrentMenuLevel = MENU_LAYER_SUBMENU;
                    // TODO: fix two sounds
                    play_sound(SOUND_MENU_CAMERA_ZOOM_IN, gDefaultSoundArgs);
                    sMainMenuButtons[buttonId]->oMenuButtonState = MENU_BUTTON_STATE_GROWING;
                    sSelectedButtonID = buttonId;
                }
                break;
            }
        }
        keyEntered = seed_keypad_get_number();
        if (keyEntered != -1) {
            seed_push_key(keyEntered);
        }
}

/**
 * Loads a save file selected after it goes into a full screen state
 * retuning sSelectedFileNum to a save value defined in fileNum.
 */
static void load_main_menu_save_file(struct Object *fileButton, s32 fileNum) {
    if (fileButton->oMenuButtonState == MENU_BUTTON_STATE_FULLSCREEN) {
        sSelectedFileNum = fileNum;
    }
}

/**
 * Returns from the previous menu back to the main menu using
 * the return button (or sound mode) as source button.
 */
static void return_to_main_menu(s16 prevMenuButtonID, struct Object *sourceButton) {
    s32 buttonID;
    // If the source button is in default state and the previous menu in full screen,
    // play zoom out sound and shrink previous menu
    if (sourceButton->oMenuButtonState == MENU_BUTTON_STATE_DEFAULT
        && sMainMenuButtons[prevMenuButtonID]->oMenuButtonState == MENU_BUTTON_STATE_FULLSCREEN) {
        play_sound(SOUND_MENU_CAMERA_ZOOM_OUT, gDefaultSoundArgs);
        sMainMenuButtons[prevMenuButtonID]->oMenuButtonState = MENU_BUTTON_STATE_SHRINKING;
        sCurrentMenuLevel = MENU_LAYER_MAIN;
    }
}

/**
 * Menu Buttons Menu Manager Initial Action
 * Creates models of the buttons in the menu. For the Mario buttons it
 * checks if a save file exists to render an specific button model for it.
 * Unlike buttons on submenus, these are never hidden or recreated.
 */
void bhv_menu_button_manager_init(void) {
    // File A
    if (save_file_exists(SAVE_FILE_A) == TRUE) {
        sMainMenuButtons[MENU_BUTTON_PLAY_FILE_A] =
            spawn_object_rel_with_rot(gCurrentObject, MODEL_MAIN_MENU_MARIO_SAVE_BUTTON_FADE,
                                      bhvMenuButton, -6400, 2800, 0, 0, 0, 0);
    } else {
        sMainMenuButtons[MENU_BUTTON_PLAY_FILE_A] =
            spawn_object_rel_with_rot(gCurrentObject, MODEL_MAIN_MENU_MARIO_NEW_BUTTON_FADE,
                                      bhvMenuButton, -6400, 2800, 0, 0, 0, 0);
    }
    sMainMenuButtons[MENU_BUTTON_PLAY_FILE_A]->oMenuButtonScale = 1.0f;

    sMainMenuButtons[MENU_BUTTON_SEED_RESET] =
        spawn_object_rel_with_rot(gCurrentObject, 12, bhvMenuButton, -6400, -3500, 0, 0, 0, 0);
    sMainMenuButtons[MENU_BUTTON_SEED_RESET]->oMenuButtonScale = 1.0f;

    sMainMenuButtons[MENU_BUTTON_SEED_OPTION] =
        spawn_object_rel_with_rot(gCurrentObject, 12, bhvMenuButton, 6400, -3500, 0, 0, 0, 0);
    sMainMenuButtons[MENU_BUTTON_SEED_OPTION]->oMenuButtonScale = 1.0f;


    sTextBaseAlpha = 0;
}

#ifdef VERSION_JP
#define SAVE_FILE_SOUND SOUND_MENU_STAR_SOUND
#else
#define SAVE_FILE_SOUND SOUND_MENU_STAR_SOUND_OKEY_DOKEY
#endif

/**
 * In the main menu, check if a button was clicked to play it's button growing state.
 * Also play a sound and/or render buttons depending of the button ID selected.
 */
static void check_main_menu_clicked_buttons(void) {
    // Main Menu buttons
    s8 buttonID;
    // Configure Main Menu button group
    for (buttonID = MENU_BUTTON_MAIN_MIN; buttonID < MENU_BUTTON_MAIN_MAX; buttonID++) {
        s16 buttonX = sMainMenuButtons[buttonID]->oPosX;
        s16 buttonY = sMainMenuButtons[buttonID]->oPosY;

        if (check_clicked_button(buttonX, buttonY, 200.0f) == TRUE) {
            // If menu button clicked, select it
            sMainMenuButtons[buttonID]->oMenuButtonState = MENU_BUTTON_STATE_GROWING;
            sSelectedButtonID = buttonID;
            break;
        }
    }

    // Play sound of the save file clicked
    switch (sSelectedButtonID) {
        case MENU_BUTTON_PLAY_FILE_A:
            play_sound(SAVE_FILE_SOUND, gDefaultSoundArgs);
            break;
    }
}
#undef SAVE_FILE_SOUND

/**
 * Menu Buttons Menu Manager Loop Action
 * Calls a menu function depending of the button chosen.
 * sSelectedButtonID is MENU_BUTTON_NONE when the file select
 * is loaded, and that checks what buttonID is clicked in the main menu.
 */
void bhv_menu_button_manager_loop(void) {
    switch (sSelectedButtonID) {
        case MENU_BUTTON_NONE:
            check_main_menu_clicked_buttons();
            seed_menu_check_clicked_buttons();
            break;
        case MENU_BUTTON_PLAY_FILE_A:
            load_main_menu_save_file(sMainMenuButtons[MENU_BUTTON_PLAY_FILE_A], 1);
            break;
        case MENU_BUTTON_SEED_RESET:
            seed_reset();
            break;
        case MENU_BUTTON_SEED_OPTION:
            exit_score_file_to_score_menu(sMainMenuButtons[MENU_BUTTON_SEED_OPTION], MENU_BUTTON_NONE);
            break;
    }

    sClickPos[0] = -10000;
    sClickPos[1] = -10000;
}

/**
 * Cursor function that handles button inputs.
 * If the cursor is clicked, sClickPos uses the same value as sCursorPos.
 */
static void handle_cursor_button_input(void) {
    if (sSelectedButtonID == MENU_BUTTON_SEED_OPTION) {
        if (gPlayer3Controller->buttonPressed & (B_BUTTON | START_BUTTON)) {
            sClickPos[0] = sCursorPos[0];
            sClickPos[1] = sCursorPos[1];
            sCursorClickingTimer = 1;
        } else {
            if (sBingoOptionSelectTimer > 0) {
                sBingoOptionSelectTimer--;
            } else if (gPlayer3Controller->buttonPressed & D_JPAD) {
                if (sBingoOptionSelection < BINGO_OPTIONS_PER_PAGE) {
                    sBingoOptionSelection = (sBingoOptionSelection + 1) % BINGO_OPTIONS_PER_PAGE;
                } else {
                    sBingoOptionSelection =
                        (sBingoOptionSelection - BINGO_OPTIONS_PER_PAGE + 1)
                        % BINGO_OPTIONS_PER_PAGE
                        + BINGO_OPTIONS_PER_PAGE;
                }
                sBingoOptionSelectTimer = BINGO_OPTION_TIMER_FRAMES;
            } else if (gPlayer3Controller->buttonPressed & U_JPAD) {
                if (sBingoOptionSelection < BINGO_OPTIONS_PER_PAGE) {
                    sBingoOptionSelection =
                        (sBingoOptionSelection + BINGO_OPTIONS_PER_PAGE - 1) % BINGO_OPTIONS_PER_PAGE;
                } else {
                    sBingoOptionSelection =
                        (
                            (sBingoOptionSelection - BINGO_OPTIONS_PER_PAGE)
                            + (BINGO_OPTIONS_PER_PAGE - 1)
                        ) % BINGO_OPTIONS_PER_PAGE
                        + BINGO_OPTIONS_PER_PAGE;
                }
                sBingoOptionSelectTimer = BINGO_OPTION_TIMER_FRAMES;
            } else if (gPlayer3Controller->buttonPressed & (L_JPAD | R_JPAD)) {
                sBingoOptionSelection += BINGO_OPTIONS_PER_PAGE;
                sBingoOptionSelection %= (BINGO_OPTIONS_PER_PAGE * 2);
                sBingoOptionSelectTimer = BINGO_OPTION_TIMER_FRAMES;
            }
            if (gPlayer3Controller->buttonPressed & A_BUTTON) {
                sToggleCurrentOption = 1;
            }
        }
    } else { // If cursor is clicked
        if (gPlayer3Controller->buttonPressed & (A_BUTTON | B_BUTTON | START_BUTTON)) {
            sClickPos[0] = sCursorPos[0];
            sClickPos[1] = sCursorPos[1];
            sCursorClickingTimer = 1;
        }
    }
}

/**
 * Cursor function that handles analog stick input and button presses with a function near the end.
 */
static void handle_controller_cursor_input(void) {
    s16 rawStickX = gPlayer3Controller->rawStickX;
    s16 rawStickY = gPlayer3Controller->rawStickY;

    // Handle deadzone
    if (rawStickY > -2 && rawStickY < 2) {
        rawStickY = 0;
    }
    if (rawStickX > -2 && rawStickX < 2) {
        rawStickX = 0;
    }

    // Move cursor
    sCursorPos[0] += rawStickX / 8;
    sCursorPos[1] += rawStickY / 8;

    // Stop cursor from going offscreen
    if (sCursorPos[0] > 132.0f) {
        sCursorPos[0] = 132.0f;
    }
    if (sCursorPos[0] < -132.0f) {
        sCursorPos[0] = -132.0f;
    }

    if (sCursorPos[1] > 90.0f) {
        sCursorPos[1] = 90.0f;
    }
    if (sCursorPos[1] < -90.0f) {
        sCursorPos[1] = -90.0f;
    }

    if (sCursorClickingTimer == 0) {
        handle_cursor_button_input();
    }
}

/**
 * Prints the cursor (Mario Hand, different to the one in the Mario screen)
 * and loads it's controller inputs in handle_controller_cursor_input
 * to be usable on the file select.
 */
static void print_menu_cursor(void) {
    handle_controller_cursor_input();
    create_dl_translation_matrix(MENU_MTX_PUSH, sCursorPos[0] + 160.0f - 5.0, sCursorPos[1] + 120.0f - 25.0, 0.0f);
    // Get the right graphic to use for the cursor.
    if (sCursorClickingTimer == 0)
        // Idle
        gSPDisplayList(gDisplayListHead++, dl_menu_idle_hand);
    if (sCursorClickingTimer != 0)
        // Grabbing
        gSPDisplayList(gDisplayListHead++, dl_menu_grabbing_hand);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
    if (sCursorClickingTimer != 0) {
        sCursorClickingTimer++; // This is a very strange way to implement a timer? It counts up and
                                // then resets to 0 instead of just counting down to 0.
        if (sCursorClickingTimer == 5) {
            sCursorClickingTimer = 0;
        }
    }
}

/**
 * Prints a hud string depending of the hud table list defined with text fade properties.
 */
static void print_hud_lut_string_fade(s8 hudLUT, s16 x, s16 y, const unsigned char *text) {
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, sTextBaseAlpha - sTextFadeAlpha);
    print_hud_lut_string(hudLUT, x, y, text);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
}

/**
 * Prints a generic white string with text fade properties.
 */
static void print_generic_string_fade(s16 x, s16 y, const unsigned char *text) {
    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, sTextBaseAlpha - sTextFadeAlpha);
    print_generic_string(x, y, text);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
}

static void draw_seed_mode_menu(void) {
    s32 xSeedPos;
    unsigned char textEnterSeed[] = { TEXT_ENTER_SEED };
    // Display "ENTER SEED" text
    print_hud_lut_string_fade(2, 100, 35, textEnterSeed);

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);  // TODO: needed?
    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);

    // Display return, reset
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, sTextBaseAlpha);
    print_generic_string(47, 35, textReset);
    print_generic_string(240, 35, textReturn);
    print_generic_string(240, 90, textOption);

    // Display keypad
    print_hud_lut_string_fade(2, 125, 100, text123);
    print_hud_lut_string_fade(2, 125, 100 + (30 * 1), text456);
    print_hud_lut_string_fade(2, 125, 100 + (30 * 2), text789);
    print_hud_lut_string_fade(2, 125 + 3, 100 + (30 * 3), text_0_); // move right slightly

    // Display seed
    if (gBingoSeedIsSet) {
        xSeedPos = 105;
    } else {
        xSeedPos = 125;
    }
    print_hud_lut_string_fade(2, xSeedPos, 100 + (30 * -1), gBingoSeedText);

    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
}

/**
 * Prints main menu strings that shows on the yellow background menu screen.
 */
static void print_main_menu_strings(void) {
    draw_seed_mode_menu();
}


static unsigned char textSingleStar[] = { TEXT_SINGLE_STAR };
static unsigned char textAButton[] = { TEXT_A_BUTTON };
static unsigned char textBButton[] = { TEXT_B_BUTTON };
static unsigned char textZButton[] = { TEXT_Z_BUTTON };
static unsigned char textTimedStar[] = { TEXT_TIMED_STAR };
static unsigned char textReverseJoystick[] = { TEXT_REVERSE_JOYSTICK };
static unsigned char textGreenDemon[] = { TEXT_GREEN_DEMON };
static unsigned char textCoinLevel[] = { TEXT_COIN_LEVEL };
static unsigned char textTotalCoin[] = { TEXT_TOTAL_COIN };
static unsigned char text1upLevel[] = { TEXT_1UP_LEVEL };
static unsigned char textStarsLevel[] = { TEXT_STARS_LEVEL };
static unsigned char textExclamBoxes[] = { TEXT_EXCLAM_BOXES };
static unsigned char textKillEnemies[] = { TEXT_KILL_ENEMIES };

static void print_bingo_options(void) {
    s32 i;
    char obj_icon[2];
    unsigned char *option;

    if (gOptionSelectIconOpacity <= 10) {
        return;
    }

    gDPSetCombineMode(gDisplayListHead++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SURF, G_RM_XLU_SURF);
    gDPSetPrimColor(gDisplayListHead++, 0, 0, 38, 38, 38, MIN(sTextBaseAlpha, 150));
    #define LEFT_X     24
    #define RIGHT_X    160
    #define TOP_Y      12
    #define ROW_HEIGHT 16
    if (sBingoOptionSelection < BINGO_OPTIONS_PER_PAGE) {
        gDPFillRectangle(
            gDisplayListHead++,
            LEFT_X,
            TOP_Y + ROW_HEIGHT * (sBingoOptionSelection + 1),
            RIGHT_X,
            TOP_Y + ROW_HEIGHT * (sBingoOptionSelection + 2)
        );
    } else {
        gDPFillRectangle(
            gDisplayListHead++,
            RIGHT_X,
            TOP_Y + ROW_HEIGHT * ((sBingoOptionSelection % BINGO_OPTIONS_PER_PAGE) + 1),
            RIGHT_X + (RIGHT_X - LEFT_X),
            TOP_Y + ROW_HEIGHT * ((sBingoOptionSelection % BINGO_OPTIONS_PER_PAGE) + 2)
        );
    }

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, sTextBaseAlpha);
    for (i = 0; i < BINGO_OBJECTIVE_TYPE_MAX; i++) {
        if (i == sBingoOptionSelection && sToggleCurrentOption == 1) {
            sToggleCurrentOption = 0;
            gBingoObjectivesDisabled[i] ^= 1;
        }

        switch (i) {
            case BINGO_OBJECTIVE_STAR:
                strcpy(obj_icon, ICON_STAR);
                option = textSingleStar;
                break;
            case BINGO_OBJECTIVE_STAR_A_BUTTON_CHALLENGE:
                strcpy(obj_icon, ICON_A_BUTTON);
                option = textAButton;
                break;
            case BINGO_OBJECTIVE_STAR_B_BUTTON_CHALLENGE:
                strcpy(obj_icon, ICON_B_BUTTON);
                option = textBButton;
                break;
            case BINGO_OBJECTIVE_STAR_Z_BUTTON_CHALLENGE:
                strcpy(obj_icon, ICON_Z_BUTTON);
                option = textZButton;
                break;
            case BINGO_OBJECTIVE_STAR_TIMED:
                strcpy(obj_icon, ICON_TIMER);
                option = textTimedStar;
                break;
            case BINGO_OBJECTIVE_STAR_REVERSE_JOYSTICK:
                strcpy(obj_icon, ICON_JOYSTICK);
                option = textReverseJoystick;
                break;
            case BINGO_OBJECTIVE_STAR_GREEN_DEMON:
                strcpy(obj_icon, ICON_GREENDEMON);
                option = textGreenDemon;
                break;
            case BINGO_OBJECTIVE_COIN:
                strcpy(obj_icon, ICON_COIN);
                option = textCoinLevel;
                break;
            case BINGO_OBJECTIVE_MULTICOIN:
                strcpy(obj_icon, ICON_MULTICOIN);
                option = textTotalCoin;
                break;
            case BINGO_OBJECTIVE_1UPS_IN_LEVEL:
                strcpy(obj_icon, ICON_1UP);
                option = text1upLevel;
                break;
            case BINGO_OBJECTIVE_STARS_IN_LEVEL:
                strcpy(obj_icon, ICON_PURPLESTAR);
                option = textStarsLevel;
                break;
            case BINGO_OBJECTIVE_EXCLAMATION_MARK_BOX:
                strcpy(obj_icon, ICON_YELLOW_EXCLAMATION_MARK_BOX);
                option = textExclamBoxes;
                break;
            // TODO: enemies.
        }
        if (i < BINGO_OPTIONS_PER_PAGE) {
            print_text(
                LEFT_X + 1, TOP_Y + 8 + ROW_HEIGHT * (BINGO_OPTIONS_PER_PAGE - i), obj_icon
            );
            print_generic_string(
                LEFT_X + 1 + 16 + 3, TOP_Y + 8 + ROW_HEIGHT * (BINGO_OPTIONS_PER_PAGE - i), option
            );
            if (gBingoObjectivesDisabled[i]) {
                gDPSetEnvColor(gDisplayListHead++, 255, 80, 80, sTextBaseAlpha);
                print_generic_string(
                    RIGHT_X - 19, TOP_Y + 8 + ROW_HEIGHT * (BINGO_OPTIONS_PER_PAGE - i), textOff
                );
                gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, sTextBaseAlpha);
            } else {
                print_generic_string(
                    RIGHT_X - 19, TOP_Y + 8 + ROW_HEIGHT * (BINGO_OPTIONS_PER_PAGE - i), textOn
                );
            }
        } else {
            print_text(
                RIGHT_X + 1, TOP_Y + 8 + ROW_HEIGHT * (BINGO_OPTIONS_PER_PAGE - (i % BINGO_OPTIONS_PER_PAGE)), obj_icon
            );
            print_generic_string(
                RIGHT_X + 1 + 16 + 3, TOP_Y + 8 + ROW_HEIGHT * (BINGO_OPTIONS_PER_PAGE - (i % BINGO_OPTIONS_PER_PAGE)), option
            );
            if (gBingoObjectivesDisabled[i]) {
                gDPSetEnvColor(gDisplayListHead++, 255, 80, 80, sTextBaseAlpha);
                print_generic_string(
                    RIGHT_X + (RIGHT_X - LEFT_X) - 19, TOP_Y + 8 + ROW_HEIGHT * (BINGO_OPTIONS_PER_PAGE - (i % BINGO_OPTIONS_PER_PAGE)), textOff
                );
                gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, sTextBaseAlpha);
            } else {
                print_generic_string(
                    RIGHT_X + (RIGHT_X - LEFT_X) - 19, TOP_Y + 8 + ROW_HEIGHT * (BINGO_OPTIONS_PER_PAGE - (i % BINGO_OPTIONS_PER_PAGE)), textOn
                );
            }
        }
    }
    #undef LEFT_X
    #undef RIGHT_X
    #undef TOP_Y
    #undef ROW_HEIGHT

    if (sToggleCurrentOption) {
        sToggleCurrentOption = 0;
    }

    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
}

/**
 * Prints file select strings depending on the menu selected.
 * Also checks if all saves exists and defines text and main menu timers.
 */
static void print_file_select_strings(void) {
    UNUSED s32 unused1;
    UNUSED s32 unused2;

    create_dl_ortho_matrix();
    switch (sSelectedButtonID) {
        case MENU_BUTTON_NONE:
            print_main_menu_strings();
            break;
        case MENU_BUTTON_SEED_OPTION:
            print_bingo_options();
            break;
    }
    // Timers for menu alpha text and the main menu itself
    if (sTextBaseAlpha < 250) {
        sTextBaseAlpha += 10;
    }
    if (sMainMenuTimer < 1000) {
        sMainMenuTimer += 1;
    }
    gOptionSelectIconOpacity = sTextBaseAlpha;
}

/**
 * Geo function that prints file select strings and the cursor.
 */
Gfx *geo_file_select_strings_and_menu_cursor(s32 callContext, UNUSED struct GraphNode *node, UNUSED f32 mtx[4][4]) {
    if (callContext == GEO_CONTEXT_RENDER) {
        print_file_select_strings();
        print_menu_cursor();
    }
    return NULL;
}

/**
 * Initiates file select values after Mario Screen.
 * Relocates cursor position of the last save if the game goes back to the Mario Screen
 * either completing a course choosing "SAVE & QUIT" or having a game over.
 */
s32 lvl_init_menu_values_and_cursor_pos(UNUSED s32 arg, UNUSED s32 unused) {
    sSelectedButtonID = MENU_BUTTON_NONE;
    sCurrentMenuLevel = MENU_LAYER_MAIN;
    sTextBaseAlpha = 0;
    // Place the cursor over the save file that was being played.
    // gCurrSaveFileNum is 1 by default when the game boots, as such
    // the cursor will point on Mario A save file.
    switch (gCurrSaveFileNum) {
        case 1: // File A
            sCursorPos[0] = -94.0f;
            sCursorPos[1] = 46.0f;
            break;
        case 2: // File B
            sCursorPos[0] = 24.0f;
            sCursorPos[1] = 46.0f;
            break;
        case 3: // File C
            sCursorPos[0] = -94.0f;
            sCursorPos[1] = 5.0f;
            break;
        case 4: // File D
            sCursorPos[0] = 24.0f;
            sCursorPos[1] = 5.0f;
            break;
    }
    sClickPos[0] = -10000;
    sClickPos[1] = -10000;
    sCursorClickingTimer = 0;
    sSelectedFileNum = 0;
    sFadeOutText = FALSE;
    sTextFadeAlpha = 0;
    sMainMenuTimer = 0;
    sSoundMode = save_file_get_sound_mode();

    //! no return value
#ifdef AVOID_UB
    return 0;
#endif
}

u32 get_seed(void) {
    if (!gBingoSeedIsSet) {
        return gGlobalTimer; // good enough as a seed
    }
    // no pow() so i'm lazy
    return (gBingoSeedText[0] * 100000000 + gBingoSeedText[1] * 10000000 + gBingoSeedText[2] * 1000000
            + gBingoSeedText[3] * 100000 + gBingoSeedText[4] * 10000 + gBingoSeedText[5] * 1000
            + gBingoSeedText[6] * 100 + gBingoSeedText[7] * 10 + gBingoSeedText[8] * 1);
}

/**
 * Updates file select menu button objects so they can be interacted.
 * When a save file is selected, it returns fileNum value
 * defined in load_main_menu_save_file.
 */
s32 lvl_update_obj_and_load_file_selected(UNUSED s32 arg, UNUSED s32 unused) {
    area_update_objects();
    if (sSelectedFileNum && !gBingoInitialized) {
        setup_bingo_objectives(get_seed());
    }
    return sSelectedFileNum;
}
