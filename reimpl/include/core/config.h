/* config.h - Configuration system
 * Based on LoadConfig (0x42AE80), SaveConfig (0x42B6E0)
 * Original reads HS.CFG with resolution, volume, key mappings
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef struct {
    int width;
    int height;
    int fullscreen;
    float sound_volume;
    float music_volume;
    int texture_quality;
    int color_mode;
    /* Key bindings (6 actions, DIK codes at KeyboardDevice+0x143-0x148) */
    int key_up;
    int key_down;
    int key_left;
    int key_right;
    int key_action1;
    int key_action2;
} config_t;

bool config_load(config_t *config, const char *path);
bool config_save(config_t *config, const char *path);
void config_defaults(config_t *config);

#endif