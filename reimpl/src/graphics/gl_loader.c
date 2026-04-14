/* gl_loader.c - Minimal OpenGL extension loader for Windows */
#ifdef _WIN32
#include "graphics/gl_loader.h"

PFNGLGENERATEMIPMAPPROC glGenerateMipmap = NULL;
PFNGLACTIVETEXTUREPROC glActiveTexture = NULL;

int gl_loader_init(void) {
    glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
    return 0; /* Non-fatal if some extensions missing */
}
#endif