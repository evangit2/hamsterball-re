/*
 * Configuration system
 * Based on LoadConfig at 0x42AE80, SaveConfig at 0x42B6E0
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "core/game.h"

/* config_t is defined in game.h */

bool config_load(config_t *config, const char *path);
bool config_save(config_t *config, const char *path);

#endif /* CONFIG_H */