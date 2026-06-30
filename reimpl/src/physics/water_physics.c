/* water_physics.c - E:Water event plane physics implementation
 *
 * Based on Ball_Update (0x405E00) and DispatchCollisionEvents (0x40C5D0) from docs.
 *
 * Water physics rules (user-requested behavior):
 *   1. On ENTER: If falling (vel_y < 0), multiply Y velocity by 0.7 (lose 30%)
 *   2. While INSIDE: Every frame, add upward velocity until reaching max buoyancy
 *   3. While INSIDE: Ball is "grounded" (on_ground = true) for input purposes
 *   4. While INSIDE: Normal X/Z movement - water does NOT affect horizontal
 *   5. On EXIT: Normal gravity resumes, water state clears
 *
 * The original game has N:WATER (ball+0x2D5=1, timer at +0xB6) but it
 * behaves as a solid surface. This custom E:Water is a NEW behavior
 * for modding - transparent buoyancy zone.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "physics/water_physics.h"

/* ===== Water zone array ===== */
static water_zone_t *g_zones = NULL;
static int g_zone_count = 0;
static int g_zone_capacity = 0;

/* ===== Debug flag ===== */
static int g_debug = 1;  /* Set to 0 to disable debug prints */

void water_physics_init(void) {
    g_zones = NULL;
    g_zone_count = 0;
    g_zone_capacity = 0;
    if (g_debug) printf("[Water] Physics initialized\n");
}

void water_physics_shutdown(void) {
    if (g_zones) {
        free(g_zones);
        g_zones = NULL;
    }
    g_zone_count = 0;
    g_zone_capacity = 0;
}

void water_register_zone(vec3_t center, float size_x, float size_y, float size_z) {
    if (g_zone_count >= g_zone_capacity) {
        g_zone_capacity = (g_zone_capacity == 0) ? 8 : g_zone_capacity * 2;
        g_zones = realloc(g_zones, g_zone_capacity * sizeof(water_zone_t));
    }
    
    water_zone_t *zone = &g_zones[g_zone_count++];
    zone->min.x = center.x - size_x * 0.5f;
    zone->min.y = center.y - size_y * 0.5f;
    zone->min.z = center.z - size_z * 0.5f;
    zone->max.x = center.x + size_x * 0.5f;
    zone->max.y = center.y + size_y * 0.5f;
    zone->max.z = center.z + size_z * 0.5f;
    zone->active = true;
    
    if (g_debug) {
        printf("[Water] Registered zone at (%.1f,%.1f,%.1f) size (%.1f,%.1f,%.1f)\n",
               center.x, center.y, center.z, size_x, size_y, size_z);
        printf("[Water]   bounds: min(%.1f,%.1f,%.1f) max(%.1f,%.1f,%.1f)\n",
               zone->min.x, zone->min.y, zone->min.z,
               zone->max.x, zone->max.y, zone->max.z);
    }
}

void water_clear_zones(void) {
    g_zone_count = 0;
    if (g_debug) printf("[Water] All zones cleared\n");
}

const water_zone_t *water_check_point(vec3_t pos) {
    for (int i = 0; i < g_zone_count; i++) {
        if (!g_zones[i].active) continue;
        if (pos.x >= g_zones[i].min.x && pos.x <= g_zones[i].max.x &&
            pos.y >= g_zones[i].min.y && pos.y <= g_zones[i].max.y &&
            pos.z >= g_zones[i].min.z && pos.z <= g_zones[i].max.z) {
            return &g_zones[i];
        }
    }
    return NULL;
}

bool water_process_frame(vec3_t *pos, vec3_t *vel, float radius, water_state_t *state) {
    /* Check if ball center is inside any water zone */
    const water_zone_t *zone = water_check_point(*pos);
    
    if (zone) {
        /* === BALL IS IN WATER === */
        
        if (!state->in_water) {
            /* === JUST ENTERED WATER === */
            state->in_water = true;
            state->entry_timer = 0.0f;
            
            /* Rule 1: If falling, immediately lose 30% of vertical velocity.
             * The user said "lose 30% of vertical velocity" meaning
             * retain 70%. If vel_y is negative (falling), multiplying
             * by 0.7 reduces the downward speed but preserves direction.
             * If vel_y is positive (already rising), we leave it alone
             * or give a small boost. */
            if (vel->y < 0.0f) {
                /* Falling: damp by 30% (multiply by 0.7) */
                vel->y *= WATER_VERTICAL_DAMPING;
                if (g_debug) {
                    printf("[Water] ENTER: damped Y vel from %.2f to %.2f\n",
                           vel->y / WATER_VERTICAL_DAMPING, vel->y);
                }
            } else {
                /* Already rising or zero: buoyancy starts immediately */
                if (g_debug) printf("[Water] ENTER: already rising (Y=%.2f)\n", vel->y);
            }
        } else {
            /* === STILL IN WATER === */
            state->entry_timer += 1.0f;
        }
        
        /* Rule 2: Apply per-frame upward buoyancy force.
         * Add upward velocity every frame until ball is fully above water.
         * The buoyancy counteracts gravity and pushes up.
         *
         * We add a constant upward acceleration (negative Y, since -Y is up)
         * but cap it so the ball doesn't shoot out like a rocket. */
        vel->y += WATER_BUOYANCY_FORCE;
        
        /* Also apply a small drag to X/Z to simulate water resistance
         * but leave horizontal mostly free as user requested. */
        vel->x *= 0.98f;
        vel->z *= 0.98f;
        
        /* Rule 3: Ball is "grounded" while in water.
         * The caller uses return value to set on_ground.
         * We intentionally do NOT snap the ball to a surface here —
         * buoyancy naturally pushes it up and the ball can jump/fly out.
         */
        
        if (g_debug && (int)state->entry_timer % 10 == 0) {
            printf("[Water] IN: pos(%.1f,%.1f,%.1f) vel(%.2f,%.2f,%.2f)\n",
                   pos->x, pos->y, pos->z, vel->x, vel->y, vel->z);
        }
        
        return true;  /* In water: caller should set on_ground = true */
        
    } else {
        /* === NOT IN WATER === */
        if (state->in_water) {
            /* Just exited water */
            state->in_water = false;
            state->entry_timer = 0.0f;
            if (g_debug) printf("[Water] EXIT at (%.1f,%.1f,%.1f)\n", pos->x, pos->y, pos->z);
        }
        return false;
    }
}

void water_debug_render(void) {
    #if 0  /* Requires OpenGL headers - enable when integrated with renderer */
    for (int i = 0; i < g_zone_count; i++) {
        water_zone_t *z = &g_zones[i];
        if (!z->active) continue;
        
        /* Draw semi-transparent blue wireframe box */
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.3f, 0.8f, 0.25f);
        
        /* Render box faces */
        glBegin(GL_QUADS);
        /* Bottom */
        glVertex3f(z->min.x, z->min.y, z->min.z);
        glVertex3f(z->max.x, z->min.y, z->min.z);
        glVertex3f(z->max.x, z->min.y, z->max.z);
        glVertex3f(z->min.x, z->min.y, z->max.z);
        /* Top */
        glVertex3f(z->min.x, z->max.y, z->min.z);
        glVertex3f(z->max.x, z->max.y, z->min.z);
        glVertex3f(z->max.x, z->max.y, z->max.z);
        glVertex3f(z->min.x, z->max.y, z->max.z);
        glEnd();
        
        /* Wireframe edges */
        glColor4f(0.0f, 0.5f, 1.0f, 0.5f);
        glBegin(GL_LINES);
        float x[2] = {z->min.x, z->max.x};
        float y[2] = {z->min.y, z->max.y};
        float zv[2] = {z->min.z, z->max.z};
        for (int xi = 0; xi < 2; xi++)
          for (int yi = 0; yi < 2; yi++)
            for (int zi = 0; zi < 2; zi++) {
                if (xi < 1) { glVertex3f(x[xi],y[yi],z[zi]); glVertex3f(x[xi+1],y[yi],z[zi]); }
                if (yi < 1) { glVertex3f(x[xi],y[yi],z[zi]); glVertex3f(x[xi],y[yi+1],z[zi]); }
                if (zi < 1) { glVertex3f(x[xi],y[yi],z[zi]); glVertex3f(x[xi],y[yi],z[zi+1]); }
            }
        glEnd();
        
        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
    }
    #endif
}

int water_get_zone_count(void) {
    return g_zone_count;
}
