/* filesys.c - Asset filesystem matching original Hamsterball directory layout */
#include "core/filesys.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void filesys_init(filesys_t *fs, const char *exe_path) {
    memset(fs, 0, sizeof(*fs));
    if (!exe_path) {
        filesys_set_base(fs, ".");
        return;
    }
    /* Extract directory from exe path (strip filename, keep dir) */
    const char *last_slash = strrchr(exe_path, '/');
    if (!last_slash) last_slash = strrchr(exe_path, '\\');
    if (last_slash) {
        size_t len = last_slash - exe_path + 1;  /* Include the slash */
        if (len >= sizeof(fs->base_path)) len = sizeof(fs->base_path) - 1;
        memcpy(fs->base_path, exe_path, len);
        fs->base_path[len] = '\0';
        /* Remove trailing slash for consistency */
        size_t blen = strlen(fs->base_path);
        if (blen > 1 && (fs->base_path[blen-1] == '/' || fs->base_path[blen-1] == '\\'))
            fs->base_path[blen-1] = '\0';
    } else {
        strncpy(fs->base_path, ".", sizeof(fs->base_path) - 1);
    }
}

void filesys_set_base(filesys_t *fs, const char *path) {
    strncpy(fs->base_path, path, sizeof(fs->base_path) - 1);
    fs->base_path[sizeof(fs->base_path) - 1] = '\0';
}

size_t filesys_resolve(const filesys_t *fs, const char *subdir,
                       const char *name, const char *ext,
                       char *out, size_t out_size) {
    if (!subdir) subdir = "";
    if (!ext) ext = "";
    int n;
    if (subdir[0] && ext[0]) {
        n = snprintf(out, out_size, "%s/%s/%s.%s", fs->base_path, subdir, name, ext);
    } else if (subdir[0]) {
        n = snprintf(out, out_size, "%s/%s/%s", fs->base_path, subdir, name);
    } else if (ext[0]) {
        n = snprintf(out, out_size, "%s/%s.%s", fs->base_path, name, ext);
    } else {
        n = snprintf(out, out_size, "%s/%s", fs->base_path, name);
    }
    return (size_t)(n > 0 ? n : 0);
}

int filesys_file_exists(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

int filesys_read_file(const char *path, uint8_t **data, size_t *size) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 0) { fclose(f); return -1; }
    *data = malloc((size_t)sz + 1);
    if (!*data) { fclose(f); return -1; }
    size_t read = fread(*data, 1, (size_t)sz, f);
    fclose(f);
    *size = read;
    (*data)[read] = 0;  /* NUL terminate for text convenience */
    return 0;
}