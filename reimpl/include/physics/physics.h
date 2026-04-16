/* physics.h - Ball physics matching original Ball_Update (0x405190)
 * 
 * Key original constants (from data section):
 *   Ball radius: 35.0f (0x420C0000) at +0x284
 *   max_speed: 5000.0f at +0x188
 *   speed_scale at +0x18C
 *   Friction: 0.95f (_DAT_004CF4C0)
 *   Y damp: 0.8f (_DAT_004CF434)
 *   Speed friction: 0.99f (_DAT_004CF4B8)
 *   Gravity: -0.15f default
 */
#ifndef PHYSICS_H
#define PHYSICS_H

#include <stdbool.h>
#include <stdint.h>
#include "core/game.h"

/* Extended math types */
typedef struct { float x, y, z, w; } vec4_t;
typedef struct { float m[3][3]; } mat3_t;
typedef struct { float m[4][4]; } mat4_t;

/* Physics functions */
void physics_init(void);
void physics_shutdown(void);
bool physics_update(float delta_time);
void physics_render(void);

/* Ball operations (match Ball_ApplyForce 0x402650, Ball_SetTrajectory 0x403850) */
void ball_reset(vec3_t start_position);
void ball_apply_force(vec3_t force);
void ball_set_velocity(vec3_t velocity);
vec3_t ball_get_position(void);

/* Input update - called before physics_update (Ball_GetInputForce 0x46EC30) */
void physics_update_input(bool up, bool down, bool left, bool right,
                          bool action1, bool action2);

/* Camera (CameraLookAt 0x413280) */
void physics_get_camera(vec3_t *pos, vec3_t *target);

/* Speed readout */
float physics_get_speed(void);

/* Collision (Collision_TraverseSpatialTree 0x465EF0) */
typedef struct {
    vec3_t normal;
    float distance;
} collision_result_t;

bool physics_check_collision(vec3_t pos, vec3_t dir, collision_result_t *result);

/* Camera configuration (from CameraLookAt 0x413280 and Scene_SetCamera 0x419FA0) */
void physics_set_cameralookat(float x, float y, float z);
void physics_set_camera_orbit(float angle_radians);

#endif