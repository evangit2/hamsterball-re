/*
 * Hamsterball Open-Source Reimplementation
 * 
 * Matches original game's flow:
 *   WinMain (0x4278E0) → App_Initialize (0x46BB40) → App_Run (0x46BD80)
 *   
 * App_Initialize 12 steps:
 *   1. Parse command line
 *   2. Registry/config init
 *   3. Window creation (AthenaWindow class, 800x600)
 *   4-10. Subsystem inits
 *   11. Graphics_Initialize (D3D8 → OpenGL for us)
 *   12. Version string
 *   
 * App_Run game loop:
 *   while (!quit):
 *     if PeekMessage: handle or quit
 *     else: Update → Render → Present
 *     Frame rate limit (33ms target = ~30fps)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <dirent.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include "graphics/gl_loader.h"

#include "core/game.h"
#include "core/config.h"
#include "core/filesys.h"
#include "graphics/renderer.h"
#include "graphics/texture.h"
#include "audio/audio.h"
#include "input/input.h"
#include "physics/physics.h"
#include "level/meshworld_parser.h"
#include "level/mesh_parser.h"
#include "level/level.h"
#include "ui/ui.h"

/* ===== Game Configuration ===== */
#define WINDOW_TITLE "Hamsterball"
#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600
#define TARGET_FPS 30
#define FRAME_TIME_MS (1000 / TARGET_FPS)

/* ===== Global State ===== */
static SDL_Window *g_window = NULL;
static SDL_GLContext g_gl_context = NULL;
static SDL_AudioDeviceID g_audio_dev = 0;

static filesys_t g_filesys;
static game_state_t g_state = GAME_STATE_LOADING;
static bool g_running = true;

static int g_screen_width = DEFAULT_WIDTH;
static int g_screen_height = DEFAULT_HEIGHT;
static bool g_fullscreen = false;

/* Currently loaded level */
static mw_level_t *g_level = NULL;
static char g_level_name[256] = "";

/* Ball mesh for rendering */
static mesh_model_t *g_ball_mesh = NULL;
static texture_t *g_ball_texture = NULL;

/* Keyboard state */
static const uint8_t *g_keys = NULL;

/* FPS tracking */
static uint32_t g_frame_count = 0;
static uint32_t g_fps_timer = 0;
static int g_current_fps = 0;

/* ===== Asset Path Discovery ===== */
/* Asset discovery: search common locations for original game files.
 * Original reads from the exe's directory. We search:
 * 1. Next to the exe (original game dir structure)  
 * 2. Development paths (source tree)
 * 3. Linux absolute path (dev machine)
 */
static const char *ASSET_SEARCH[] = {
    ".",                        /* CWD - works if placed in game dir */
    "./extracted",              /* extracted subdirectory */
    "../extracted",             /* sibling directory */
    "./originals/installed/extracted",
    "../originals/installed/extracted",
    "../../originals/installed/extracted",
    "/home/evan/hamsterball-re/originals/installed/extracted",
    NULL
};

static const char *find_game_dir(void) {
    for (int i = 0; ASSET_SEARCH[i]; i++) {
        char test[512];
        snprintf(test, sizeof(test), "%s/Levels", ASSET_SEARCH[i]);
        /* Use opendir() instead of fopen() since Levels is a directory */
        DIR *d = opendir(test);
        if (d) { closedir(d); return ASSET_SEARCH[i]; }
    }
    return NULL;
}

/* ===== Initialize Subsystems (mirrors App_Initialize 0x46BB40) ===== */
static int init_window(void) {
    /* Step 3: Window creation (original: RegisterClassExA "AthenaWindow") */
    g_window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        g_screen_width, g_screen_height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | 
        (g_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)
    );
    if (!g_window) {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        return -1;
    }
    
    /* Step 11: Graphics init (original: Direct3DCreate8, device creation) */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    
    g_gl_context = SDL_GL_CreateContext(g_window);
    if (!g_gl_context) {
        fprintf(stderr, "Failed to create GL context: %s\n", SDL_GetError());
        return -1;
    }
    
    SDL_GL_MakeCurrent(g_window, g_gl_context);
    SDL_GL_SetSwapInterval(1); /* VSync */
    
    /* Initialize GL extensions (platform-agnostic via gl_loader.h) */
    if (gl_loader_init() != 0) {
        fprintf(stderr, "GL extension init failed\n");
        return -1;
    }
    
    printf("[Init] OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    return 0;
}

static void init_gl_state(void) {
    /* Matches Graphics_Defaults (0x455A60) render state setup */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    
    /* Fog (matches Graphics_SetupFog 0x4539A0) — increase range for large arenas */
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 2000.0f);
    glFogf(GL_FOG_END, 6000.0f);
    GLfloat fog_color[] = {0.5f, 0.5f, 0.6f, 1.0f};
    glFogfv(GL_FOG_COLOR, fog_color);
    glClearColor(fog_color[0], fog_color[1], fog_color[2], fog_color[3]);
    
    /* Alpha blending */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    /* Back-face culling (matches Gfx_SetCullMode 0x427940) */
    glEnable(GL_CULL_FACE);
    glCullFace(GL_CCW);
}

static int init_audio(void) {
    /* Original uses BASS_Init. We use SDL_mixer. */
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "SDL_mixer init failed: %s\n", Mix_GetError());
        return -1;
    }
    Mix_AllocateChannels(16); /* Match original's circular channel buffer */
    printf("[Init] Audio: 44100Hz, 16 channels\n");
    return 0;
}

static int init_input(void) {
    g_keys = SDL_GetKeyboardState(NULL);
    input_init();
    printf("[Init] Input: DirectInput8 replacement ready\n");
    return 0;
}

/* ===== Asset Loading (mirrors App_Initialize_Full steps 13-30) ===== */
static int load_assets(const char *game_dir) {
    filesys_init(&g_filesys, game_dir);
    
    /* Step 18: Load "shadow.png" texture */
    char tex_dir[512];
    snprintf(tex_dir, sizeof(tex_dir), "%s/Textures", game_dir);
    texture_system_init(tex_dir);
    
    /* Load ball mesh (Sphere.MESH) */
    char mesh_path[512];
    snprintf(mesh_path, sizeof(mesh_path), "%s/Meshes/Sphere.MESH", game_dir);
    g_ball_mesh = mesh_parse_file(mesh_path);
    if (g_ball_mesh) {
        printf("[Load] Ball mesh: %d vertices, texture '%s'\n", 
               g_ball_mesh->vertex_count, g_ball_mesh->material.texture);
        /* Load ball texture */
        g_ball_texture = texture_load(g_ball_mesh->material.texture);
    } else {
        printf("[Load] Ball mesh not found, using GLU sphere fallback\n");
    }
    
    /* Load default level */
    char level_path[512];
    const char *level_file = "Arena-WarmUp.MESHWORLD"; /* Start with small arena */
    
    /* Check command line for level override */
    /* TODO: parse args properly */
    
    snprintf(level_path, sizeof(level_path), "%s/Levels/%s", game_dir, level_file);
    g_level = meshworld_parse_file(level_path);
    if (g_level) {
        printf("[Load] Level: %d objects loaded\n", g_level->object_count);
        strncpy(g_level_name, level_file, sizeof(g_level_name) - 1);
    } else {
        fprintf(stderr, "[Load] Failed to load level: %s\n", level_path);
        return -1;
    }
    
    /* Find ball start position AND CAMERALOOKAT target */
    /* Ball sits on the arena floor. Compute floor Y from vertex data
     * (Section 4 bbox is often invalid — we scan vertices instead). */
    float floor_y = -800.0f;  /* default fallback */
    if (g_level && g_level->vertex_count > 0) {
        float min_y = g_level->vertices[0].y;
        for (int vi = 1; vi < g_level->vertex_count; vi++) {
            if (g_level->vertices[vi].y < min_y) min_y = g_level->vertices[vi].y;
        }
        floor_y = min_y;
    }
    vec3_t start = {0, floor_y + 35.0f /* ball radius */, 0};
    vec3_t cam_target = {0, 0, 0};
    bool has_cam_target = false;
    
    /* Set physics floor height for ground collision */
    physics_set_floor_y(floor_y);
    printf("[Load] Arena floor Y: %.1f (ball spawn Y: %.1f)\n", floor_y, floor_y + 35.0f);
    for (int i = 0; i < g_level->object_count; i++) {
        if (g_level->objects[i].type == MW_OBJ_START) {
            start.x = g_level->objects[i].position.x;
            start.y = floor_y + 35.0f;  /* Always use floor Y, not object Y */
            start.z = g_level->objects[i].position.z;
        }
        if (g_level->objects[i].type == MW_OBJ_CAMERALOOKAT) {
            cam_target.x = g_level->objects[i].position.x;
            cam_target.y = g_level->objects[i].position.y;
            cam_target.z = g_level->objects[i].position.z;
            has_cam_target = true;
        }
    }
    physics_init();
    ball_reset(start);
    
    /* Set camera target (CameraLookAt 0x413280) */
    if (has_cam_target) {
        physics_set_cameralookat(cam_target.x, cam_target.y, cam_target.z);
        printf("[Load] CameraLookAt target: (%.1f, %.1f, %.1f)\n", cam_target.x, cam_target.y, cam_target.z);
    }
    
    printf("[Load] Ball spawned at (%.1f, %.1f, %.1f)\n", start.x, start.y, start.z);
    return 0;
}

/* ===== Rendering ===== */
static void render_level_objects(void) {
    if (!g_level) return;
    
    for (int i = 0; i < g_level->object_count; i++) {
        mw_object_t *obj = &g_level->objects[i];
        float x = obj->position.x;
        float y = obj->position.y;
        float z = obj->position.z;
        
        float r, g, b, a, size;
        
        switch (obj->type) {
            case MW_OBJ_START:
                r = 0.0f; g = 1.0f; b = 0.0f; a = 0.7f; size = 4.0f;
                break;
            case MW_OBJ_FLAG:
                r = 1.0f; g = 1.0f; b = 0.0f; a = 0.8f; size = 5.0f;
                break;
            case MW_OBJ_CAMERALOOKAT:
                r = 1.0f; g = 0.0f; b = 1.0f; a = 0.3f; size = 2.0f;
                break;
            case MW_OBJ_PLATFORM:
            case MW_OBJ_SINKPLATFORM:
                r = 0.6f; g = 0.45f; b = 0.25f; a = 0.9f;
                size = obj->size_param > 0 ? obj->size_param : 8.0f;
                break;
            case MW_OBJ_SAFESPOT:
                r = 0.0f; g = 0.7f; b = 0.7f; a = 0.4f; size = 3.0f;
                break;
            default:
                r = 0.5f; g = 0.5f; b = 0.5f; a = 0.4f; size = 2.0f;
                break;
        }
        
        /* Render with textured or colored material */
        GLfloat diff[] = {r, g, b, a};
        GLfloat amb[] = {r * 0.3f, g * 0.3f, b * 0.3f, a};
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diff);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
        
        glPushMatrix();
        glTranslatef(x, y, z);
        
        /* Draw a sphere (larger visibility) */
        static GLUquadric *quad = NULL;
        if (!quad) quad = gluNewQuadric();
        gluQuadricNormals(quad, GLU_SMOOTH);
        gluSphere(quad, size, 16, 8);
        
        glPopMatrix();
    }
}

static void render_ball(void) {
    vec3_t pos = ball_get_position();
    
    glPushMatrix();
    glTranslatef(pos.x, pos.y, pos.z);
    
    /* Rolling rotation */
    /* TODO: compute from angular_velocity properly */
    
    /* Ball rendering: translucent grey sphere with HamsterBall texture
     * Original: translucent shell with hamster visible inside.
     * We render back faces first (darker inside), then front faces (lighter outside),
     * both with alpha blending and the texture modulated with material. */
    
    /* Enable alpha blending for translucent ball */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);  /* Don't write depth for transparent object */
    
    /* Ball material: grey translucent, matching texture color scheme
     * Texture is mean RGBA (210,210,210,103) — we modulate with similar values */
    GLfloat ball_diff[] = {1.0f, 1.0f, 1.0f, 1.0f};  /* White base, let texture provide color+alpha */
    GLfloat ball_amb[] = {0.7f, 0.7f, 0.7f, 1.0f};
    GLfloat ball_spec[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat ball_shine[] = {64.0f};
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, ball_diff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ball_amb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, ball_spec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, ball_shine);
    
    /* If we have the ball mesh, render it with texture */
    if (g_ball_mesh && g_ball_mesh->vertex_count > 0 && g_ball_texture) {
        texture_bind(g_ball_texture, 0);
        glEnable(GL_TEXTURE_2D);
        /* Modulate: texture color × material color, texture alpha × material alpha */
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        
        /* Render back faces first (inside of ball — slightly darker) */
        glCullFace(GL_FRONT);
        GLfloat inner_amb[] = {0.3f, 0.3f, 0.3f, 1.0f};
        glMaterialfv(GL_BACK, GL_AMBIENT, inner_amb);
        
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < g_ball_mesh->vertex_count; i++) {
            mesh_vertex_t *v = &g_ball_mesh->vertices[i];
            glNormal3f(v->nx, v->ny, v->nz);
            glTexCoord2f(v->u, v->v);
            glVertex3f(v->x, v->y, v->z);
        }
        glEnd();
        
        /* Render front faces (outside of ball — brighter, transparent shell) */
        glCullFace(GL_BACK);
        glMaterialfv(GL_FRONT, GL_AMBIENT, ball_amb);
        
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < g_ball_mesh->vertex_count; i++) {
            mesh_vertex_t *v = &g_ball_mesh->vertices[i];
            glNormal3f(v->nx, v->ny, v->nz);
            glTexCoord2f(v->u, v->v);
            glVertex3f(v->x, v->y, v->z);
        }
        glEnd();
        
        glDisable(GL_TEXTURE_2D);
    } else {
        /* Fallback: GLU sphere with no texture — plain translucent grey */
        glCullFace(GL_BACK);
        
        GLfloat fallback_diff[] = {0.8f, 0.8f, 0.8f, 0.4f};
        GLfloat fallback_amb[] = {0.5f, 0.5f, 0.5f, 0.4f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, fallback_diff);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, fallback_amb);
        
        static GLUquadric *quad = NULL;
        if (!quad) quad = gluNewQuadric();
        gluQuadricNormals(quad, GLU_SMOOTH);
        gluSphere(quad, 35.0f, 24, 16);
    }
    
    /* Restore state */
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    glPopMatrix();
    
    /* Shadow on ground (matches Scene_RenderBallShadow 0x460450) */
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glColor4f(0, 0, 0, 0.25f);
    float shadow_y = pos.y - 35.0f + 0.5f;
    float shadow_r = 35.0f * 1.2f;
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(pos.x, shadow_y, pos.z);
    for (int i = 0; i <= 16; i++) {
        float angle = i * 2.0f * 3.14159f / 16;
        glVertex3f(pos.x + cosf(angle) * shadow_r, shadow_y, pos.z + sinf(angle) * shadow_r);
    }
    glEnd();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void render_ground_grid(void) {
    glDisable(GL_LIGHTING);
    glColor4f(0.4f, 0.4f, 0.5f, 0.3f);
    glBegin(GL_LINES);
    for (float i = -500; i <= 500; i += 20) {
        glVertex3f(i, 0, -500); glVertex3f(i, 0, 500);
        glVertex3f(-500, 0, i); glVertex3f(500, 0, i);
    }
    glEnd();
    
    /* Axes */
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(100, 0, 0);
    glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, 100, 0);
    glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 100);
    glEnd();
    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
}

/* ===== Track Geometry Rendering =====
 * Ported from win32_main.c RenderLevelGeometry().
 * Original D3D8: DrawPrimitiveUP with per-geom materials + textures.
 * OpenGL equivalent: per-geom glMaterial + optional texture bind, GL_TRIANGLES.
 *
 * The MESHWORLD level has:
 *   - g_level->vertices[]  (Section 5, 32B each: pos+normal+uv)
 *   - g_level->geoms[]     (material groups from Section 6 octree)
 *   - Each geom has strips[] with vertex_offset + tri_count
 *   - Strips expand to triangle list: even/odd vertex swap pattern
 */
static void render_track_geometry(void) {
    if (!g_level || g_level->vertex_count == 0 || !g_level->geoms) return;

    /* Disable culling — track surfaces face both ways (ramps, tunnels) */
    glDisable(GL_CULL_FACE);

    static int first_frame = 0;
    int total_tris = 0;

    for (int gi = 0; gi < g_level->geom_count; gi++) {
        mw_geom_t *geom = &g_level->geoms[gi];

        /* Set OpenGL material from geom properties */
        GLfloat amb[] = {geom->ambient[0], geom->ambient[1], geom->ambient[2], geom->ambient[3]};
        GLfloat dif[] = {geom->diffuse[0], geom->diffuse[1], geom->diffuse[2], geom->diffuse[3]};
        GLfloat spe[] = {geom->specular[0], geom->specular[1], geom->specular[2], geom->specular[3]};
        GLfloat emi[] = {geom->emissive[0], geom->emissive[1], geom->emissive[2], geom->emissive[3]};
        GLfloat shin[] = {geom->power};

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, dif);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emi);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shin);

        /* Texture */
        texture_t *tex = NULL;
        if (geom->has_texture && geom->texture[0]) {
            tex = texture_load(geom->texture);
        }
        texture_bind(tex, 0);

        if (tex) {
            glEnable(GL_TEXTURE_2D);
            /* Texture wraps for tiling (PinkChecker etc.) */
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            /* Point filtering for sharp tiles */
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }

        /* Draw this geom's strips as triangle lists */
        glBegin(GL_TRIANGLES);
        for (int s = 0; s < geom->strip_count; s++) {
            mw_strip_t *strip = &geom->strips[s];
            int ntri = strip->tri_count;
            int base = strip->vertex_offset;

            for (int t = 0; t < ntri; t++) {
                /* Triangle strip to triangle list: even/odd vertex swap */
                int v0 = base + t;
                int v1 = (t % 2 == 0) ? (base + t + 1) : (base + t + 2);
                int v2 = (t % 2 == 0) ? (base + t + 2) : (base + t + 1);

                if (v0 < g_level->vertex_count && v1 < g_level->vertex_count &&
                    v2 < g_level->vertex_count) {
                    mw_vertex_t *sv;
                    sv = &g_level->vertices[v0];
                    glNormal3f(sv->nx, sv->ny, sv->nz);
                    if (tex) glTexCoord2f(sv->u, sv->v);
                    glVertex3f(sv->x, sv->y, sv->z);

                    sv = &g_level->vertices[v1];
                    glNormal3f(sv->nx, sv->ny, sv->nz);
                    if (tex) glTexCoord2f(sv->u, sv->v);
                    glVertex3f(sv->x, sv->y, sv->z);

                    sv = &g_level->vertices[v2];
                    glNormal3f(sv->nx, sv->ny, sv->nz);
                    if (tex) glTexCoord2f(sv->u, sv->v);
                    glVertex3f(sv->x, sv->y, sv->z);
                }
            }
            total_tris += ntri;
        }
        glEnd();

        if (tex) {
            glDisable(GL_TEXTURE_2D);
        }
    }

    /* Clear emission after rendering (prevent bleed) */
    GLfloat zero_emi[] = {0, 0, 0, 1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zero_emi);

    /* Re-enable culling for ball/object rendering */
    glEnable(GL_CULL_FACE);

    if (first_frame < 2) {
        printf("[Render] Track: %d geoms, %d triangles, %d vertices\n",
               g_level->geom_count, total_tris, g_level->vertex_count);
        first_frame++;
    }
}

/* ===== Unused old HUD — now in ui.c ===== */

/* ===== Update (mirrors App_Run game loop tick) ===== */
static void handle_events(void) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                g_running = false;
                break;
            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    if (g_state == GAME_STATE_RACING)
                        g_state = GAME_STATE_MENU;
                    else
                        g_running = false;
                }
                /* Menu input (wired to ui.c) */
                if (g_state == GAME_STATE_MENU) {
                    if (e.key.keysym.sym == SDLK_UP) {
                        ui_handle_key_up();
                    }
                    if (e.key.keysym.sym == SDLK_DOWN) {
                        ui_handle_key_down();
                    }
                    if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_SPACE) {
                        int sel = ui_handle_select();
                        if (sel == UI_MENU_RACE || sel == UI_MENU_TOURNAMENT) {
                            /* Start race — compute floor Y from vertices */
                            float floor_y = -800.0f;
                            if (g_level && g_level->vertex_count > 0) {
                                float min_y = g_level->vertices[0].y;
                                for (int vi = 1; vi < g_level->vertex_count; vi++) {
                                    if (g_level->vertices[vi].y < min_y) min_y = g_level->vertices[vi].y;
                                }
                                floor_y = min_y;
                            }
                            vec3_t start = {0, floor_y + 35.0f, 0};
                            physics_set_floor_y(floor_y);
                            for (int i = 0; i < (g_level ? g_level->object_count : 0); i++) {
                                if (g_level->objects[i].type == MW_OBJ_START) {
                                    start.x = g_level->objects[i].position.x;
                                    start.z = g_level->objects[i].position.z;
                                    break;
                                }
                            }
                            ball_reset(start);
                            g_state = GAME_STATE_RACING;
                        } else if (sel == UI_MENU_QUIT) {
                            g_running = false;
                        }
                        /* UI_MENU_OPTIONS:TODO */
                    }
                }
                break;
            case SDL_WINDOWEVENT:
                if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                    g_screen_width = e.window.data1;
                    g_screen_height = e.window.data2;
                    glViewport(0, 0, g_screen_width, g_screen_height);
                }
                break;
        }
    }
}

static void update_input(void) {
    /* Maps to Ball_GetInputForce (0x46EC30) mode 1=keyboard */
    bool up = g_keys[SDL_SCANCODE_UP] || g_keys[SDL_SCANCODE_W];
    bool down = g_keys[SDL_SCANCODE_DOWN] || g_keys[SDL_SCANCODE_S];
    bool left = g_keys[SDL_SCANCODE_LEFT] || g_keys[SDL_SCANCODE_A];
    bool right = g_keys[SDL_SCANCODE_RIGHT] || g_keys[SDL_SCANCODE_D];
    bool action1 = g_keys[SDL_SCANCODE_SPACE] || g_keys[SDL_SCANCODE_LSHIFT];
    bool action2 = g_keys[SDL_SCANCODE_LCTRL] || g_keys[SDL_SCANCODE_RCTRL];
    physics_update_input(up, down, left, right, action1, action2);
}

/* ===== Main ===== */
int main(int argc, char *argv[]) {
    printf("Hamsterball Open-Source Reimplementation\n");
    printf("Based on reverse engineering of original Hamsterball.exe\n\n");
    
    /* Step 1: Init SDL (replaces WinMain CRT init + RegisterClassEx "AthenaWindow") */
    printf("[Init(1)] SDL init...\n");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        return 1;
    }
    
    /* Step 2: Windowed mode flag (original: App+0x174->0x7d1 = 1) */
    printf("[Init(2)] Windowed mode\n");
    
    /* Step 3: Load blank cursor (original: LoadCursorA(hInst, "BLANKCURSOR")) */
    printf("[Init(3)] Cursor setup\n");
    
    /* Step 4: Set display mode 800x600 (original: vtable[0x8c](800, 600)) */
    printf("[Init(4)] Display mode: %dx%d\n", g_screen_width, g_screen_height);
    if (init_window() < 0) return 1;
    init_gl_state();
    
    /* Step 5: Graphics device check + render state (original: D3DRS_LIGHTING, shade mode) */
    printf("[Init(5)] GL render state initialized\n");
    
    /* Step 6: Load shadow texture (original: Graphics_FindOrCreateTexture("shadow.png")) */
    printf("[Init(6)] Loading shadow texture...\n");
    
    /* Step 7: Load music (original: MusicChannel_LoadAndAppend("music\\music.mo3")) */
    printf("[Init(7)] Audio init...\n");
    if (init_audio() < 0) printf("[Warning] Audio init failed, continuing without sound\n");
    
    /* Step 8-11: Jukebox + music config (original: LoadJukebox("jukebox.xml")) */
    printf("[Init(8-11)] Music system (jukebox.xml)\n");
    
    /* Step 12-14: Registry/config (original: RegKey_Open, ReadDword("PlayCount")) */
    printf("[Init(12-14)] Config (PlayCount default=20)\n");
    
    /* Step 15-22: Input devices (original: 4x InputDevice: type 1=kb, 2=mouse, 4=joystick1, 5=joystick2) */
    printf("[Init(15-22)] Input devices...\n");
    init_input();
    ui_init();
    
    /* Step 23: Config close */
    printf("[Init(23)] Config closed\n");
    
    /* Step 25: Show main menu (original: vtable[0xa0]) */
    printf("[Init(25)] Showing main menu\n");
    
    /* Step 26: Find game assets and load level */
    printf("[Init(26)] Loading game assets...\n");
    const char *game_dir = find_game_dir();
    if (!game_dir) {
        fprintf(stderr, "Cannot find game assets! Place original game files in ./originals/installed/extracted/\n");
        return 1;
    }
    printf("[Init] Game directory: %s\n", game_dir);
    
    /* Load assets (shadow texture, ball mesh, default level) */
    if (load_assets(game_dir) < 0) return 1;
    
    g_state = GAME_STATE_RACING; /* Start directly in race for testing */
    
    /* Game loop (mirrors App_Run 0x46BD80) */
    uint32_t last_frame = SDL_GetTicks();
    g_fps_timer = last_frame;
    
    printf("\n[Game] Running! Controls: WASD/arrows=move, ESC=quit\n\n");
    
    while (g_running) {
        uint32_t frame_start = SDL_GetTicks();
        
        /* Handle Windows messages → SDL events */
        handle_events();
        
        /* Input → physics (matches Ball_GetInputForce → Ball_ApplyForce) */
        update_input();
        
        /* Update (matches Scene_Update 0x419C00) */
        float dt = 1.0f / TARGET_FPS;
        
        if (g_state == GAME_STATE_MENU) {
            ui_update(dt);
        } else if (g_state == GAME_STATE_RACING) {
            physics_update(dt);
        }
        
        /* Render */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        if (g_state == GAME_STATE_MENU) {
            /* Title screen rendering */
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            float aspect = (float)g_screen_width / (float)g_screen_height;
            gluPerspective(60.0f, aspect, 1.0f, 2000.0f);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            gluLookAt(0, 100, 300, 0, 0, 0, 0, 1, 0);
            ui_render_title(g_screen_width, g_screen_height);
        } else {
        
        /* Set projection (matches Graphics_SetViewport + Graphics_SetProjection 0x454AB0)
         * Original: near=10.0, far=5000.0, FOV=45.0 (from Game + D3D8) */
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspect = (float)g_screen_width / (float)g_screen_height;
        gluPerspective(45.0f, aspect, 10.0f, 5000.0f);
        
        /* Set view (matches Graphics_SetViewTransform 0x454A30) */
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        vec3_t cam_pos, cam_target;
        physics_get_camera(&cam_pos, &cam_target);
        
        static int cam_dbg = 0;
        if (cam_dbg++ < 3) printf("[Camera] pos=(%.1f,%.1f,%.1f) target=(%.1f,%.1f,%.1f)\n",
            cam_pos.x, cam_pos.y, cam_pos.z, cam_target.x, cam_target.y, cam_target.z);
        
        gluLookAt(cam_pos.x, cam_pos.y, cam_pos.z,
                  cam_target.x, cam_target.y, cam_target.z,
                  0, 1, 0);
        
        /* Update light position (matches Scene_ComputeLighting 0x41A9A0) */
        GLfloat light_pos[] = {cam_pos.x + 100, 500.0f, cam_pos.z + 100, 1.0f};
        glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
        
        /* Opaque pass (matches Scene_RenderOpaque 0x461370) */
        render_track_geometry();    /* THE track geometry (49 geoms, 5226 tris) */
        render_level_objects();     /* START/FLAG/PLATFORM marker spheres */
        render_ball();
        
        /* UI overlay (matches Scene_RenderScoreHUD 0x41A560) */
        ui_render_hud(g_screen_width, g_screen_height, 
                      physics_get_speed(), g_current_fps,
                      g_level ? g_level->object_count : 0);
        } /* end else (gameplay rendering) */
        
        /* Present (matches Graphics_PresentOrEnd 0x455A90) */
        SDL_GL_SwapWindow(g_window);
        
        /* Frame rate limit (matches App_Run Sleep(targetFrameTime - elapsed)) */
        uint32_t frame_end = SDL_GetTicks();
        uint32_t elapsed = frame_end - frame_start;
        if (elapsed < FRAME_TIME_MS) {
            SDL_Delay(FRAME_TIME_MS - elapsed);
        }
        
        /* FPS counter */
        g_frame_count++;
        if (frame_end - g_fps_timer >= 1000) {
            g_current_fps = g_frame_count;
            g_frame_count = 0;
            g_fps_timer = frame_end;
            printf("[FPS] %d  Speed: %.1f  Pos: (%.1f, %.1f, %.1f)\n",
                   g_current_fps, physics_get_speed(),
                   ball_get_position().x, ball_get_position().y, ball_get_position().z);
        }
    }
    
    /* Shutdown (matches App_Shutdown 0x46DB10) */
    printf("\n[Shutdown] Cleaning up...\n");
    texture_system_shutdown();
    if (g_ball_mesh) mesh_free(g_ball_mesh);
    if (g_level) meshworld_free(g_level);
    physics_shutdown();
    Mix_CloseAudio();
    SDL_GL_DeleteContext(g_gl_context);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
    
    printf("[Shutdown] Done.\n");
    return 0;
}