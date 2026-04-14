/*
 * Hamsterball Ball Physics
 * 
 * Based on reverse engineering of Ball_Update at 0x405190 (18KB decomp)
 * Ball_ApplyForce at 0x402650
 * Ball_CheckCollisionPlanes at 0x402810
 * Ball_CollisionCheck at 0x402DE0
 *
 * Ball object layout from 0x40AFE0 (0xC98 bytes):
 *   +0x164: X position (float)
 *   +0x168: Y position (float)
 *   +0x16C: Z position (float)
 *   +0x170-0x178: velocity vector (zeroed on collision)
 *   +0x284: ball radius
 *   +0x2F0: frame counter (affects force scaling)
 *   +0x324: special state (shrunk/slow multiplier)
 *   +0x808: freeze counter
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include "physics/physics.h"
#include "input/input.h"

/* Constants from original binary data section (CONFIRMED via Ghidra) */
#define GRAVITY             -0.15f      /* Default downward pull */
#define BALL_RADIUS         35.0f       /* 0x420C0000 at Ball+0x284 */
#define MAX_VELOCITY        5000.0f     /* Ball+0x188 max_speed */
#define FRICTION            0.95f       /* _DAT_004CF4C0 per-frame friction */
#define Y_DAMP              0.8f        /* _DAT_004CF434 vertical damping */
#define SPEED_FRICTION      0.99f       /* _DAT_004CF4B8 */
#define ACCEL_FORCE         40.0f       /* Keyboard force magnitude (scaled for radius) */
#define BRAKE_FORCE         0.7f        /* Braking deceleration */
#define COLLISION_BOUNCE    0.3f        /* Coefficient of restitution */
#define CAM_FOLLOW_SPEED    0.08f       /* Camera lerp factor */
#define CAM_HEIGHT_OFFSET   80.0f       /* Camera height above ball */
#define CAM_DISTANCE        120.0f      /* Camera distance behind ball */

/* Global ball state */
static ball_t g_ball;

/* Camera state */
static struct {
    vec3_t position;
    vec3_t target;
} g_camera;

/* Input state */
static struct {
    bool up, down, left, right;
    bool action1, action2;
} g_input;

/* Physics timing */
static uint32_t g_frame_count = 0;
static float g_delta_accum = 0.0f;
static const float PHYSICS_STEP = 1.0f / 60.0f;  /* Fixed timestep */

void physics_init(void) {
    memset(&g_ball, 0, sizeof(ball_t));
    g_ball.radius = BALL_RADIUS;
    g_ball.health = 1.0f;
    g_ball.on_ground = false;
    
    memset(&g_camera, 0, sizeof(g_camera));
    memset(&g_input, 0, sizeof(g_input));
    g_frame_count = 0;
}

void physics_shutdown(void) {
    /* Nothing to clean up yet */
}

void ball_reset(vec3_t start_position) {
    g_ball.position = start_position;
    g_ball.velocity = (vec3_t){0, 0, 0};
    g_ball.acceleration = (vec3_t){0, 0, 0};
    g_ball.on_ground = false;
    g_ball.in_tar = false;
    g_ball.dizzy = false;
    g_ball.health = 1.0f;
    g_ball.angular_velocity = 0.0f;
    g_frame_count = 0;
}

vec3_t ball_get_position(void) {
    return g_ball.position;
}

void ball_apply_force(vec3_t force) {
    /* Based on Ball_ApplyForce at 0x402650 */
    /* Skip if frozen */
    if (g_ball.health <= 0.0f) return;
    
    /* First frame gets different scaling (from _DAT_004CF380) */
    float scale = 1.0f;
    if (g_frame_count == 0) {
        scale = 0.5f;  /* Reduced force on first frame */
    }
    
    /* Apply force to acceleration */
    g_ball.acceleration.x += force.x * scale;
    g_ball.acceleration.y += force.y * scale;
    g_ball.acceleration.z += force.z * scale;
}

/* Simple height-map collision for now (flat ground at y=0 + level objects)
 * Real implementation would use Ball_AdvancePositionOrCollision (0x4564C0)
 * with Collision_TraverseSpatialTree (0x465EF0) */
static float get_ground_height(float x, float z) {
    /* Arena floor is at y=0 approximately */
    return 0.0f;
}

static void physics_step(float dt) {
    /* Based on Ball_Update at 0x405190 */
    
    /* Gravity - always pull down */
    g_ball.acceleration.y += GRAVITY;
    
    /* Apply keyboard input as force */
    vec3_t input_force = {0, 0, 0};
    
    if (g_input.up)    input_force.z -= ACCEL_FORCE;
    if (g_input.down)  input_force.z += ACCEL_FORCE;
    if (g_input.left)  input_force.x -= ACCEL_FORCE;
    if (g_input.right) input_force.x += ACCEL_FORCE;
    
    /* Brake/reverse with action2 */
    if (g_input.action2) {
        g_ball.velocity.x *= (1.0f - BRAKE_FORCE * dt);
        g_ball.velocity.z *= (1.0f - BRAKE_FORCE * dt);
    }
    
    ball_apply_force(input_force);
    
    /* Integrate velocity (semi-implicit Euler) */
    g_ball.velocity.x += g_ball.acceleration.x * dt;
    g_ball.velocity.y += g_ball.acceleration.y * dt;
    g_ball.velocity.z += g_ball.acceleration.z * dt;
    
    /* Clamp velocity */
    float speed = sqrtf(g_ball.velocity.x * g_ball.velocity.x + 
                        g_ball.velocity.y * g_ball.velocity.y + 
                        g_ball.velocity.z * g_ball.velocity.z);
    if (speed > MAX_VELOCITY) {
        float scale = MAX_VELOCITY / speed;
        g_ball.velocity.x *= scale;
        g_ball.velocity.y *= scale;
        g_ball.velocity.z *= scale;
    }
    
    /* Friction - reduced when in tar */
    float friction = g_ball.in_tar ? 0.92f : FRICTION;
    g_ball.velocity.x *= friction;
    g_ball.velocity.z *= friction;
    
    /* Integrate position */
    g_ball.position.x += g_ball.velocity.x;
    g_ball.position.y += g_ball.velocity.y;
    g_ball.position.z += g_ball.velocity.z;
    
    /* Ground collision (Ball_AdvancePositionOrCollision 0x4564C0) */
    float ground_y = get_ground_height(g_ball.position.x, g_ball.position.z);
    
    if (g_ball.position.y - g_ball.radius < ground_y) {
        /* On ground */
        g_ball.position.y = ground_y + g_ball.radius;
        
        /* Bounce or settle */
        if (g_ball.velocity.y < -1.0f) {
            g_ball.velocity.y *= -COLLISION_BOUNCE;  /* Bounce */
        } else {
            g_ball.velocity.y = 0.0f;  /* Settle on ground */
        }
        g_ball.on_ground = true;
    } else {
        g_ball.on_ground = false;
    }
    
    /* Clear acceleration for next frame */
    g_ball.acceleration = (vec3_t){0, 0, 0};
    
    /* Rolling angular velocity (visual only) */
    float horiz_speed = sqrtf(g_ball.velocity.x * g_ball.velocity.x + 
                              g_ball.velocity.z * g_ball.velocity.z);
    g_ball.angular_velocity = horiz_speed * 0.5f;
    
    g_frame_count++;
}

void physics_update_input(bool up, bool down, bool left, bool right, 
                          bool action1, bool action2) {
    g_input.up = up;
    g_input.down = down;
    g_input.left = left;
    g_input.right = right;
    g_input.action1 = action1;
    g_input.action2 = action2;
}

bool physics_update(float delta_time) {
    /* Fixed timestep physics (prevents variable-rate issues) */
    g_delta_accum += delta_time;
    
    while (g_delta_accum >= PHYSICS_STEP) {
        physics_step(PHYSICS_STEP);
        g_delta_accum -= PHYSICS_STEP;
    }
    
    /* Update camera - follows ball */
    vec3_t cam_target = g_ball.position;
    g_camera.target.x += (cam_target.x - g_camera.target.x) * CAM_FOLLOW_SPEED;
    g_camera.target.y += (cam_target.y - g_camera.target.y) * CAM_FOLLOW_SPEED;
    g_camera.target.z += (cam_target.z - g_camera.target.z) * CAM_FOLLOW_SPEED;
    
    /* Camera position: above and behind ball (based on velocity direction) */
    float behind_x = 0.0f, behind_z = -1.0f;
    float speed = sqrtf(g_ball.velocity.x * g_ball.velocity.x + 
                        g_ball.velocity.z * g_ball.velocity.z);
    if (speed > 0.5f) {
        behind_x = -g_ball.velocity.x / speed;
        behind_z = -g_ball.velocity.z / speed;
    }
    
    vec3_t desired_cam = {
        g_ball.position.x + behind_x * CAM_DISTANCE,
        g_ball.position.y + CAM_HEIGHT_OFFSET,
        g_ball.position.z + behind_z * CAM_DISTANCE
    };
    
    float lerp = 0.05f;
    g_camera.position.x += (desired_cam.x - g_camera.position.x) * lerp;
    g_camera.position.y += (desired_cam.y - g_camera.position.y) * lerp;
    g_camera.position.z += (desired_cam.z - g_camera.position.z) * lerp;
    
    return true;
}

void physics_render(void) {
    /* Draw the ball - based on Ball_Render at 0x402860 */
    /* Original uses D3D8: SetRenderState for lighting, z-enable, cull, specular */
    
    float x = g_ball.position.x;
    float y = g_ball.position.y;
    float z = g_ball.position.z;
    float r = g_ball.radius;
    
    /* Ball material */
    GLfloat ball_diffuse[] = {0.9f, 0.7f, 0.3f, 1.0f};  /* Golden hamster color */
    GLfloat ball_ambient[] = {0.3f, 0.2f, 0.1f, 1.0f};
    GLfloat ball_specular[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat ball_shininess[] = {50.0f};
    
    glMaterialfv(GL_FRONT, GL_DIFFUSE, ball_diffuse);
    glMaterialfv(GL_FRONT, GL_AMBIENT, ball_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, ball_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, ball_shininess);
    
    glPushMatrix();
    glTranslatef(x, y, z);
    
    /* Rolling rotation */
    if (g_ball.angular_velocity > 0.01f) {
        float angle = g_ball.angular_velocity * g_frame_count * 0.1f;
        /* Rotate around perpendicular to velocity direction */
        float vx = g_ball.velocity.x;
        float vz = g_ball.velocity.z;
        float len = sqrtf(vx * vx + vz * vz);
        if (len > 0.001f) {
            glRotatef(angle * 57.2958f, -vz / len, 0, vx / len);
        }
    }
    
    /* Draw sphere using GLU */
    static GLUquadric *quad = NULL;
    if (!quad) quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluSphere(quad, r, 24, 16);
    glPopMatrix();
    
    /* Draw shadow on ground */
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
    
    float ground_y = get_ground_height(x, z) + 0.1f;
    float shadow_r = r * 1.2f;
    float shadow_scale = 1.0f - (y - ground_y) * 0.003f;
    if (shadow_scale < 0.3f) shadow_scale = 0.3f;
    shadow_r *= shadow_scale;
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(x, ground_y, z);
    for (int i = 0; i <= 16; i++) {
        float a = i * 2.0f * 3.14159f / 16;
        glVertex3f(x + cosf(a) * shadow_r, ground_y, z + sinf(a) * shadow_r);
    }
    glEnd();
    
    glEnable(GL_LIGHTING);
    
    /* Draw ball ground marker (debug circle) */
    glDisable(GL_LIGHTING);
    glColor4f(1.0f, 1.0f, 0.0f, 0.4f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 16; i++) {
        float a = i * 2.0f * 3.14159f / 16;
        glVertex3f(x + cosf(a) * r, ground_y + 0.2f, z + sinf(a) * r);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void physics_get_camera(vec3_t *pos, vec3_t *target) {
    if (pos) *pos = g_camera.position;
    if (target) *target = g_camera.target;
}

float physics_get_speed(void) {
    return sqrtf(g_ball.velocity.x * g_ball.velocity.x + 
                 g_ball.velocity.y * g_ball.velocity.y + 
                 g_ball.velocity.z * g_ball.velocity.z);
}