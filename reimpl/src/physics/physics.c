/*
 * Hamsterball Ball Physics + Camera System
 * 
 * Physics based on reverse engineering of Ball_Update at 0x405190 (18KB decomp)
 * Camera based on Scene_SetCamera (0x419FA0) and CameraLookAt (0x413280)
 *
 * Camera: The original game uses a HIGH ISOMETRIC camera, NOT a chase cam.
 *   - CameraLookAt (0x413280): distance=45.0, height=800.0, orbit around ball
 *   - Scene_SetCamera (0x419FA0): orbit_dir = (cos(angle), 0.9, sin(angle))
 *   - This creates ~42-degree forward tilt from above
 *   - Input is relative to camera: UP = forward in camera space
 *
 * Ball layout from 0x40AFE0 (0xC98 bytes):
 *   +0x164: position (Vec3)
 *   +0x170: velocity (Vec3)
 *   +0x284: ball radius = 35.0f
 *   +0x188: max_speed = 5000.0f
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/glu.h>

#include "physics/physics.h"
#include "input/input.h"

/* ===== Constants from original binary (CONFIRMED via Ghidra) ===== */
#define GRAVITY             -0.15f      /* Default downward pull */
#define BALL_RADIUS         35.0f       /* 0x420C0000 at Ball+0x284 */
#define MAX_VELOCITY        5000.0f     /* Ball+0x188 max_speed */
#define FRICTION            0.95f       /* _DAT_004CF4C0 per-frame friction */
#define Y_DAMP              0.8f        /* _DAT_004CF434 vertical damping */
#define SPEED_FRICTION      0.99f       /* _DAT_004CF4B8 */
#define ACCEL_FORCE         40.0f       /* Keyboard force magnitude */
#define BRAKE_FORCE         0.7f        /* Braking deceleration */
#define COLLISION_BOUNCE    0.3f        /* Coefficient of restitution */

/* ===== Camera constants from CameraLookAt (0x413280) ===== */
/* Original: distance=45.0 (0x42340000), height=800.0 (0x44480000)
 * Scene_SetCamera uses orbit_dir = (cos(angle), 0.9, sin(angle))
 * with distance applied, giving camera position at:
 *   orbit_dir * distance from the target
 * The "0.9" Y component means the camera is 0.9/1.0 of the way to horizontal
 * from directly above, creating ~42 degree tilt
 *
 * For visual parity: 800 is the orbit "distance" (scalar), and the direction
 * vector (cos, 0.9, sin) means camera is at:
 *   eye = target + 800 * normalize(cos(a), 0.9, sin(a))
 *
 * Simplified: camera is high above and slightly behind the target
 */
#define CAM_ORBIT_DISTANCE  1200.0f     /* Large distance to see the whole arena from above */
#define CAM_TILT_Y         1.5f        /* Higher Y = more top-down view (was 0.9 = ~42° tilt) */
#define CAM_FOLLOW_LERP    0.08f       /* Camera interpolation speed */
#define CAM_SNAP_SPEED     0.15f       /* Faster follow during movement */

/* ===== Global ball state ===== */
static ball_t g_ball;

/* ===== Camera state ===== */
static struct {
    vec3_t position;       /* Current camera eye position */
    vec3_t target;         /* Current camera look-at target (lerps toward ball) */
    float orbit_angle;     /* Scene+0x29BC: rotation around Y axis (radians) */
    float orbit_distance;  /* Scene+0x29C0: distance scalar */
    float tilt_y;          /* Y component of orbit direction (0.9 = isometric tilt) */
} g_camera;

/* ===== Input state ===== */
static struct {
    bool up, down, left, right;
    bool action1, action2;
} g_input;

/* ===== Physics timing ===== */
static uint32_t g_frame_count = 0;
static float g_delta_accum = 0.0f;
static const float PHYSICS_STEP = 1.0f / 60.0f;

/* ===== CAMERALOOKAT target (set from MESHWORLD object) ===== */
static vec3_t g_cameralookat_target = {0, 0, 0};
static bool g_has_cameralookat = false;

/* ===== Forward declarations ===== */
static void update_camera_position(void);

/* ===== Camera-relative input direction computation ===== 
 * In the original, input force is applied in WORLD space but the
 * controls feel camera-relative because the isometric view maps
 * naturally: UP=forward in camera view = -Z in world when orbit_angle=0.
 *
 * From Ball_GetInputForce (0x46EC30):
 *   keyboard mode applies force directly along world axes X/Z,
 *   but the player perceives this as camera-relative because
 *   the camera is oriented at orbit_angle.
 *
 * We rotate input by orbit_angle so UP always means "forward on screen".
 */
static vec3_t camera_relative_input(float raw_x, float raw_z) {
    /* Rotate (raw_x, raw_z) by -orbit_angle to go from camera space to world space */
    float ca = cosf(-g_camera.orbit_angle);
    float sa = sinf(-g_camera.orbit_angle);
    vec3_t result;
    result.x = raw_x * ca - raw_z * sa;
    result.y = 0.0f;
    result.z = raw_x * sa + raw_z * ca;
    return result;
}

void physics_init(void) {
    memset(&g_ball, 0, sizeof(ball_t));
    g_ball.radius = BALL_RADIUS;
    g_ball.health = 1.0f;
    g_ball.on_ground = false;
    
    memset(&g_camera, 0, sizeof(g_camera));
    g_camera.orbit_angle = 0.7854f;    /* pi/4 = 45° — nice diagonal view */
    g_camera.orbit_distance = CAM_ORBIT_DISTANCE;
    g_camera.tilt_y = CAM_TILT_Y;
    g_camera.target = (vec3_t){0, 0, 0};
    g_camera.position = (vec3_t){0, CAM_ORBIT_DISTANCE * CAM_TILT_Y, -CAM_ORBIT_DISTANCE};
    
    memset(&g_input, 0, sizeof(g_input));
    g_frame_count = 0;
    
    g_has_cameralookat = false;
    g_cameralookat_target = (vec3_t){0, 0, 0};
}

void physics_shutdown(void) {
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
    
    /* Snap camera to ball position on reset (matches Camera Snap mode 4) */
    g_camera.target = start_position;
    update_camera_position();
}

vec3_t ball_get_position(void) {
    return g_ball.position;
}

void ball_apply_force(vec3_t force) {
    if (g_ball.health <= 0.0f) return;
    
    float scale = (g_frame_count == 0) ? 0.5f : 1.0f;
    g_ball.acceleration.x += force.x * scale;
    g_ball.acceleration.y += force.y * scale;
    g_ball.acceleration.z += force.z * scale;
}

/* ===== Simple ground collision (placeholder until full MESHWORLD collision) =====
 * Arena floor Y is computed from vertex data min Y (≈-800 for WarmUp).
 * Ball sits on floor at floor_y + radius. */
static float g_arena_floor_y = -800.0f;

void physics_set_floor_y(float floor_y) {
    g_arena_floor_y = floor_y;
}

static float get_ground_height(float x, float z) {
    return g_arena_floor_y;
}

/* ===== Update camera position from orbit parameters =====
 * Matches Scene_SetCamera (0x419FA0) Mode 5 orbit rotation:
 *   cos_a = cos(orbit_angle)
 *   sin_a = sin(orbit_angle)
 *   orbit_dir = (cos_a, 0.9, sin_a) — normalized implicitly by gluLookAt
 *   camera_pos = target + orbit_dir * distance
 */
static void update_camera_position(void) {
    float ca = cosf(g_camera.orbit_angle);
    float sa = sinf(g_camera.orbit_angle);
    
    /* orbit_dir = (cos(angle), 0.9, sin(angle)) from Scene_SetCamera decomp
     * This is NOT normalized — it sets the direction/distance ratio.
     * The actual position: eye = target + orbit_dir * distance
     * With distance=800: eye.x = target.x + 800*cos(a), etc.
     */
    g_camera.position.x = g_camera.target.x + g_camera.orbit_distance * ca;
    g_camera.position.y = g_camera.target.y + g_camera.orbit_distance * g_camera.tilt_y;
    g_camera.position.z = g_camera.target.z + g_camera.orbit_distance * sa;
}

/* ===== Set CAMERALOOKAT target from MESHWORLD object =====
 * Called by level loader when it finds a CAMERALOOKAT object.
 * Matches CameraLookAt (0x413280) step 4-5:
 *   target = CAMERALOOKAT position
 *   current = CAMERALOOKAT position (snap, not lerp)
 */
void physics_set_cameralookat(float x, float y, float z) {
    g_cameralookat_target = (vec3_t){x, y, z};
    g_has_cameralookat = true;
    /* Snap camera target to CAMERALOOKAT position (initial, may switch to ball follow) */
}

/* ===== Set orbit angle (for menu orbit, level-specific angle, etc) ===== */
void physics_set_camera_orbit(float angle) {
    g_camera.orbit_angle = angle;
}

static void physics_step(float dt) {
    /* Gravity */
    g_ball.acceleration.y += GRAVITY;
    
    /* Camera-relative input force
     * Ball_GetInputForce (0x46EC30) applies force in world space,
     * but we rotate by orbit_angle so controls feel natural.
     * UP key = ball moves forward on screen (negative Z in cam space)
     * When orbit_angle=0, forward on screen = -Z in world (standard D3D)
     */
    float input_raw_x = 0.0f, input_raw_z = 0.0f;
    if (g_input.up)    input_raw_z -= ACCEL_FORCE;
    if (g_input.down)  input_raw_z += ACCEL_FORCE;
    if (g_input.left)  input_raw_x -= ACCEL_FORCE;
    if (g_input.right) input_raw_x += ACCEL_FORCE;
    
    /* Rotate input from camera space to world space */
    vec3_t input_force = camera_relative_input(input_raw_x, input_raw_z);
    ball_apply_force(input_force);
    
    /* Brake */
    if (g_input.action2) {
        g_ball.velocity.x *= (1.0f - BRAKE_FORCE * dt);
        g_ball.velocity.z *= (1.0f - BRAKE_FORCE * dt);
    }
    
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
    
    /* Friction */
    float friction = g_ball.in_tar ? 0.92f : FRICTION;
    g_ball.velocity.x *= friction;
    g_ball.velocity.z *= friction;
    
    /* Integrate position */
    g_ball.position.x += g_ball.velocity.x;
    g_ball.position.y += g_ball.velocity.y;
    g_ball.position.z += g_ball.velocity.z;
    
    /* Ground collision (placeholder) */
    float ground_y = get_ground_height(g_ball.position.x, g_ball.position.z);
    if (g_ball.position.y - g_ball.radius < ground_y) {
        g_ball.position.y = ground_y + g_ball.radius;
        if (g_ball.velocity.y < -1.0f) {
            g_ball.velocity.y *= -COLLISION_BOUNCE;
        } else {
            g_ball.velocity.y = 0.0f;
        }
        g_ball.on_ground = true;
    } else {
        g_ball.on_ground = false;
    }
    
    /* Clear acceleration */
    g_ball.acceleration = (vec3_t){0, 0, 0};
    
    /* Rolling angular velocity (visual) */
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
    /* Fixed timestep physics */
    g_delta_accum += delta_time;
    while (g_delta_accum >= PHYSICS_STEP) {
        physics_step(PHYSICS_STEP);
        g_delta_accum -= PHYSICS_STEP;
    }
    
    /* ===== Camera update (Scene_SetCamera 0x419FA0) ===== */
    /* 
     * The original has two camera target modes:
     * 1. Arena/Rumble: camera follows CAMERALOOKAT position (fixed overhead)
     * 2. Race: camera follows ball position
     * 
     * For both: target smoothly lerps toward the desired point,
     * then camera position = target + orbit_dir * distance
     */
    vec3_t desired_target;
    if (g_has_cameralookat) {
        /* Arena mode: camera looks at a point near arena center (XZ from CAMERALOOKAT)
         * but at the ball's Y level so the camera stays above the action.
         * Original: CameraLookAt defines where camera looks, ball moves around below. */
        desired_target.x = g_cameralookat_target.x * 0.7f + g_ball.position.x * 0.3f;
        desired_target.y = g_ball.position.y;  /* Follow ball Y, not CAMERALOOKAT Y */
        desired_target.z = g_cameralookat_target.z * 0.7f + g_ball.position.z * 0.3f;
    } else {
        /* Race mode: camera directly follows ball */
        desired_target = g_ball.position;
    }
    
    /* Smooth lerp: faster during movement (matches Scene_SetCamera spring) */
    float lerp = CAM_FOLLOW_LERP;
    float speed = sqrtf(g_ball.velocity.x * g_ball.velocity.x + 
                        g_ball.velocity.z * g_ball.velocity.z);
    if (speed > 50.0f) lerp = CAM_SNAP_SPEED;
    
    g_camera.target.x += (desired_target.x - g_camera.target.x) * lerp;
    g_camera.target.y += (desired_target.y - g_camera.target.y) * lerp;
    g_camera.target.z += (desired_target.z - g_camera.target.z) * lerp;
    
    /* Update camera position from orbit */
    update_camera_position();
    
    return true;
}

void physics_render(void) {
    float x = g_ball.position.x;
    float y = g_ball.position.y;
    float z = g_ball.position.z;
    float r = g_ball.radius;
    
    /* Ball material */
    GLfloat ball_diffuse[] = {0.9f, 0.7f, 0.3f, 1.0f};
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
        float vx = g_ball.velocity.x;
        float vz = g_ball.velocity.z;
        float len = sqrtf(vx * vx + vz * vz);
        if (len > 0.001f) {
            glRotatef(angle * 57.2958f, -vz / len, 0, vx / len);
        }
    }
    
    /* Draw sphere */
    static GLUquadric *quad = NULL;
    if (!quad) quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluSphere(quad, r, 24, 16);
    glPopMatrix();
    
    /* Shadow on ground */
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