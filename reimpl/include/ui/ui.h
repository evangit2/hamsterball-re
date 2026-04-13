/*
 * UI system - Menu and in-game interface
 * String references:
 *   "CLICK HERE TO PLAY!" at 0x4D3EAC
 *   "HAMSTERBALL TOURNAMENT!" at 0x4D8708
 *   "=Simple Menu" at debug strings
 *   "RACE TIME:" at 0x4D03D0
 *   "BROKEN BALLS:" at 0x4D6DD0
 * Status: INFERRED
 */

#ifndef UI_H
#define UI_H

#include <stdbool.h>

/* UI state machine */
typedef enum {
    UI_STATE_TITLE,          /* Title screen with "Main Theme" music */
    UI_STATE_MAIN_MENU,      /* "CLICK HERE TO PLAY!" / "CLICK HERE TO EXIT" */
    UI_STATE_PRACTICE,       /* Practice level select */
    UI_STATE_TOURNAMENT,     /* "HAMSTERBALL TOURNAMENT!" */
    UI_STATE_TIME_TRIAL,     /* "TimeTrials" texture */
    UI_STATE_PARTY_RACE,     /* "PartyRace" / "2PRACE" */
    UI_STATE_OPTIONS,        /* Options dialog */
    UI_STATE_BUY,            /* eSellerate purchase prompt */
    UI_STATE_HIGH_SCORES,    /* "A NEW HIGH SCORE!!" */
} ui_state_t;

void ui_init(void);
void ui_shutdown(void);

/* Menu updates */
void ui_update_menu(void);
void ui_update_race(void);
void ui_update_tournament(void);

/* Rendering */
void ui_render_menu(void);
void ui_render_race(void);
void ui_render_tournament(void);

/* Text rendering using bitmap font system */
void ui_draw_text(const char *text, float x, float y, const char *font_name);
void ui_draw_text_centered(const char *text, float y, const char *font_name);

/* Font names from Fonts/ directory (CONFIRMED) */
#define FONT_ARIAL_NARROW_12_BOLD "ArialNarrow12Bold"
#define FONT_SHOWCARD_GOTHIC_14   "ShowcardGothic14"
#define FONT_SHOWCARD_GOTHIC_16   "ShowcardGothic16"
#define FONT_SHOWCARD_GOTHIC_28   "ShowcardGothic28"
#define FONT_SHOWCARD_GOTHIC_72   "ShowcardGothic72"

#endif /* UI_H */