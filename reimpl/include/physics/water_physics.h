/* water_physics.h - E:Water event plane physics system
 *
 * Based on EVENT_PLANES.md event plane system and original N:WATER handling.
 * The E:Water event plane is an INVISIBLE collision trigger that:
 *   1. When entered: immediately reduces downward Y velocity by 30%
 *   2. While inside: applies per-frame upward buoyancy force
 *   3. Grounds the ball (is_on_ground = true) while touching
 *   4. Allows normal X/Z horizontal movement
 *
 * Water zones are defined in MESHWORLD files as E:Water objects.
 * Their bounding geometry determines the water volume.
 */

#ifndef WATER_PHYSICS_H
#define WATER_PHYSICS_H

#include <stdbool.h>
#include <stdint.h>
#include "core/game.h"

/* ===== Water Zone Definition =====
 * A water zone is an axis-aligned bounding box defined by:
 *   - min/max corner (from MESHWORLD object position + size)
 *   - Optional Y-only plane (infinite X/Z, fixed Y range)
 *
 * The E:Water object in MESHWORLD provides center position.
 * We derive bounds from object + modifier_value (size) or use
 * a default volume around the position. */
typedef struct {
    vec3_t min;     /* Bottom-back-left corner of water volume */
    vec3_t max;     /* Top-front-right corner of water volume */
    bool active;    /* Enabled flag */
} water_zone_t;

/* ===== Water Physics State (per-ball) ===== */
typedef struct {
    bool in_water;          /* Currently inside a water zone */
    float entry_timer;      /* Time since entering water (frames) */
    float damping_factor;   /* Vertical velocity retention (0.7 = lose 30%) */
    float buoyancy_force;   /* Upward force per frame while in water */
    float max_buoyancy;     /* Max upward velocity from buoyancy */
} water_state_t;

/* ===== Constants ===== */
#define WATER_VERTICAL_DAMPING    0.70f   /* Keep 70% of Y vel = lose 30% */
#define WATER_BUOYANCY_FORCE      2.5f    /* Upward acceleration per frame */
#define WATER_MAX_BUOYANCY_VEL    8.0f    /* Terminal upward velocity in water */
#define WATER_ENTER_COOLDOWN      5       /* Frames before re-entry allowed */

/* ===== API ===== */

/* Initialize water physics system. Call once at game start. */
void water_physics_init(void);

/* Shutdown - free water zone array. */
void water_physics_shutdown(void);

/* Register a water zone from level data.
 * Called during level load for each E:Water object found. */
void water_register_zone(vec3_t center, float size_x, float size_y, float size_z);

/* Clear all water zones (call on level unload). */
void water_clear_zones(void);

/* Check if a position is inside any water zone.
 * Returns pointer to matched zone or NULL. */
const water_zone_t *water_check_point(vec3_t pos);

/* Process ball interaction with water for one physics frame.
 * Call AFTER gravity but BEFORE ground collision.
 *
 * Parameters:
 *   pos     - current ball position (read/write, may snap to water surface)
 *   vel     - current ball velocity (read/write, modifies Y)
 *   radius  - ball radius (for surface snap)
 *   state   - ball's water state (read/write)
 *
 * Returns true if ball is in water (velocity may be modified). */
bool water_process_frame(vec3_t *pos, vec3_t *vel, float radius, water_state_t *state);

/* Render water zones for debugging (semi-transparent blue cubes). */
void water_debug_render(void);

/* Get count of registered zones. */
int water_get_zone_count(void);

#endif /* WATER_PHYSICS_H */
