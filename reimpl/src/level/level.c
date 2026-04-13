/* Level management stub */
#include <stdio.h>
#include <stdbool.h>
#include "level/level.h"

void level_load_list(const char *dir) { (void)dir; }
void level_shutdown(void) {}
void level_update(float dt) { (void)dt; }
void level_render(void) {}
const char *level_get_name(int race) { (void)race; return "Unknown"; }