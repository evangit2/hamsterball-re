/* gl_loader.h - Platform-specific OpenGL extension loader
 * Windows: Minimal inline loader (no GLEW dependency)
 * Linux: #include <GL/glew.h> + <GL/glu.h> as usual
 */
#ifndef GL_LOADER_H
#define GL_LOADER_H

#ifdef _WIN32
/* ===== Windows: minimal GL extension loader ===== */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

/* GL constants that may be missing from old Windows GL headers */
#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#endif
#ifndef GLGenerateMipmap
#define GL_GENERATE_MIPMAP 0x8198
#endif
#ifndef GL_SHADING_LANGUAGE_VERSION
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

/* Extension function pointer types */
typedef void (APIENTRY *PFNGLGENERATEMIPMAPPROC)(GLenum target);
typedef void (APIENTRY *PFNGLACTIVETEXTUREPROC)(GLenum texture);

extern PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
extern PFNGLACTIVETEXTUREPROC glActiveTexture;

/* Returns 0 on success */
int gl_loader_init(void);

/* GLEW compatibility shim */
#define GLEW_OK 0
#define glewInit() gl_loader_init()
#define glewGetErrorString(err) "GL extension error"
#define glewExperimental 0

#else
/* ===== Linux: GLEW handles everything ===== */
#include <GL/glew.h>
#include <GL/glu.h>

static inline int gl_loader_init(void) {
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    return (err == GLEW_OK) ? 0 : (int)err;
}
#endif

#endif