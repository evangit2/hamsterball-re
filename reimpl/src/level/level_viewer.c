/*
 * Hamsterball Level Viewer - First runnable test
 * Loads and displays MESHWORLD level data using SDL2 + OpenGL
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <GL/glew.h>

/* Include the parser */
#include "level/meshworld_parser.h"

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

static SDL_Window *window = NULL;
static SDL_GLContext gl_context = NULL;
static bool running = true;

/* Camera */
static float cam_x = 0.0f, cam_y = 0.0f, cam_z = 200.0f;
static float cam_angle_x = 30.0f, cam_angle_y = 0.0f;
static int mouse_dragging = 0;
static int last_mouse_x = 0, last_mouse_y = 0;

static void init_gl(void) {
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)WINDOW_WIDTH / WINDOW_HEIGHT, 1.0, 10000.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    /* Enable lighting */
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat light_pos[] = {100.0f, 200.0f, 300.0f, 1.0f};
    GLfloat light_diff[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat light_amb[] = {0.3f, 0.3f, 0.3f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diff);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_amb);
    
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

static void draw_objects(mw_level_t *level) {
    for (int i = 0; i < level->object_count; i++) {
        mw_object_t *obj = &level->objects[i];
        
        float x = obj->position.x;
        float y = obj->position.y;
        float z = obj->position.z;
        
        /* Choose color based on object type */
        float r, g, b, a;
        float size = 2.0f;
        
        switch (obj->type) {
            case MW_OBJ_START:
                r = 0.0f; g = 1.0f; b = 0.0f; a = 1.0f;  /* Green */
                size = 3.0f;
                break;
            case MW_OBJ_FLAG:
                r = 1.0f; g = 1.0f; b = 0.0f; a = 1.0f;  /* Yellow */
                size = 4.0f;
                break;
            case MW_OBJ_SAFESPOT:
                r = 0.0f; g = 0.8f; b = 0.8f; a = 0.5f;  /* Cyan, transparent */
                size = 3.0f;
                break;
            case MW_OBJ_CAMERALOOKAT:
                r = 1.0f; g = 0.0f; b = 1.0f; a = 1.0f;  /* Magenta */
                size = 2.0f;
                break;
            case MW_OBJ_PLATFORM:
            case MW_OBJ_SINKPLATFORM:
                r = 0.6f; g = 0.4f; b = 0.2f; a = 0.8f;  /* Brown */
                size = obj->size_param > 0 ? obj->size_param / 2.0f : 5.0f;
                break;
            case MW_OBJ_NODIZZY:
                r = 0.5f; g = 0.0f; b = 0.5f; a = 0.3f;  /* Purple, transparent */
                size = 5.0f;
                break;
            default:
                r = 0.5f; g = 0.5f; b = 0.5f; a = 0.5f;  /* Grey */
                size = 2.0f;
                break;
        }
        
        glColor4f(r, g, b, a);
        
        /* Draw a simple marker at each position */
        glPushMatrix();
        glTranslatef(x, y, z);
        
        /* Draw a cube */
        glBegin(GL_QUADS);
        float s = size / 2.0f;
        /* Front */
        glNormal3f(0, 0, 1);
        glVertex3f(-s, -s,  s); glVertex3f( s, -s,  s);
        glVertex3f( s,  s,  s); glVertex3f(-s,  s,  s);
        /* Back */
        glNormal3f(0, 0, -1);
        glVertex3f(-s, -s, -s); glVertex3f(-s,  s, -s);
        glVertex3f( s,  s, -s); glVertex3f( s, -s, -s);
        /* Top */
        glNormal3f(0, 1, 0);
        glVertex3f(-s,  s, -s); glVertex3f(-s,  s,  s);
        glVertex3f( s,  s,  s); glVertex3f( s,  s, -s);
        /* Bottom */
        glNormal3f(0, -1, 0);
        glVertex3f(-s, -s, -s); glVertex3f( s, -s, -s);
        glVertex3f( s, -s,  s); glVertex3f(-s, -s,  s);
        /* Right */
        glNormal3f(1, 0, 0);
        glVertex3f( s, -s, -s); glVertex3f( s,  s, -s);
        glVertex3f( s,  s,  s); glVertex3f( s, -s,  s);
        /* Left */
        glNormal3f(-1, 0, 0);
        glVertex3f(-s, -s, -s); glVertex3f(-s, -s,  s);
        glVertex3f(-s,  s,  s); glVertex3f(-s,  s, -s);
        glEnd();
        
        glPopMatrix();
    }
}

static void draw_grid(float size, float step) {
    glDisable(GL_LIGHTING);
    glColor4f(0.3f, 0.3f, 0.3f, 0.5f);
    
    glBegin(GL_LINES);
    for (float i = -size; i <= size; i += step) {
        glVertex3f(i, 0, -size);
        glVertex3f(i, 0, size);
        glVertex3f(-size, 0, i);
        glVertex3f(size, 0, i);
    }
    glEnd();
    
    glEnable(GL_LIGHTING);
}

static void draw_origin(void) {
    glDisable(GL_LIGHTING);
    glLineWidth(2.0f);
    
    glBegin(GL_LINES);
    /* X axis - Red */
    glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(50, 0, 0);
    /* Y axis - Green */
    glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, 50, 0);
    /* Z axis - Blue */
    glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 50);
    glEnd();
    
    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
}

static void render(mw_level_t *level) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    /* Camera */
    glTranslatef(0, 0, -cam_z);
    glRotatef(cam_angle_x, 1, 0, 0);
    glRotatef(cam_angle_y, 0, 1, 0);
    glTranslatef(-cam_x, -cam_y, 0);
    
    draw_grid(500.0f, 50.0f);
    draw_origin();
    draw_objects(level);
    
    SDL_GL_SwapWindow(window);
}

static void handle_events(void) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_ESCAPE) running = false;
                if (e.key.keysym.sym == SDLK_r) {
                    /* Reset camera */
                    cam_x = cam_y = 0; cam_z = 200;
                    cam_angle_x = 30; cam_angle_y = 0;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (e.button.button == 1) {
                    mouse_dragging = 1;
                    last_mouse_x = e.button.x;
                    last_mouse_y = e.button.y;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (e.button.button == 1) mouse_dragging = 0;
                break;
            case SDL_MOUSEMOTION:
                if (mouse_dragging) {
                    int dx = e.motion.x - last_mouse_x;
                    int dy = e.motion.y - last_mouse_y;
                    cam_angle_y += dx * 0.5f;
                    cam_angle_x += dy * 0.5f;
                    last_mouse_x = e.motion.x;
                    last_mouse_y = e.motion.y;
                }
                break;
            case SDL_MOUSEWHEEL:
                cam_z -= e.wheel.y * 20.0f;
                if (cam_z < 10) cam_z = 10;
                break;
        }
    }
    
    /* Keyboard camera movement */
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    float speed = 5.0f;
    if (keys[SDL_SCANCODE_W]) cam_y += speed;
    if (keys[SDL_SCANCODE_S]) cam_y -= speed;
    if (keys[SDL_SCANCODE_A]) cam_x -= speed;
    if (keys[SDL_SCANCODE_D]) cam_x += speed;
    if (keys[SDL_SCANCODE_Q]) cam_z -= speed * 2;
    if (keys[SDL_SCANCODE_E]) cam_z += speed * 2;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Hamsterball Level Viewer\n");
        printf("Usage: %s <file.MESHWORLD>\n", argv[0]);
        printf("Controls:\n");
        printf("  Mouse drag - Rotate camera\n");
        printf("  Mouse wheel - Zoom in/out\n");
        printf("  WASD - Move camera\n");
        printf("  QE - Zoom in/out\n");
        printf("  R - Reset camera\n");
        printf("  ESC - Quit\n");
        return 1;
    }
    
    /* Load the level */
    FILE *f = fopen(argv[1], "rb");
    if (!f) { perror("fopen"); return 1; }
    
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    uint8_t *data = malloc(size);
    fread(data, 1, size, f);
    fclose(f);
    
    printf("Loading %s (%zu bytes)\n", argv[1], size);
    mw_level_t *level = meshworld_parse(data, size);
    if (!level) {
        fprintf(stderr, "Failed to parse level\n");
        free(data);
        return 1;
    }
    
    /* Print level info */
    printf("Level has %d objects:\n", level->object_count);
    for (int i = 0; i < level->object_count; i++) {
        mw_object_t *obj = &level->objects[i];
        printf("  [%2d] %-30s pos=(%7.1f, %7.1f, %7.1f)",
               i, obj->type_string,
               obj->position.x, obj->position.y, obj->position.z);
        if (obj->texture[0])
            printf(" tex='%s'", obj->texture);
        printf("\n");
    }
    
    /* Center camera on first object */
    if (level->object_count > 0) {
        cam_x = level->objects[0].position.x;
        cam_y = level->objects[0].position.y;
        cam_z = 200.0f;
    }
    
    /* Initialize SDL + OpenGL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    window = SDL_CreateWindow("Hamsterball Level Viewer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    
    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        return 1;
    }
    
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "GLEW init failed: %s\n", glewGetErrorString(err));
        return 1;
    }
    
    init_gl();
    
    printf("\nLevel viewer running. Press ESC to quit.\n");
    
    /* Main loop */
    while (running) {
        handle_events();
        render(level);
        SDL_Delay(16);  /* ~60 FPS */
    }
    
    /* Cleanup */
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    meshworld_free(level);
    free(data);
    
    return 0;
}