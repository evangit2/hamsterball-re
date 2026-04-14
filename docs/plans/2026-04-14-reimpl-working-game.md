# Hamsterball Reimplementation Plan: Working Game with Comprehensive Tests

> **For Hermes:** Use subagent-driven-development skill to implement this plan task-by-task.

**Goal:** Build a working Hamsterball reimplementation that loads all original assets the same way and calls the same APIs, with a comprehensive test suite to verify correctness and diagnose problems.

**Architecture:** Replace the current stub-heavy codebase with a layered, testable engine. Each module matches the original binary's subsystem (filesystem, mesh, meshworld, texture, audio, input, physics, scene, camera, UI/menu, game state). All asset loading uses the original game's file formats and directory structure. SDL2+OpenGL replaces D3D8, SDL_mixer replaces BASS. Tests verify each layer against known binary behavior.

**Tech Stack:** C11, SDL2, OpenGL 2.1, SDL_mixer, SDL_image, GLU, CMake, bash test harness

---

## Phase 1: Test Infrastructure & Asset Loading (Foundation)

### Task 1: Create test harness infrastructure

**Objective:** Set up a test runner that can run unit tests and integration tests, report pass/fail, and capture output.

**Files:**
- Create: `reimpl/tests/test_runner.sh`
- Create: `reimpl/tests/test_helpers.h`
- Create: `reimpl/tests/test_helpers.c`
- Modify: `reimpl/CMakeLists.txt`

**Step 1:** Create test runner shell script

```bash
#!/bin/bash
# test_runner.sh - Run all tests, report results
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${1:-$SCRIPT_DIR/../build}"
PASS=0; FAIL=0; TOTAL=0
run_test() {
    local name="$1" cmd="$2"
    TOTAL=$((TOTAL+1))
    printf "  TEST %-50s " "$name"
    if output=$($cmd 2>&1); then
        PASS=$((PASS+1))
        echo "PASS"
    else
        FAIL=$((FAIL+1))
        echo "FAIL"
        echo "$output" | sed 's/^/    /'
    fi
}
# Unit tests (no display needed)
for t in "$BUILD_DIR"/test_*; do
    [ -x "$t" ] && run_test "$(basename $t)" "$t"
done
# Integration tests need display
export DISPLAY=${DISPLAY:-:99}
for t in "$BUILD_DIR"/itest_*; do
    [ -x "$t" ] && run_test "$(basename $t)" "timeout 10 $t"
done
echo ""
echo "Results: $PASS/$TOTAL passed, $FAIL failed"
[ $FAIL -eq 0 ]
```

**Step 2:** Create C test helper macros

```c
// test_helpers.h - Simple C test macros
#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static int g_tests_run = 0, g_tests_pass = 0, g_tests_fail = 0;

#define TEST(name) void name(void)
#define RUN_TEST(name) do { \
    printf("  %-50s ", #name); \
    g_tests_run++; \
    name(); \
    g_tests_pass++; \
    printf("PASS\n"); \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        g_tests_fail++; g_tests_pass--; \
        printf("FAIL\n    %s != %s (line %d): %ld != %ld\n", \
               #a, #b, __LINE__, (long)(a), (long)(b)); \
        return; \
    } \
} while(0)

#define ASSERT_FEQ(a, b, eps) do { \
    if (fabs((a)-(b)) > (eps)) { \
        g_tests_fail++; g_tests_pass--; \
        printf("FAIL\n    %s != %s (line %d): %f != %f\n", \
               #a, #b, __LINE__, (double)(a), (double)(b)); \
        return; \
    } \
} while(0)

#define ASSERT_NEQ(a, b) do { if ((a) == (b)) { \
    g_tests_fail++; g_tests_pass--; \
    printf("FAIL\n    %s == %s (line %d)\n", #a, #b, __LINE__); return; \
} } while(0)

#define ASSERT_NULL(p) do { if ((p) != NULL) { \
    g_tests_fail++; g_tests_pass--; \
    printf("FAIL\n    %s not NULL (line %d)\n", #p, __LINE__); return; \
} } while(0)

#define ASSERT_NOT_NULL(p) do { if ((p) == NULL) { \
    g_tests_fail++; g_tests_pass--; \
    printf("FAIL\n    %s is NULL (line %d)\n", #p, __LINE__); return; \
} } while(0)

#define ASSERT_STR_EQ(a, b) do { if (strcmp((a),(b)) != 0) { \
    g_tests_fail++; g_tests_pass--; \
    printf("FAIL\n    %s != %s (line %d): \"%s\" != \"%s\"\n", \
           #a, #b, __LINE__, (a), (b)); return; \
} } while(0)

#define TEST_MAIN() int main(void) { \
    printf("Running tests...\n"); \
    test_all(); \
    printf("\n%d/%d passed\n", g_tests_pass, g_tests_run); \
    return g_tests_fail > 0 ? 1 : 0; \
}

#endif
```

**Step 3:** Add CMake test targets

```cmake
# At end of CMakeLists.txt, add:
enable_testing()

# Find all test source files
file(GLOB TEST_SOURCES "tests/test_*.c")
file(GLOB ITEST_SOURCES "tests/itest_*.c")

foreach(src ${TEST_SOURCES})
    get_filename_component(name ${src} NAME_WE)
    add_executable(${name} ${src} src/level/meshworld_parser.c src/level/mesh_parser.c)
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${name} m)
    add_test(NAME ${name} COMMAND ${name})
endforeach()

# Integration tests need SDL+GL
foreach(src ${ITEST_SOURCES})
    get_filename_component(name ${src} NAME_WE)
    add_executable(${name} ${src} src/level/meshworld_parser.c src/level/mesh_parser.c)
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/include ${SDL2_INCLUDE_DIRS})
    target_link_libraries(${name} ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} m)
endforeach()
```

**Step 4:** Run to verify

```bash
cd ~/hamsterball-re/reimpl/build && cmake .. && make test_runner 2>/dev/null; bash ../tests/test_runner.sh
```

Expected: 0 tests (empty harness), exit 0.

**Step 5:** Commit

```bash
cd ~/hamsterball-re && git add reimpl/tests/ reimpl/CMakeLists.txt && git commit -m "reimpl: test harness infrastructure - shell runner, C macros, CMake targets"
```

### Task 2: Asset path configuration and filesystem module

**Objective:** Create a centralized asset path resolver that mirrors the original game's directory structure.

**Files:**
- Create: `reimpl/include/core/filesys.h`
- Create: `reimpl/src/core/filesys.c`
- Create: `reimpl/tests/test_filesys.c`

Original game reads from these locations (relative to exe directory):
- `Levels/` — .MESHWORLD level files
- `Meshes/` — .MESH model files
- `Textures/` — .PNG texture files (also .BMP, .JPG)
- `Sounds/` — .WAV sound files
- `Music/` — .MO3 music files
- `Data/` — game data (racedata.xml, jukebox.xml, HS.CFG)
- `Fonts/` — font.description + font PNGs

**Step 1:** Write filesystem module

```c
// filesys.h
#ifndef FILESYS_H
#define FILESYS_H
#include <stddef.h>
typedef struct { char base_path[512]; } filesys_t;
void filesys_init(filesys_t *fs, const char *exe_path);
void filesys_set_base(filesys_t *fs, const char *path);
size_t filesys_resolve(const filesys_t *fs, const char *subdir, const char *name, const char *ext, char *out, size_t out_size);
int filesys_file_exists(const char *path);
int filesys_read_file(const char *path, uint8_t **data, size_t *size);
#endif
```

**Step 2:** Write test

```c
// test_filesys.c
#include "test_helpers.h"
#include "core/filesys.h"

TEST(test_filesys_init) {
    filesys_t fs;
    filesys_init(&fs, "/path/to/Hamsterball.exe");
    ASSERT_STR_EQ(fs.base_path, "/path/to");
}

TEST(test_filesys_resolve) {
    filesys_t fs;
    filesys_init(&fs, "/games/hamsterball");
    char out[512];
    size_t len = filesys_resolve(&fs, "Levels", "Level1", "MESHWORLD", out, sizeof(out));
    ASSERT_NEQ(len, 0);
    ASSERT_STR_EQ(out, "/games/hamsterball/Levels/Level1.MESHWORLD");
}

TEST(test_filesys_exists) {
    // Use a file we know exists
    ASSERT_EQ(filesys_file_exists("/dev/null"), 1);
    ASSERT_EQ(filesys_file_exists("/nonexistent/file.xyz"), 0);
}

void test_all(void) {
    RUN_TEST(test_filesys_init);
    RUN_TEST(test_filesys_resolve);
    RUN_TEST(test_filesys_exists);
}

TEST_MAIN()
```

**Step 3:** Implement, build, test, commit.

### Task 3: MESHWORLD parser tests against all 87 original levels

**Objective:** Verify the existing parser works on ALL original level files and report statistics.

**Files:**
- Create: `reimpl/tests/test_meshworld.c`

**Step 1:** Write test that iterates all levels

```c
// test_meshworld.c
#include "test_helpers.h"
#include "level/meshworld_parser.h"
#include "core/filesys.h"
#include <dirent.h>

#define LEVELS_DIR "../originals/installed/extracted/Levels"

static int count_levels(void) {
    DIR *d = opendir(LEVELS_DIR);
    if (!d) return 0;
    int count = 0;
    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (strstr(ent->d_name, ".MESHWORLD")) count++;
    }
    closedir(d);
    return count;
}

TEST(test_all_levels_parse) {
    DIR *d = opendir(LEVELS_DIR);
    ASSERT_NOT_NULL(d);
    int parsed = 0, total = 0;
    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (!strstr(ent->d_name, ".MESHWORLD")) continue;
        total++;
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", LEVELS_DIR, ent->d_name);
        mw_level_t *level = meshworld_parse_file(path);
        if (level && level->object_count > 0) parsed++;
        if (level) meshworld_free(level);
    }
    closedir(d);
    printf("(%d/%d parsed) ", parsed, total);
    ASSERT_EQ(parsed, total);  // All must parse
    ASSERT_EQ(total, 87);      // Known count
}

TEST(test_level1_objects) {
    mw_level_t *level = meshworld_parse_file(LEVELS_DIR "/Level1.MESHWORLD");
    ASSERT_NOT_NULL(level);
    ASSERT_NEQ(level->object_count, 0);
    // Level1 has known START objects
    int starts = 0;
    for (int i = 0; i < level->object_count; i++) {
        if (level->objects[i].type == MW_OBJ_START) starts++;
    }
    ASSERT_NEQ(starts, 0);
    meshworld_free(level);
}

TEST(test_arena_levels_exist) {
    const char *arenas[] = {"Arena-WarmUp", "Arena-Beginner", "Arena-Dizzy",
        "Arena-Expert", "Arena-Glass", "Arena-Impossible"};
    for (int i = 0; i < 6; i++) {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s.MESHWORLD", LEVELS_DIR, arenas[i]);
        mw_level_t *level = meshworld_parse_file(path);
        ASSERT_NOT_NULL_MSG(level, arenas[i]);
        meshworld_free(level);
    }
}

void test_all(void) {
    RUN_TEST(test_all_levels_parse);
    RUN_TEST(test_level1_objects);
    RUN_TEST(test_arena_levels_exist);
}

TEST_MAIN()
```

**Step 2:** Build and run: `cd build && make test_meshworld && ./test_meshworld`

**Step 3:** Fix any parse failures, commit.

### Task 4: MESH parser tests against all 33 original meshes

**Objective:** Same as Task 3 but for .MESH model files.

**Files:**
- Create: `reimpl/tests/test_mesh.c`

**Step 1:** Test all meshes parse, verify vertex counts match known values.

**Step 2:** Build, run, fix, commit.

### Task 5: Texture loading module (PNG/BMP/JPG via SDL_image)

**Objective:** Load texture files the same paths the original game uses (Textures/ dir, with -mip1 variants).

**Files:**
- Create: `reimpl/include/graphics/texture.h`
- Create: `reimpl/src/graphics/texture.c`
- Create: `reimpl/tests/itest_texture.c`

Original game texture loading from 0x476770 (Texture_Create):
1. Strip extension from filename
2. Try: `{path}{name}-mip1.{fmt}` then `{path}{name}.{fmt}`
3. Try formats: PNG, JPG, BMP in order
4. Store in texture cache (AthenaList at Graphics+0x2E4)

**Step 1:** Implement texture module with SDL_image backend.

**Step 2:** Write integration test that loads known textures from original game dir.

**Step 3:** Build, test, commit.

### Task 6: Material and mesh rendering integration

**Objective:** Load a .MESH file, apply its material, load its texture, and render it with OpenGL.

**Files:**
- Create: `reimpl/include/graphics/mesh_render.h`
- Create: `reimpl/src/graphics/mesh_render.c`
- Create: `reimpl/tests/itest_mesh_render.c`

**Step 1:** Implement mesh GL upload (VBO from vertex array + texture).

**Step 2:** Integration test: load Sphere.MESH, load HamsterBall.png, render to offscreen, verify no GL errors.

**Step 3:** Build, test, commit.

### Task 7: Audio loading module (WAV via SDL_mixer)

**Objective:** Load .WAV sound files from original game's Sounds/ directory.

**Files:**
- Create: `reimpl/include/audio/sound.h`
- Create: `reimpl/src/audio/sound.c`
- Create: `reimpl/tests/itest_audio.c`

Original game at 0x459660 (Sound_LoadOggOrWav): tries .ogg then .wav fallback.
At 0x459810 (Sound_GetNextChannel): circular buffer channel allocator.

**Step 1:** Implement WAV loading with SDL_mixer.

**Step 2:** Test: load 5 known sounds, verify they play without error.

**Step 3:** Build, test, commit.

### Task 8: Font loading module

**Objective:** Load font.description + font PNG files from original Fonts/ directory.

**Files:**
- Create: `reimpl/include/graphics/font.h`
- Create: `reimpl/src/graphics/font.c`
- Create: `reimpl/tests/itest_font.c`

Font_Load at 0x457130. Font format: text .description file mapping char codes to UV rects in font PNG atlas.

**Step 1:** Implement font loader.

**Step 2:** Test: load showcardgothic16 (used by splash screen), verify glyph metrics.

**Step 3:** Build, test, commit.

## Phase 2: Game Subsystems

### Task 9: Input module with key rebinding

**Objective:** Full DirectInput8 replacement with SDL keyboard/mouse/joystick, 6 rebindable actions.

**Files:**
- Modify: `reimpl/src/input/input.c`
- Modify: `reimpl/include/input/input.h`
- Create: `reimpl/tests/itest_input.c`

Original input: KeyboardDevice at 0x46E250, 6 key bindings at +0x143-0x148.
Ball_GetInputForce at 0x46EC30: mode 1=keyboard, 2=mouse, 4-7=joystick.

**Step 1:** Implement configurable key mapping with SDL scancode support.

**Step 2:** Test: simulate key events, verify input state changes.

**Step 3:** Commit.

### Task 10: Physics engine with correct constants

**Objective:** Implement ball physics with the exact constants from the original binary, replacing placeholder values.

**Files:**
- Modify: `reimpl/src/physics/physics.c`
- Modify: `reimpl/include/physics/physics.h`
- Create: `reimpl/tests/test_physics.c`

Original constants (from data section):
- Ball radius: 35.0f (0x420C0000) — NOT 3.0f
- max_speed: 5000.0f (at Ball+0x188)
- speed_scale: (at Ball+0x18C)  
- Gravity: -0.15f default
- Friction: 0.95f (_DAT_004CF4C0)
- Y damp: 0.8f (_DAT_004CF434)
- Speed friction: 0.99f (_DAT_004CF4B8)

**Step 1:** Replace all placeholder constants with correct binary values.

**Step 2:** Write unit tests: apply force → verify position after N steps matches expected trajectory.

**Step 3:** Commit.

### Task 11: Collision system - mesh collision

**Objective:** Implement the core collision detection using the spatial tree system from the original.

**Files:**
- Create: `reimpl/include/physics/collision.h`
- Create: `reimpl/src/physics/collision.c`
- Create: `reimpl/tests/test_collision.c`

Key original functions:
- Collision_TraverseSpatialTree (0x465EF0): octree traversal
- Ball_AdvancePositionOrCollision (0x4564C0): core physics+collision
- AABB_ContainsPoint (0x4580D0)
- Mesh_FindClosestCollision (0x465D90)

**Step 1:** Implement AABB, spatial tree, ray-mesh collision.

**Step 2:** Test: sphere vs known triangle, verify hit point.

**Step 3:** Commit.

### Task 12: Camera system

**Objective:** Camera follow system matching original behavior.

**Files:**
- Create: `reimpl/include/graphics/camera.h`
- Create: `reimpl/src/graphics/camera.c`

Original: CameraLookAt (0x413280), Level_SelectCameraProfile (0x40ACA0).
Camera follows ball with lerp, configurable height/distance per level.

**Step 1:** Move camera code from physics.c into camera.c. Add CameraLookAt targets from level data.

**Step 2:** Commit.

### Task 13: Scene graph / object system

**Objective:** Scene graph matching the original's object lifecycle: add, update, render, remove.

**Files:**
- Create: `reimpl/include/scene/scene.h`
- Create: `reimpl/src/scene/scene.c`

Original: Scene_AddObject (0x469990, 77 xrefs), GameUpdate (0x469CF0), Scene_RenderAllObjects (0x45E0E0).

**Step 1:** Implement SceneObject base, Scene container, update/render dispatch.

**Step 2:** Commit.

### Task 14: Game state machine

**Objective:** Title screen, menu, race countdown, race, results — matching original app state flow.

**Files:**
- Create: `reimpl/include/core/gamestate.h`
- Create: `reimpl/src/core/gamestate.c`

Original flow: WinMain→App_Initialize→App_ShowMainMenu→[menu dispatch]→App_StartRace→Scene_SpawnBallsAndObjects→3-2-1-GO→racing→Scene_HandleBallFinish→results

**Step 1:** Implement state machine with transitions.

**Step 2:** Commit.

## Phase 3: Integration & Rendering

### Task 15: Level renderer - mesh geometry from MESHWORLD

**Objective:** Actually render the mesh geometry stored in MESHWORLD files (not just object markers).

**Files:**
- Create: `reimpl/include/graphics/level_render.h`
- Create: `reimpl/src/graphics/level_render.c`

The existing meshworld_parser extracts object positions but not the triangle geometry section. Need to extend the parser to read vertex/face data and upload to GL.

**Step 1:** Extend meshworld_parser to read Section 5 (vertex array) and face data.

**Step 2:** Implement GL VBO upload from parsed vertex data.

**Step 3:** Test: render Level1 with actual geometry visible.

**Step 4:** Commit.

### Task 16: 3-pass render pipeline (opaque, alpha, shadow)

**Objective:** Match original's 3-pass render: opaque → alpha → shadow (Scene_RenderAllObjects 0x45E0E0).

**Files:**
- Modify: `reimpl/src/graphics/renderer.c`

**Step 1:** Sort scene objects by render pass type. Render in correct order with depth/stencil states.

**Step 2:** Commit.

### Task 17: Ball rendering with MESH model

**Objective:** Render the ball using the actual Sphere.MESH + HamsterBall.png texture, matching Ball_Render (0x402860).

**Files:**
- Modify: `reimpl/src/physics/physics.c`

**Step 1:** Load Sphere.MESH at startup. Render with texture, rolling animation.

**Step 2:** Commit.

### Task 18: UI rendering with fonts

**Objective:** Render text overlays (timer, speed, menu) using loaded fonts.

**Files:**
- Create: `reimpl/include/graphics/ui_render.h`
- Create: `reimpl/src/graphics/ui_render.c`

Original: Font_DrawGlyph (0x457440), UI_DrawTextCentered (0x409C60), UI_DrawTextShadow (0x4012C0).

**Step 1:** Implement glyph quad rendering from font atlas.

**Step 2:** Commit.

### Task 19: End-to-end integration test

**Objective:** Automated test that runs the game through a complete scenario.

**Files:**
- Create: `reimpl/tests/itest_e2e.c`

**Step 1:** Test: init → load level → spawn ball → apply input → verify ball moves → verify camera follows → verify render completes → shutdown. All headless with GL offscreen.

**Step 2:** Commit.

## Phase 4: Asset Compatibility Verification

### Task 20: Asset compatibility test suite

**Objective:** Comprehensive test that verifies ALL original assets load correctly.

**Files:**
- Create: `reimpl/tests/test_asset_compat.c`

Test matrix:
- All 87 .MESHWORLD levels parse with >0 objects
- All 33 .MESH models parse with >0 vertices
- All 229 textures load without error
- All 63 sounds load and play
- Font files load
- RaceData.xml parses
- Music files load

**Step 1:** Implement, run, fix any failures.

**Step 2:** Commit.

### Task 21: Save/load config compatibility

**Objective:** Read/write HS.CFG in the same format as the original.

Original: LoadConfig (0x42AE80), SaveConfig (0x42B6E0).

**Files:**
- Create: `reimpl/include/core/config_rw.h`
- Create: `reimpl/src/core/config_rw.c`
- Create: `reimpl/tests/test_config.c`

**Step 1:** Implement, test with original HS.CFG if present.

**Step 2:** Commit.

## Key Verification Commands

```bash
# Build everything
cd ~/hamsterball-re/reimpl/build && cmake .. && make -j$(nproc)

# Run all unit tests (no display needed)
make test

# Run integration tests (needs X)
DISPLAY=:99 make test

# Run game
DISPLAY=:99 ./hamsterball --level ../originals/installed/extracted/Levels/Level1.MESHWORLD

# Asset compatibility test
./test_asset_compat

# Full test suite
bash ../tests/test_runner.sh
```