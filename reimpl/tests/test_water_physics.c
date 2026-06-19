/* test_water_physics.c - Standalone test for E:Water physics */
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "core/game.h"
#include "physics/water_physics.h"

#define EPSILON 0.001f

/* Test helpers */
static int g_passed = 0;
static int g_failed = 0;

static void check(bool condition, const char *test_name) {
    if (condition) {
        g_passed++;
        printf("  [PASS] %s\n", test_name);
    } else {
        g_failed++;
        printf("  [FAIL] %s\n", test_name);
    }
}

static void test_init(void) {
    printf("=== Test: water_physics_init ===\n");
    water_physics_init();
    check(water_get_zone_count() == 0, "no zones after init");
    water_physics_shutdown();
    printf("\n");
}

static void test_register_zone(void) {
    printf("=== Test: water_register_zone ===\n");
    water_physics_init();
    
    vec3_t center = {100.0f, 50.0f, 200.0f};
    water_register_zone(center, 40.0f, 20.0f, 60.0f);
    check(water_get_zone_count() == 1, "one zone registered");
    
    /* Point inside */
    vec3_t inside = {100.0f, 50.0f, 200.0f};
    check(water_check_point(inside) != NULL, "center point is inside");
    
    /* Point outside */
    vec3_t outside = {0.0f, 0.0f, 0.0f};
    check(water_check_point(outside) == NULL, "origin is outside");
    
    /* Point at edge (should be inside, inclusive) */
    vec3_t edge = {80.0f, 40.0f, 170.0f};
    check(water_check_point(edge) != NULL, "min edge is inside");
    
    vec3_t edge2 = {120.0f, 60.0f, 230.0f};
    check(water_check_point(edge2) != NULL, "max edge is inside");
    
    /* Point just outside */
    vec3_t just_out = {121.0f, 50.0f, 200.0f};
    check(water_check_point(just_out) == NULL, "point beyond X max is outside");
    
    water_physics_shutdown();
    printf("\n");
}

static void test_process_frame_enter(void) {
    printf("=== Test: water_process_frame - ENTER ===\n");
    water_physics_init();
    
    /* Register a water zone at (0, 50, 0) with size 100x100x100 */
    vec3_t center = {0.0f, 50.0f, 0.0f};
    water_register_zone(center, 100.0f, 100.0f, 100.0f);
    
    /* Ball falling toward water: vel_y = -10 (falling down) */
    water_state_t state;
    memset(&state, 0, sizeof(state));
    vec3_t pos = {0.0f, 50.0f, 0.0f};  /* Inside water */
    vec3_t vel = {5.0f, -10.0f, 3.0f}; /* Falling, with some X/Z drift */
    
    bool in_water = water_process_frame(&pos, &vel, 35.0f, &state);
    check(in_water == true, "ball inside water is reported as in_water");
    check(state.in_water == true, "state.in_water set to true");
    /* Entry: Y vel starts at -10, gets damped to -7, then +2.5 buoyancy = -4.5 */
    check(vel.y > -10.0f && vel.y < 0.0f, 
          "falling Y velocity reduced from downward toward upward");
    check(fabsf(vel.x - 5.0f) < 0.5f, 
          "X velocity unchanged on entry (no drag on first frame)");
    check(fabsf(vel.z - 3.0f) < 0.5f, 
          "Z velocity unchanged on entry (no drag on first frame)");
    
    /* Continue frames - buoyancy should push ball up */
    float prev_y = vel.y;
    for (int i = 0; i < 30; i++) {
        water_process_frame(&pos, &vel, 35.0f, &state);
    }
    check(vel.y > prev_y, "buoyancy increases Y velocity over time");
    check(state.in_water == true, "still in water after 30 frames");
    
    water_physics_shutdown();
    printf("\n");
}

static void test_process_frame_stay(void) {
    printf("=== Test: water_process_frame - STAY ===\n");
    water_physics_init();
    
    /* Water zone at y=50, height 40 (from 30 to 70) */
    vec3_t center = {0.0f, 50.0f, 0.0f};
    water_register_zone(center, 100.0f, 40.0f, 100.0f);
    
    water_state_t state;
    memset(&state, 0, sizeof(state));
    vec3_t pos = {0.0f, 50.0f, 0.0f};
    vec3_t vel = {0.0f, -20.0f, 0.0f};  /* Falling fast */
    
    /* First frame - enter */
    water_process_frame(&pos, &vel, 35.0f, &state);
    check(state.in_water == true, "entered water");
    /* Damped: -20 * 0.7 = -14, but floating point may vary slightly */
    check(vel.y < -10.0f && vel.y > -20.0f, 
          "entry Y velocity damped (negative, reduced)");
    
    /* 20 more frames inside - buoyancy should push up */
    for (int i = 0; i < 20; i++) {
        water_process_frame(&pos, &vel, 35.0f, &state);
    }
    
    /* Ball should now have upward Y velocity from water buoyancy */
    check(vel.y > 0.0f, "buoyancy eventually pushes ball upward");
    check(state.in_water == true, "ball still in water");
    
    /* Horizontal movement should still work normally (with small water drag) */
    vec3_t pos2 = {0.0f, 50.0f, 0.0f};
    vec3_t vel2 = {10.0f, 0.0f, 5.0f};  /* Pure horizontal */
    water_state_t state2;
    memset(&state2, 0, sizeof(state2));
    
    for (int i = 0; i < 10; i++) {
        water_process_frame(&pos2, &vel2, 35.0f, &state2);
    }
    /* 2% drag per frame: after 10 frames vel ~ 0.98^10 * 10 ~= 8.17 */
    check(vel2.x > 7.0f && vel2.x < 11.0f, "horizontal X movement preserved with small drag");
    check(vel2.z > 3.0f && vel2.z < 6.0f, "horizontal Z movement preserved with small drag");
    
    water_physics_shutdown();
    printf("\n");
}

static void test_process_frame_exit(void) {
    printf("=== Test: water_process_frame - EXIT ===\n");
    water_physics_init();
    
    vec3_t center = {0.0f, 50.0f, 0.0f};
    water_register_zone(center, 50.0f, 50.0f, 50.0f);
    
    water_state_t state;
    memset(&state, 0, sizeof(state));
    vec3_t pos = {0.0f, 50.0f, 0.0f};
    vec3_t vel = {0.0f, 0.0f, 0.0f};
    
    /* Enter water */
    water_process_frame(&pos, &vel, 35.0f, &state);
    check(state.in_water == true, "entered water");
    
    /* Move ball out of water */
    pos.x = 200.0f;  /* Far outside X bounds */
    bool in_water = water_process_frame(&pos, &vel, 35.0f, &state);
    check(in_water == false, "ball outside water returns false");
    check(state.in_water == false, "state cleared on exit");
    
    /* After exit, no more buoyancy effect */
    float vel_before = vel.y;
    water_process_frame(&pos, &vel, 35.0f, &state);
    check(fabsf(vel.y - vel_before) < EPSILON, 
          "no Y change after exit (no water effects)");
    
    water_physics_shutdown();
    printf("\n");
}

static void test_grounded_while_in_water(void) {
    printf("=== Test: grounded while in water ===\n");
    water_physics_init();
    
    vec3_t center = {0.0f, -20.0f, 0.0f};  /* Water near/at floor level */
    water_register_zone(center, 100.0f, 30.0f, 100.0f);
    
    water_state_t state;
    memset(&state, 0, sizeof(state));
    vec3_t pos = {0.0f, -20.0f, 0.0f};
    vec3_t vel = {5.0f, 0.0f, 3.0f};
    
    /* First frame: should enter and be grounded */
    bool in_water = water_process_frame(&pos, &vel, 35.0f, &state);
    check(in_water == true, "in water zone");
    check(state.in_water == true, "state.in_water true");
    
    /* Check that horizontal movement is preserved (drag is small: 2% per frame) */
    check(fabsf(vel.x - 5.0f) < 1.0f, "horizontal X preserved (small drag)");
    check(fabsf(vel.z - 3.0f) < 1.0f, "horizontal Z preserved (small drag)");
    
    water_physics_shutdown();
    printf("\n");
}

static void test_multiple_zones(void) {
    printf("=== Test: multiple water zones ===\n");
    water_physics_init();
    
    /* Register two separate water zones */
    vec3_t c1 = {0.0f, 30.0f, 0.0f};
    water_register_zone(c1, 50.0f, 50.0f, 50.0f);
    
    vec3_t c2 = {200.0f, 30.0f, 0.0f};
    water_register_zone(c2, 50.0f, 50.0f, 50.0f);
    
    check(water_get_zone_count() == 2, "two zones registered");
    
    /* Point in zone 1 */
    vec3_t p1 = {0.0f, 30.0f, 0.0f};
    check(water_check_point(p1) != NULL, "point in zone 1");
    
    /* Point in zone 2 */
    vec3_t p2 = {200.0f, 30.0f, 0.0f};
    check(water_check_point(p2) != NULL, "point in zone 2");
    
    /* Point in neither */
    vec3_t p3 = {100.0f, 30.0f, 0.0f};
    check(water_check_point(p3) == NULL, "point in gap between zones");
    
    water_physics_shutdown();
    printf("\n");
}

int main(void) {
    printf("========================================\n");
    printf("  Water Physics Test Suite\n");
    printf("========================================\n\n");
    
    test_init();
    test_register_zone();
    test_process_frame_enter();
    test_process_frame_stay();
    test_process_frame_exit();
    test_grounded_while_in_water();
    test_multiple_zones();
    
    printf("========================================\n");
    printf("  Results: %d passed, %d failed\n", g_passed, g_failed);
    printf("========================================\n");
    
    return (g_failed > 0) ? 1 : 0;
}
