#include <ultra64.h>
#include <PR/os_cont.h>
#include <PR/os_libc.h>

#include "types.h"
#include "game.h"
#include "sm64.h"
#include "print.h"
#include "hud.h"
#include "area.h"
#include "save_file.h"
#include "bingo.h"
#include "bingo_ui.h"
#include "bingo_descriptions.h"
#include "ingame_menu.h"
#include "menu/file_select.h"
#include "engine/behavior_script.h"
#include "level_update.h"
#include "strcpy.h"
#include "segment2.h"
#include "ingame_menu.h"

// Cursor positions. -1 indicates that the cursor has not spawned yet.
// 0-indexed with bottom-left as (0, 0)
static s32 sBingoCursorX = -1;
static s32 sBingoCursorY = -1;

// Cursor default positions.
#define BINGO_CURSOR_X_DEFAULT 2
#define BINGO_CURSOR_Y_DEFAULT 2

// Cursor timing
#define BINGO_CURSOR_TIMEOUT_FRAMES 6
static s32 sBingoCursorTimer = 0; // if nonzero, don't move cursor

int bingostrlen(char *str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

void draw_bingo_win_screen() {
    char timestamp[10];
    char msg[40];

    getTimeFmt(timestamp, gbGlobalBingoTimer);
    sprintf(msg, "YOUR TIME WAS %s", timestamp);
    print_text(40, 60, msg);

    if (gbBingoShowCongratsCounter == (gbBingoShowCongratsLimit - 1)) {
        print_text(60, 40, "PRESS L AGAIN TO");
        print_text(110, 20, "DISMISS");
    } else {
        print_text(30, 40, "YOU ARE A SUPER PLAYER");
    }
}

void draw_bingo_hud_timer() {
    s32 i, j;
    s32 count = 0;
    struct BingoObjective *objective;
    const char empty_string[50] = { '\0' };
    char buffer[50];
    char buffer2[50];
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 5; j++) {
            objective = &gBingoObjectives[5 * i + j];
            if (objective->type == BINGO_OBJECTIVE_STAR_TIMED
                && objective->data.starTimerObjective.course == gCurrCourseNum
                && objective->state != BINGO_STATE_FAILED_IN_THIS_COURSE
                && objective->state != BINGO_STATE_COMPLETE) {
                getTimeFmtTiny(buffer, objective->data.starTimerObjective.maxTime
                                           - objective->data.starTimerObjective.timer);
                sprintf(buffer2, "%c%d: %s", 0xFA, objective->data.starTimerObjective.starIndex + 1,
                        buffer);
                print_text_not_tiny(242, 190 - (18 * (count + 1)), buffer2);
                count++;
                strcpy(buffer, empty_string);
                strcpy(buffer2, empty_string);
            }
        }
    }
    if (count != 0) {
        print_text_not_tiny(230, 190, "Time remaining:\xFF");
    }
}

void bingo_print_description(char *str) {
    int last_space = 0;
    int last_space_line_chars = 0;
    int line_chars = 0;
    s32 iter = 0;
    s32 total_lines = 0;
    u8 finalDesc[150] = { 0x11, 0x28, 0x2F, 0x2F, 0xFF };

    while (str[iter] != '\0') {
        line_chars++;
        // Chop the line if it's getting too long
        if (str[iter] == ' ') {
            if (line_chars >= 25) {
                line_chars = iter - last_space_line_chars;
                finalDesc[last_space] = 0xFE;
                // update number of lines
                total_lines++;
            }
            last_space = iter;
            last_space_line_chars = iter;
        }
        finalDesc[iter] = str[iter]; // tiny_text_convert_ascii(str[iter]);
        iter++;
    }
    // sue me
    if (line_chars >= 25) {
        finalDesc[last_space] = 0xFE;
    }
    finalDesc[iter] = 0xFF;

    print_text_not_tiny(180, 100 + total_lines * 10, finalDesc);
}

void print_bingo_icon(s32 x, s32 y, s32 iconIndex) {
    s32 rectX = x;
    s32 rectY = 224 - y;
    const u8 *const *glyphs = segmented_to_virtual(bingo_lut);


    gDPPipeSync(gDisplayListHead++);
    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, glyphs[iconIndex]);
    gSPDisplayList(gDisplayListHead++, dl_hud_img_load_tex_block);

    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gOptionSelectIconOpacity);
    gSPTextureRectangle(gDisplayListHead++, rectX << 2, rectY << 2, (rectX + 16) << 2,
                        (rectY + 16) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);

}

void draw_bingo_screen() {
    struct BingoObjective *objective;
    int i, j, length;
    int spacing = 35;
    enum BingoObjectiveIcon icon;
    char desc_text[300];
    char seed_print[20];
    char *bingo[5] = { "B", "I", "N", "G", "O" };

    // TODO: add gbBingosCompleted check.
    if (gPlayer1Controller->buttonDown & L_TRIG && gHudDisplay.flags != HUD_DISPLAY_NONE) {
        shade_screen_opacity(180);
    }

    // Title.
    for (i = 0; i < 5; i++) {
        print_text_large(6 + spacing * i, HUD_TOP_Y + 5, bingo[i]);
    }

    // Seed.
    sprintf(seed_print, "SEED %09d", gBingoInitialSeed);
    print_text_tiny(240, 214, seed_print);

    // Lines.
    for (i = 0; i < 4; i++) {
        print_vertical_line(25 + spacing * i, HUD_TOP_Y - 36);
    }
    for (i = 0; i < 4; i++) {
        print_horizontal_line(37 + spacing * i);
    }

    // Icons.
    // This has to be a separate for-loop from the below in order
    // to save some RSP commands; namely, avoiding duplicating the one
    // that directly follows this comment.
    gSPDisplayList(gDisplayListHead++, dl_hud_img_begin);
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 5; j++) {
            objective = &gBingoObjectives[5 * i + j];
            switch (objective->state) {
                case BINGO_STATE_COMPLETE:
                    icon = BINGO_ICON_SUCCESS;
                    break;
                case BINGO_STATE_FAILED_IN_THIS_COURSE:
                    icon = BINGO_ICON_FAILED;
                    break;
                default:
                    icon = objective->icon;
                    break;
            }
            print_bingo_icon(11 + spacing * j, 28 + spacing * i, icon);
        }
    }
    gSPDisplayList(gDisplayListHead++, dl_hud_img_end);

    // Subtitles.
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 5; j++) {
            objective = &gBingoObjectives[5 * i + j];
            length = strlen_tiny(objective->title);
            print_text_tiny(
                11 + spacing * j + 8 - (length) / 2 + 1,
                75 + spacing * (4 - i),
                objective->title
            );
        }
    }

    // Update cursor on button press.
    if (gPlayer1Controller->buttonDown & JPAD_BUTTONS && !sBingoCursorTimer) {
        if (sBingoCursorX == -1) {
            sBingoCursorX = BINGO_CURSOR_X_DEFAULT;
            sBingoCursorY = BINGO_CURSOR_Y_DEFAULT;
        } else {
            if (gPlayer1Controller->buttonDown & U_JPAD) {
                sBingoCursorY = (sBingoCursorY + 1) % 5;
            } else if (gPlayer1Controller->buttonDown & D_JPAD) {
                sBingoCursorY = (sBingoCursorY - 1 + 5) % 5;
            } else if (gPlayer1Controller->buttonDown & L_JPAD) {
                sBingoCursorX = (sBingoCursorX - 1 + 5) % 5;
            } else if (gPlayer1Controller->buttonDown & R_JPAD) {
                sBingoCursorX = (sBingoCursorX + 1) % 5;
            }
        }
        sBingoCursorTimer = BINGO_CURSOR_TIMEOUT_FRAMES;
    } else if (sBingoCursorTimer > 0) {
        sBingoCursorTimer--;
    }

    // Print cursor and details.
    if (sBingoCursorX != -1) {
        print_hand(14 + spacing * sBingoCursorX, 18 + spacing * sBingoCursorY);

        objective = &gBingoObjectives[5 * sBingoCursorY + sBingoCursorX];
        describe_objective(objective, desc_text);
        bingo_print_description(desc_text);
    }

    return;
}
