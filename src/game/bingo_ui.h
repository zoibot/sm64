#ifndef _BINGO_UI_H
#define _BINGO_UI_H

extern s8 gBingoAllowBoardToShow;
extern s8 gForceDrawBingoScreen;

void print_bingo_icon(s32 x, s32 y, s32 iconIndex);
void print_bingo_icon_alpha(s32 x, s32 y, s32 iconIndex, u8 alpha);
void draw_bingo_hud_timer(void);
void draw_bingo_screen(void);
void draw_bingo_win_screen(void);

#endif /* _BINGO_UI_H */