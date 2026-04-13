/*
 * Level Management
 * Based on Level_ctor at 0x461510, LevelState_ctor at 0x461460
 */

#ifndef LEVEL_H
#define LEVEL_H

#include <stdbool.h>
#include "level/meshworld_parser.h"

/* Level functions */
void level_load_list(const char *dir);
void level_shutdown(void);
void level_update(float dt);
void level_render(void);

/* Level name lookup */
const char *level_get_name(int race);

#endif /* LEVEL_H */