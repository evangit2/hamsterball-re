/*
 * Hamsterball Level Viewer v2 - Real Geometry Renderer
 * Loads .MESH models and .MESHWORLD level data to render 3D levels.
 * 
 * Controls:
 *   WASD + mouse = fly around
 *   +/- = zoom
 *   L = toggle wireframe
 *   T = toggle textures
 *   ESC = quit
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dirent.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/glew.h>

#include "level/mesh_parser.h"
#include "level/meshworld_parser.h"

/* Camera state */
typedef struct {
    float pos[3];     /* eye position */
    float yaw, pitch;  /* Euler angles */
    float speed;
    float sensitivity;
} camera_t;

/* Renderable mesh instance */
typedef struct {
    mesh_model_t *mesh;
    float position[3];
    float rotation[4]; /* quaternion or euler */
    float scale;
    char type_string[256];
    GLuint texture_id;
    int highlight;     /* draw with special color */
} mesh_instance_t;

/* Level data */
typedef struct {
    char name[256];
    float min_bbox[3];
    float max_bbox[3];
    mesh_instance_t *instances;
    int instance_count;
    int instance_capacity;
    mesh_model_t *ball_mesh;      /* Sphere.MESH for the ball */
    float ball_position[3];        /* start position from level data */
    float *level_vertices;         /* raw level geometry from MESHWORLD */
    int level_vertex_count;
} level_data_t;

/* Global state */
static SDL_Window *window = NULL;
static SDL_GLContext gl_context = NULL;
static int running = 1;
static int wireframe = 0;
static int show_textures = 1;
static int width = 1280, height = 720;
static camera_t camera;
static level_data_t level;

/* Initialize camera */
static void camera_init(camera_t *c) {
    memset(c, 0, sizeof(*c));
    c->pos[1] = 100.0f;
    c->pos[2] = 200.0f;
    c->yaw = -90.0f;
    c->pitch = -30.0f;
    c->speed = 100.0f;
    c->sensitivity = 0.2f;
}

/* Get camera forward/right/up vectors */
static void camera_vectors(const camera_t *c, float fwd[3], float right[3], float up[3]) {
    float yaw_r = c->yaw * M_PI / 180.0f;
    float pitch_r = c->pitch * M_PI / 180.0f;
    
    fwd[0] = cosf(pitch_r) * cosf(yaw_r);
    fwd[1] = sinf(pitch_r);
    fwd[2] = cosf(pitch_r) * sinf(yaw_r);
    
    right[0] = cosf(yaw_r);
    right[1] = 0;
    right[2] = sinf(yaw_r);
    
    /* up = right x forward */
    up[0] = right[1]*fwd[2] - right[2]*fwd[1];
    up[1] = right[2]*fwd[0] - right[0]*fwd[2];
    up[2] = right[0]*fwd[1] - right[1]*fwd[0];
}

/* Load a texture from file */
static GLuint load_texture(const char *path) {
    if (!show_textures) return 0;
    
    /* Try multiple path variations */
    char fullpath[512];
    const char *base_dir = getenv("MESH_DIR");
    if (!base_dir) base_dir = "textures";
    
    /* Try direct path first */
    SDL_Surface *surf = IMG_Load(path);
    if (!surf) {
        /* Try with textures directory */
        snprintf(fullpath, sizeof(fullpath), "%s/%s", base_dir, path);
        surf = IMG_Load(fullpath);
    }
    if (!surf) {
        /* Try with the original installed textures dir */
        snprintf(fullpath, sizeof(fullpath), 
                 "%s/originals/installed/extracted/Textures/%s",
                 getenv("HOME") ? "" : ".", path);
        surf = IMG_Load(fullpath);
    }
    if (!surf) return 0;
    
    /* Convert to RGBA */
    SDL_Surface *rgba = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surf);
    if (!rgba) return 0;
    
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba->w, rgba->h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, rgba->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    SDL_FreeSurface(rgba);
    return tex;
}

/* Render a mesh */
static void render_mesh(const mesh_model_t *mesh, GLuint texture,
                        float pos[3], float rot[4], float scale,
                        int highlight) {
    if (!mesh || mesh->vertex_count == 0) return;
    
    glPushMatrix();
    glTranslatef(pos[0], pos[1], pos[2]);
    if (scale != 1.0f && scale != 0.0f) glScalef(scale, scale, scale);
    
    if (highlight) {
        glColor3f(1.0f, 0.3f, 0.3f);
    } else if (mesh->material.diffuse[0] > 0 || mesh->material.diffuse[1] > 0) {
        glColor4f(mesh->material.diffuse[0], mesh->material.diffuse[1],
                   mesh->material.diffuse[2], mesh->material.diffuse[3]);
    } else {
        glColor3f(0.7f, 0.7f, 0.7f);
    }
    
    if (texture && show_textures) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);
    }
    
    /* Render vertices as points/triangles */
    glBegin(wireframe ? GL_LINES : GL_TRIANGLES);
    for (int i = 0; i < mesh->vertex_count; i++) {
        const mesh_vertex_t *v = &mesh->vertices[i];
        
        /* Normal */
        if (!wireframe) {
            glNormal3f(v->nx, v->ny, v->nz);
        }
        
        /* Texture coordinate */
        if (texture && show_textures) {
            glTexCoord2f(v->u, v->v);
        }
        
        /* Position */
        glVertex3f(v->x, v->y, v->z);
        
        /* In wireframe mode, connect to next vertex */
        if (wireframe && i + 1 < mesh->vertex_count) {
            const mesh_vertex_t *v2 = &mesh->vertices[i + 1];
            glVertex3f(v2->x, v2->y, v2->z);
        }
    }
    glEnd();
    
    /* Also draw as points for visibility */
    if (!wireframe) {
        glPointSize(3.0f);
        glColor3f(highlight ? 1.0f : 0.8f, highlight ? 1.0f : 0.8f, 0.0f);
        glBegin(GL_POINTS);
        for (int i = 0; i < mesh->vertex_count; i++) {
            const mesh_vertex_t *v = &mesh->vertices[i];
            glVertex3f(v->x, v->y, v->z);
        }
        glEnd();
    }
    
    if (texture && show_textures) {
        glDisable(GL_TEXTURE_2D);
    }
    
    glPopMatrix();
}

/* Render the ball at start position */
static void render_ball(void) {
    if (!level.ball_mesh) return;
    GLuint tex = load_texture(level.ball_mesh->material.texture);
    render_mesh(level.ball_mesh, tex, level.ball_position, NULL, 1.0f, 1);
}

/* Render level objects */
static void render_instances(void) {
    for (int i = 0; i < level.instance_count; i++) {
        mesh_instance_t *inst = &level.instances[i];
        if (inst->mesh) {
            render_mesh(inst->mesh, inst->texture_id,
                       inst->position, inst->rotation, inst->scale,
                       inst->highlight);
        }
    }
}

/* Set up OpenGL projection */
static void setup_projection(void) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)width / (float)height;
    gluPerspective(60.0f, aspect, 1.0f, 5000.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    float fwd[3], right[3], up[3];
    camera_vectors(&camera, fwd, right, up);
    
    float lookat[3];
    lookat[0] = camera.pos[0] + fwd[0];
    lookat[1] = camera.pos[1] + fwd[1];
    lookat[2] = camera.pos[2] + fwd[2];
    
    gluLookAt(camera.pos[0], camera.pos[1], camera.pos[2],
              lookat[0], lookat[1], lookat[2],
              up[0], up[1], up[2]);
}

/* Draw grid and axes */
static void draw_grid(void) {
    glColor4f(0.3f, 0.3f, 0.3f, 0.5f);
    glBegin(GL_LINES);
    for (int i = -500; i <= 500; i += 50) {
        glVertex3f(i, 0, -500);
        glVertex3f(i, 0, 500);
        glVertex3f(-500, 0, i);
        glVertex3f(500, 0, i);
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
}

/* Draw start position marker */
static void draw_start_marker(float x, float y, float z) {
    /* Draw a cross at start position */
    float s = 15.0f;
    glColor3f(0, 1, 0);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex3f(x-s, y, z); glVertex3f(x+s, y, z);
    glVertex3f(x, y-s, z); glVertex3f(x, y+s, z);
    glVertex3f(x, y, z-s); glVertex3f(x, y, z+s);
    glEnd();
    
    /* Sphere around start */
    glColor4f(0, 1, 0, 0.3f);
    glPushMatrix();
    glTranslatef(x, y, z);
    
    /* Draw a circle */
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 32; i++) {
        float a = i * 2.0f * M_PI / 32.0f;
        glVertex3f(cosf(a) * s, 0, sinf(a) * s);
    }
    glEnd();
    
    glPopMatrix();
    glLineWidth(1.0f);
}

/* Input handling */
static void handle_input(float dt) {
    SDL_Event event;
    int mouse_dx = 0, mouse_dy = 0;
    
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            running = 0;
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: running = 0; break;
            case SDLK_l: wireframe = !wireframe; break;
            case SDLK_t: show_textures = !show_textures; break;
            }
            break;
        case SDL_MOUSEMOTION:
            if (event.motion.state & SDL_BUTTON_LMASK) {
                mouse_dx += event.motion.xrel;
                mouse_dy += event.motion.yrel;
            }
            break;
        }
    }
    
    /* Mouse look */
    camera.yaw += mouse_dx * camera.sensitivity;
    camera.pitch -= mouse_dy * camera.sensitivity;
    if (camera.pitch > 89.0f) camera.pitch = 89.0f;
    if (camera.pitch < -89.0f) camera.pitch = -89.0f;
    
    /* Keyboard movement */
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    float fwd[3], right[3], up[3];
    camera_vectors(&camera, fwd, right, up);
    float speed = camera.speed * dt;
    
    if (keys[SDL_SCANCODE_W]) { camera.pos[0] += fwd[0]*speed; camera.pos[1] += fwd[1]*speed; camera.pos[2] += fwd[2]*speed; }
    if (keys[SDL_SCANCODE_S]) { camera.pos[0] -= fwd[0]*speed; camera.pos[1] -= fwd[1]*speed; camera.pos[2] -= fwd[2]*speed; }
    if (keys[SDL_SCANCODE_A]) { camera.pos[0] -= right[0]*speed; camera.pos[1] = 0; camera.pos[2] -= right[2]*speed; }
    if (keys[SDL_SCANCODE_D]) { camera.pos[0] += right[0]*speed; camera.pos[1] = 0; camera.pos[2] += right[2]*speed; }
    if (keys[SDL_SCANCODE_SPACE]) { camera.pos[1] += speed; }
    if (keys[SDL_SCANCODE_LSHIFT]) { camera.pos[1] -= speed; }
    if (keys[SDL_SCANCODE_EQUALS] || keys[SDL_SCANCODE_KP_PLUS]) { camera.speed *= 1.1f; }
    if (keys[SDL_SCANCODE_MINUS] || keys[SDL_SCANCODE_KP_MINUS]) { camera.speed *= 0.9f; }
}

/* Load a MESH file, trying multiple paths */
static mesh_model_t *load_mesh(const char *name) {
    char path[512];
    const char *home = getenv("HOME") ? getenv("HOME") : "/root";
    const char *dirs[] = {
        "~/hamsterball-re/originals/installed/extracted/Meshes",
        "./Meshes",
        "../Meshes",
        NULL
    };
    
    for (int i = 0; dirs[i]; i++) {
        if (dirs[i][0] == '~') {
            snprintf(path, sizeof(path), "%s%s/%s.MESH", home, dirs[i]+1, name);
        } else {
            snprintf(path, sizeof(path), "%s/%s.MESH", dirs[i], name);
        }
        
        mesh_model_t *m = mesh_parse_file(path);
        if (m && m->vertex_count > 0) return m;
        if (m) mesh_free(m);
    }
    return NULL;
}

/* Load level data from MESHWORLD file */
static int load_level(const char *path) {
    memset(&level, 0, sizeof(level));
    
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "Failed to open %s\n", path);
        return -1;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *data = malloc(size);
    fread(data, 1, size, f);
    fclose(f);
    
    /* Parse binary MESHWORLD level data using string scanner */
    /* The format is: [uint32 count] [type_string + data] repeated */
    level.instance_capacity = 128;
    level.instances = calloc(level.instance_capacity, sizeof(mesh_instance_t));
    level.instance_count = 0;
    
    /* Load sphere mesh for ball */
    level.ball_mesh = load_mesh("Sphere");
    if (!level.ball_mesh) level.ball_mesh = load_mesh("Sphere+Tar");
    
    /* Scan for type strings in the binary data */
    for (size_t pos = 0; pos < (size_t)size - 4; pos++) {
        uint32_t slen = *(uint32_t *)(data + pos);
        if (slen < 2 || slen > 80 || pos + 4 + slen > (size_t)size) continue;
        
        char candidate[256] = {0};
        memcpy(candidate, data + pos + 4, slen < 255 ? slen : 255);
        
        /* Clean trailing nulls and spaces */
        for (int i = slen - 1; i >= 0; i--) {
            if (candidate[i] == '\0' || candidate[i] == ' ') candidate[i] = '\0';
            else break;
        }
        
        if (strlen(candidate) < 2) continue;
        
        /* Check if this looks like a game object type */
        if (strncmp(candidate, "START", 5) == 0 ||
            strncmp(candidate, "FLAG", 4) == 0 ||
            strncmp(candidate, "SAFESPOT", 8) == 0 ||
            strncmp(candidate, "PLATFORM", 8) == 0 ||
            strncmp(candidate, "N:", 2) == 0 ||
            strncmp(candidate, "E:", 2) == 0 ||
            strncmp(candidate, "BCMESH", 6) == 0 ||
            strncmp(candidate, "BADBALL", 7) == 0 ||
            strncmp(candidate, "S:", 2) == 0 ||
            strncmp(candidate, "RND", 3) == 0) {
            
            if (level.instance_count >= level.instance_capacity) {
                level.instance_capacity *= 2;
                level.instances = realloc(level.instances, 
                                          level.instance_capacity * sizeof(mesh_instance_t));
            }
            
            mesh_instance_t *inst = &level.instances[level.instance_count];
            memset(inst, 0, sizeof(*inst));
            strncpy(inst->type_string, candidate, 255);
            
            /* Position follows type string */
            size_t data_start = pos + 4 + slen;
            if (data_start + 12 <= (size_t)size) {
                inst->position[0] = *(float *)(data + data_start);
                inst->position[1] = *(float *)(data + data_start + 4);
                inst->position[2] = *(float *)(data + data_start + 8);
            }
            
            /* Rotation (4 floats) after position */
            if (data_start + 28 <= (size_t)size) {
                inst->rotation[0] = *(float *)(data + data_start + 12);
                inst->rotation[1] = *(float *)(data + data_start + 16);
                inst->rotation[2] = *(float *)(data + data_start + 20);
                inst->rotation[3] = *(float *)(data + data_start + 24);
            }
            
            inst->scale = 1.0f;
            
            /* Highlight special objects */
            if (strncmp(candidate, "START", 5) == 0) {
                inst->highlight = 1;
                level.ball_position[0] = inst->position[0];
                level.ball_position[1] = inst->position[1];
                level.ball_position[2] = inst->position[2];
                fprintf(stderr, "[LEVEL] Start position: (%.1f, %.1f, %.1f)\n",
                       inst->position[0], inst->position[1], inst->position[2]);
            }
            
            fprintf(stderr, "[LEVEL] Found object: '%s' at (%.1f, %.1f, %.1f)\n",
                   candidate, inst->position[0], inst->position[1], inst->position[2]);
            
            level.instance_count++;
        }
    }
    
    /* Compute bounding box from instances */
    int first = 1;
    for (int i = 0; i < level.instance_count; i++) {
        float *p = level.instances[i].position;
        if (first) {
            level.min_bbox[0] = level.max_bbox[0] = p[0];
            level.min_bbox[1] = level.max_bbox[1] = p[1];
            level.min_bbox[2] = level.max_bbox[2] = p[2];
            first = 0;
        } else {
            for (int j = 0; j < 3; j++) {
                if (p[j] < level.min_bbox[j]) level.min_bbox[j] = p[j];
                if (p[j] > level.max_bbox[j]) level.max_bbox[j] = p[j];
            }
        }
    }
    
    /* Center camera on level */
    if (level.instance_count > 0) {
        camera.pos[0] = (level.min_bbox[0] + level.max_bbox[0]) / 2.0f;
        camera.pos[1] = level.max_bbox[1] + 100.0f;
        camera.pos[2] = (level.min_bbox[2] + level.max_bbox[2]) / 2.0f + 100.0f;
    }
    
    snprintf(level.name, sizeof(level.name), "%s", path);
    free(data);
    
    fprintf(stderr, "[LEVEL] Loaded %d objects from %s\n", level.instance_count, path);
    return 0;
}

/* Main render loop */
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <level.MESHWORLD>\n", argv[0]);
        return 1;
    }
    
    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    
    /* Initialize SDL_image */
    if (IMG_Init(IMG_INIT_PNG) < 0) {
        fprintf(stderr, "IMG_Init failed\n");
    }
    
    /* Create window */
    window = SDL_CreateWindow("Hamsterball Level Viewer",
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               width, height,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    
    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        fprintf(stderr, "SDL_GL_CreateContext failed\n");
        return 1;
    }
    
    /* Initialize GLEW */
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "glewInit failed: %s\n", glewGetErrorString(err));
        return 1;
    }
    
    /* Set up OpenGL */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    /* Light */
    GLfloat light_pos[] = {100.0f, 500.0f, 100.0f, 1.0f};
    GLfloat light_ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat light_diffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    
    /* Load level */
    camera_init(&camera);
    if (load_level(argv[1]) < 0) {
        fprintf(stderr, "Failed to load level\n");
        return 1;
    }
    
    /* Main loop */
    Uint32 last_time = SDL_GetTicks();
    while (running) {
        Uint32 now = SDL_GetTicks();
        float dt = (now - last_time) / 1000.0f;
        if (dt > 0.1f) dt = 0.1f;
        last_time = now;
        
        handle_input(dt);
        
        /* Handle window resize */
        SDL_GetWindowSize(window, &width, &height);
        
        /* Render */
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        setup_projection();
        draw_grid();
        render_instances();
        render_ball();
        
        /* Draw object markers */
        for (int i = 0; i < level.instance_count; i++) {
            mesh_instance_t *inst = &level.instances[i];
            if (strncmp(inst->type_string, "START", 5) == 0) {
                draw_start_marker(inst->position[0], inst->position[1], inst->position[2]);
            } else if (strncmp(inst->type_string, "FLAG", 4) == 0) {
                glColor3f(1.0f, 1.0f, 0.0f);
                glPointSize(8.0f);
                glBegin(GL_POINTS);
                glVertex3f(inst->position[0], inst->position[1], inst->position[2]);
                glEnd();
                glPointSize(1.0f);
            } else {
                glColor3f(0.5f, 0.5f, 0.8f);
                glPointSize(5.0f);
                glBegin(GL_POINTS);
                glVertex3f(inst->position[0], inst->position[1], inst->position[2]);
                glEnd();
                glPointSize(1.0f);
            }
        }
        
        SDL_GL_SwapWindow(window);
        SDL_Delay(16);
    }
    
    /* Cleanup */
    if (level.ball_mesh) mesh_free(level.ball_mesh);
    for (int i = 0; i < level.instance_count; i++) {
        if (level.instances[i].mesh) mesh_free(level.instances[i].mesh);
        if (level.instances[i].texture_id) glDeleteTextures(1, &level.instances[i].texture_id);
    }
    free(level.instances);
    
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    
    return 0;
}