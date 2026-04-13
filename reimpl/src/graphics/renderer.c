/*
 * Hamsterball OpenGL Renderer
 * Based on Graphics_Initialize at 0x455380
 * Graphics_BeginFrame at 0x453B50, Graphics_PresentOrEnd at 0x455A90
 */

#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>

#include "graphics/renderer.h"

static SDL_Window *g_window = NULL;
static SDL_GLContext g_gl_context = NULL;
static int g_width = 800;
static int g_height = 600;

/* Original projection: 10.0 near, 5000.0 far at Graphics_SetProjection 0x454AB0 */
#define NEAR_PLANE 10.0
#define FAR_PLANE 5000.0
#define FOV 45.0

bool renderer_init(int width, int height) {
    g_width = width;
    g_height = height;
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        fprintf(stderr, "[RENDERER] SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    g_window = SDL_CreateWindow("Hamsterball",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    
    if (!g_window) {
        fprintf(stderr, "[RENDERER] SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }
    
    g_gl_context = SDL_GL_CreateContext(g_window);
    if (!g_gl_context) {
        fprintf(stderr, "[RENDERER] SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        return false;
    }
    
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "[RENDERER] GLEW init failed: %s\n", glewGetErrorString(err));
        return false;
    }
    
    /* Enable vsync */
    SDL_GL_SetSwapInterval(1);
    
    /* Graphics_InitRenderStates at 0x42C810 */
    glClearColor(0.4f, 0.6f, 0.9f, 1.0f);  /* Sky blue */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
    /* Setup projection - Graphics_SetProjection at 0x454AB0 */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double aspect = (double)width / (double)height;
    gluPerspective(FOV, aspect, NEAR_PLANE, FAR_PLANE);
    glMatrixMode(GL_MODELVIEW);
    
    /* Lighting setup - Graphics_SetupLights at 0x454630 */
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat light_pos[] = {200.0f, 500.0f, 300.0f, 1.0f};
    GLfloat light_diff[] = {1.0f, 0.95f, 0.9f, 1.0f};
    GLfloat light_amb[] = {0.25f, 0.25f, 0.3f, 1.0f};
    GLfloat light_spec[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diff);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_amb);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_spec);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    printf("[RENDERER] Initialized OpenGL %s (%dx%d)\n", 
           glGetString(GL_VERSION), width, height);
    return true;
}

void renderer_shutdown(void) {
    if (g_gl_context) {
        SDL_GL_DeleteContext(g_gl_context);
        g_gl_context = NULL;
    }
    if (g_window) {
        SDL_DestroyWindow(g_window);
        g_window = NULL;
    }
    SDL_Quit();
}

void renderer_clear(void) {
    /* Graphics_BeginFrame at 0x453B50 */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void renderer_present(void) {
    /* Graphics_PresentOrEnd at 0x455A90 */
    SDL_GL_SwapWindow(g_window);
}

SDL_Window *renderer_get_window(void) {
    return g_window;
}

void renderer_set_camera(vec3_t pos, vec3_t target) {
    gluLookAt(pos.x, pos.y, pos.z,
              target.x, target.y, target.z,
              0.0, 1.0, 0.0);
}

void renderer_resize(int width, int height) {
    if (height == 0) height = 1;
    g_width = width;
    g_height = height;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOV, (double)width / (double)height, NEAR_PLANE, FAR_PLANE);
    glMatrixMode(GL_MODELVIEW);
}

void renderer_draw_sphere(float x, float y, float z, float radius,
                          float r, float g, float b, float a) {
    GLfloat diff[] = {r, g, b, a};
    GLfloat amb[] = {r * 0.3f, g * 0.3f, b * 0.3f, a};
    GLfloat spec[] = {0.6f, 0.6f, 0.6f, 1.0f};
    GLfloat shin[] = {40.0f};
    
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
    glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
    glMaterialfv(GL_FRONT, GL_SHININESS, shin);
    
    glPushMatrix();
    glTranslatef(x, y, z);
    
    static GLUquadric *quad = NULL;
    if (!quad) quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluSphere(quad, radius, 24, 16);
    
    glPopMatrix();
}