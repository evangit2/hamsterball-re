/*
 * Hamsterball Open-Source Reimplementation
 * Main entry point and core game loop
 * 
 * Based on reverse engineering of the original Hamsterball.exe
 * Engine: "Athena" by Raptisoft
 * 
 * WinMain at 0x4278E0, App_Run at 0x46BD80
 * Game loop: PeekMessage → Input → Update → Render → Present
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include "core/game.h"
#include "core/config.h"
#include "graphics/renderer.h"
#include "audio/audio.h"
#include "input/input.h"
#include "physics/physics.h"
#include "level/meshworld_parser.h"
#include "level/level.h"
#include "ui/ui.h"

/* Global game state */
game_state_t g_game;
config_t g_config;

/* Screen dimensions */
static int g_screen_width = 800;
static int g_screen_height = 600;

/* Level loaded for viewing */
static mw_level_t *g_level = NULL;

/* Find START object in level for ball spawn */
static vec3_t find_start_position(mw_level_t *level) {
    vec3_t start = {0, 10.0f, 0};
    for (int i = 0; i < level->object_count; i++) {
        if (level->objects[i].type == MW_OBJ_START) {
            start.x = level->objects[i].position.x;
            start.y = level->objects[i].position.y + 10.0f;
            start.z = level->objects[i].position.z;
            return start;
        }
    }
    /* Fallback: use first object position */
    if (level->object_count > 0) {
        start.x = level->objects[0].position.x;
        start.y = level->objects[0].position.y + 10.0f;
        start.z = level->objects[0].position.z;
    }
    return start;
}

/* Draw level objects as markers (same as level_viewer.c) */
static void render_level_objects(mw_level_t *level) {
    for (int i = 0; i < level->object_count; i++) {
        mw_object_t *obj = &level->objects[i];
        
        float x = obj->position.x;
        float y = obj->position.y;
        float z = obj->position.z;
        
        float r, g, b, a;
        float size = 2.0f;
        
        switch (obj->type) {
            case MW_OBJ_START:
                r = 0.0f; g = 1.0f; b = 0.0f; a = 1.0f;
                size = 3.0f;
                break;
            case MW_OBJ_GOAL:
            case MW_OBJ_FLAG:
                r = 1.0f; g = 1.0f; b = 0.0f; a = 1.0f;
                size = 4.0f;
                break;
            case MW_OBJ_CAMERALOOKAT:
                r = 1.0f; g = 0.0f; b = 1.0f; a = 1.0f;
                size = 2.0f;
                break;
            case MW_OBJ_PLATFORM:
            case MW_OBJ_SINKPLATFORM:
                r = 0.6f; g = 0.4f; b = 0.2f; a = 0.8f;
                size = obj->size_param > 0 ? obj->size_param / 2.0f : 5.0f;
                break;
            case MW_OBJ_SAFESPOT:
                r = 0.0f; g = 0.8f; b = 0.8f; a = 0.5f;
                size = 3.0f;
                break;
            default:
                r = 0.5f; g = 0.5f; b = 0.5f; a = 0.5f;
                size = 2.0f;
                break;
        }
        
        GLfloat diff[] = {r, g, b, a};
        GLfloat amb[] = {r * 0.3f, g * 0.3f, b * 0.3f, a};
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diff);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
        
        glPushMatrix();
        glTranslatef(x, y, z);
        
        /* Draw a simple cube */
        glBegin(GL_QUADS);
        float s = size / 2.0f;
        glNormal3f(0, 0, 1);
        glVertex3f(-s, -s,  s); glVertex3f( s, -s,  s);
        glVertex3f( s,  s,  s); glVertex3f(-s,  s,  s);
        glNormal3f(0, 0, -1);
        glVertex3f(-s, -s, -s); glVertex3f(-s,  s, -s);
        glVertex3f( s,  s, -s); glVertex3f( s, -s, -s);
        glNormal3f(0, 1, 0);
        glVertex3f(-s,  s, -s); glVertex3f(-s,  s,  s);
        glVertex3f( s,  s,  s); glVertex3f( s,  s, -s);
        glNormal3f(0, -1, 0);
        glVertex3f(-s, -s, -s); glVertex3f( s, -s, -s);
        glVertex3f( s, -s,  s); glVertex3f(-s, -s,  s);
        glNormal3f(1, 0, 0);
        glVertex3f( s, -s, -s); glVertex3f( s,  s, -s);
        glVertex3f( s,  s,  s); glVertex3f( s, -s,  s);
        glNormal3f(-1, 0, 0);
        glVertex3f(-s, -s, -s); glVertex3f(-s, -s,  s);
        glVertex3f(-s,  s,  s); glVertex3f(-s,  s, -s);
        glEnd();
        
        glPopMatrix();
    }
}

/* Draw ground grid */
static void render_grid(float size, float step) {
    glDisable(GL_LIGHTING);
    glColor4f(0.4f, 0.4f, 0.5f, 0.5f);
    
    glBegin(GL_LINES);
    for (float i = -size; i <= size; i += step) {
        glVertex3f(i, 0, -size);
        glVertex3f(i, 0, size);
        glVertex3f(-size, 0, i);
        glVertex3f(size, 0, i);
    }
    glEnd();
    
    /* Axes */
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(50, 0, 0);
    glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, 50, 0);
    glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 50);
    glEnd();
    glLineWidth(1.0f);
    
    glEnable(GL_LIGHTING);
}

/* Draw 2D HUD overlay */
static void render_hud(void) {
    /* Switch to 2D */
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, g_screen_width, g_screen_height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    /* Speed indicator bar */
    float speed = physics_get_speed();
    float bar_w = speed * 10.0f;
    if (bar_w > 200.0f) bar_w = 200.0f;
    
    glColor4f(0.2f, 0.8f, 0.2f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(10, 10); glVertex2f(10 + bar_w, 10);
    glVertex2f(10 + bar_w, 25); glVertex2f(10, 25);
    glEnd();
    
    /* Speed text background */
    glColor4f(0, 0, 0, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(8, 8); glVertex2f(218, 8);
    glVertex2f(218, 27); glVertex2f(8, 27);
    glEnd();
    
    /* Restore 3D */
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

/* Forward declarations */
static bool game_init(const char *level_path);
static void game_shutdown(void);
static void game_loop(void);
static void game_render(void);

/*
 * Main entry point - corresponds to WinMain at 0x004BB4C8
 */
int main(int argc, char *argv[]) {
    printf("Hamsterball Open-Source Reimplementation\n");
    printf("Based on reverse engineering of Raptisoft's Athena engine\n\n");
    
    const char *level_path = NULL;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [options] <level.MESHWORLD>\n", argv[0]);
            printf("Options:\n");
            printf("  --help     Show this help\n");
            printf("  --debug    Enable debug output\n");
            printf("\nControls:\n");
            printf("  Arrow keys - Roll the ball\n");
            printf("  Space      - Action button\n");
            printf("  Shift      - Brake\n");
            printf("  R          - Reset ball\n");
            printf("  ESC        - Quit\n");
            return 0;
        }
        if (strcmp(argv[i], "--debug") == 0) {
            g_game.debug = true;
        } else {
            level_path = argv[i];
        }
    }
    
    if (!game_init(level_path)) {
        fprintf(stderr, "Failed to initialize game\n");
        return 1;
    }
    
    game_loop();
    game_shutdown();
    return 0;
}

static bool game_init(const char *level_path) {
    memset(&g_game, 0, sizeof(game_state_t));
    memset(&g_config, 0, sizeof(config_t));
    
    printf("[INIT] Starting game initialization...\n");
    
    /* Initialize renderer (Graphics::Initialize at 0x455380) */
    if (!renderer_init(g_screen_width, g_screen_height)) {
        fprintf(stderr, "Failed to initialize renderer\n");
        return false;
    }
    
    /* Initialize input (DirectInput8Create at Input_Init 0x46EE10) */
    if (!input_init()) {
        fprintf(stderr, "Warning: Failed to initialize input\n");
    }
    
    /* Initialize physics */
    physics_init();
    
    /* Load level if provided */
    if (level_path) {
        FILE *f = fopen(level_path, "rb");
        if (!f) {
            fprintf(stderr, "Cannot open level: %s\n", level_path);
        } else {
            fseek(f, 0, SEEK_END);
            size_t size = ftell(f);
            fseek(f, 0, SEEK_SET);
            uint8_t *data = malloc(size);
            fread(data, 1, size, f);
            fclose(f);
            
            g_level = meshworld_parse(data, size);
            free(data);
            
            if (g_level) {
                printf("[INIT] Level loaded: %d objects\n", g_level->object_count);
                vec3_t start = find_start_position(g_level);
                ball_reset(start);
            } else {
                fprintf(stderr, "Failed to parse level\n");
            }
        }
    }
    
    if (!g_level) {
        /* No level - just put ball at origin */
        vec3_t start = {0, 10.0f, 0};
        ball_reset(start);
        printf("[INIT] No level loaded, ball at origin\n");
    }
    
    g_game.running = true;
    g_game.state = GAME_STATE_RACE;
    
    /* Initialize camera to look at ball */
    vec3_t ball_pos = ball_get_position();
    /* Position camera above and behind */
    /* (Physics camera will smooth towards this) */
    
    printf("[INIT] Game initialization complete\n");
    return true;
}

static void game_shutdown(void) {
    printf("[SHUTDOWN] Cleaning up...\n");
    
    if (g_level) {
        meshworld_free(g_level);
        g_level = NULL;
    }
    
    physics_shutdown();
    input_shutdown();
    renderer_shutdown();
}

static void game_loop(void) {
    /* Based on App_Run at 0x46BD80 */
    /* Original: PeekMessage → vtable.Update → vtable.Render → Present */
    
    uint32_t last_time = SDL_GetTicks();
    int frame_count = 0;
    uint32_t fps_time = SDL_GetTicks();
    
    printf("[LOOP] Entering main game loop\n");
    
    while (g_game.running) {
        /* Process events (PeekMessage equivalent) */
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    g_game.running = false;
                    break;
                case SDL_KEYDOWN:
                    if (e.key.keysym.sym == SDLK_ESCAPE) g_game.running = false;
                    if (e.key.keysym.sym == SDLK_r) {
                        /* Reset ball to start position */
                        vec3_t start = {0, 10.0f, 0};
                        if (g_level) start = find_start_position(g_level);
                        ball_reset(start);
                    }
                    break;
                case SDL_WINDOWEVENT:
                    if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                        renderer_resize(e.window.data1, e.window.data2);
                    }
                    break;
            }
        }
        
        /* Calculate delta time */
        uint32_t current_time = SDL_GetTicks();
        float delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;
        if (delta_time > 0.1f) delta_time = 0.1f;
        
        /* Update input (DirectInput polling) */
        input_update();
        
        /* Update physics (Ball_Update at 0x405190) */
        bool up, down, left, right, action1, action2;
        input_get_all_keys(&up, &down, &left, &right, &action1, &action2);
        physics_update_input(up, down, left, right, action1, action2);
        physics_update(delta_time);
        
        /* Render frame */
        game_render();
        
        /* FPS counter */
        frame_count++;
        if (current_time - fps_time >= 1000) {
            if (g_game.debug) {
                printf("[FPS] %d fps, ball at (%.1f, %.1f, %.1f) speed=%.2f\n",
                       frame_count,
                       ball_get_position().x,
                       ball_get_position().y, 
                       ball_get_position().z,
                       physics_get_speed());
            }
            frame_count = 0;
            fps_time = current_time;
        }
    }
}

static void game_render(void) {
    /* Graphics_BeginFrame at 0x453B50 */
    renderer_clear();
    
    /* Set camera from physics */
    vec3_t cam_pos, cam_target;
    physics_get_camera(&cam_pos, &cam_target);
    renderer_set_camera(cam_pos, cam_target);
    
    /* Draw level geometry */
    render_grid(500.0f, 50.0f);
    if (g_level) render_level_objects(g_level);
    
    /* Render ball (Ball_Render at 0x402860) */
    physics_render();
    
    /* HUD overlay */
    render_hud();
    
    /* Graphics_PresentOrEnd at 0x455A90 */
    renderer_present();
}