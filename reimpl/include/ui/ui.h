/* ui.h - UI and menu system
 * Original: App_ShowMainMenu (0x46B630), UI_DrawTextShadow (0x4012C0)
 */
#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include "core/game.h"
#include "graphics/texture.h"

/* Title screen states matching original App_ShowMainMenu */
typedef enum {
    UI_TITLE_LOGO,      /* Show logos (Miniclip, Raptisoft) */
    UI_TITLE_MAIN,      /* Title screen with menu */
    UI_TITLE_LEVELSEL,  /* Level selection */
} ui_title_state_t;

/* Menu selection indices (matches SimpleMenu dispatch) */
typedef enum {
    UI_MENU_RACE = 0,
    UI_MENU_TOURNAMENT = 1,
    UI_MENU_OPTIONS = 2,
    UI_MENU_QUIT = 3,
    UI_MENU_COUNT = 4
} ui_menu_item_t;

void ui_init(void);
void ui_update(float dt);
void ui_render_title(int screen_w, int screen_h);
void ui_render_hud(int screen_w, int screen_h, float speed, int fps, int obj_count);
void ui_shutdown(void);

/* Input handlers (called from main.c event loop) */
void ui_handle_key_up(void);
void ui_handle_key_down(void);
int  ui_handle_select(void);  /* Returns menu item index, or -1 if not in menu */

/* State queries */
int  ui_get_selection(void);
bool ui_is_active(void);

#endif