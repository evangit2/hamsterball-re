#include <stdio.h>
#include "level/meshworld_parser.h"
int main() {
    mw_level_t *l = meshworld_parse_file("build-win64/Levels/Arena-WarmUp.MESHWORLD");
    if (!l) return 1;
    printf("bg_color: (%.2f, %.2f, %.2f)\n", l->bg_color.x, l->bg_color.y, l->bg_color.z);
    printf("ambient: (%.2f, %.2f, %.2f)\n", l->ambient_color.x, l->ambient_color.y, l->ambient_color.z);
    meshworld_free(l);
    return 0;
}
