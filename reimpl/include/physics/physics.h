/*
 * Physics system - Ball movement and collision
 * Based on BallPath string at 0x4D5DB0
 * Corresponds to rolling physics, collision detection, ball mechanics
 * Status: INFERRED
 */

#ifndef PHYSICS_H
#define PHYSICS_H

#include <stdbool.h>
#include <stdint.h>

/* 3D Vector - CONFIRMED from MESH/MESHWORLD analysis */
typedef struct {
    float x, y, z;
} vec3_t;

typedef struct {
    float x, y, z, w;
} vec4_t;

typedef struct {
    float m[3][3];
} mat3_t;

typedef struct {
    float m[4][4];
} mat4_t;

/* Ball state - corresponds to Sphere.MESH physics */
typedef struct {
    vec3_t position;
    vec3_t velocity;
    vec3_t acceleration;
    float radius;           /* 0.045 from Sphere.MESH (CONFIRMED) */
    float angular_velocity;
    float health;           /* Ball state: intact, cracking, broken */
    bool on_ground;
    bool in_tar;            /* "Sign-BewareTheTar" texture */
    bool in_tube;           /* "ToobChecker" levels */
    bool dizzy;             /* "E:NODIZZY" modifier */
} ball_t;

/* Physics functions */
void physics_init(void);
void physics_shutdown(void);
bool physics_update(float delta_time);
void physics_render(void);

/* Ball operations */
void ball_reset(vec3_t start_position);
void ball_apply_force(vec3_t force);
void ball_set_velocity(vec3_t velocity);
vec3_t ball_get_position(void);

/* Input update - called before physics_update */
void physics_update_input(bool up, bool down, bool left, bool right,
                          bool action1, bool action2);

/* Camera - set up by physics based on ball position */
void physics_get_camera(vec3_t *pos, vec3_t *target);

/* Speed readout */
float physics_get_speed(void);

/* Collision detection - corresponds to .meshcollision format */
typedef struct {
    vec3_t normal;
    float distance;
} collision_result_t;

bool physics_check_collision(vec3_t pos, vec3_t dir, collision_result_t *result);

#endif /* PHYSICS_H */