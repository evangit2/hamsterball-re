/* filesys.h - Asset path resolution matching original game layout */
#ifndef FILESYS_H
#define FILESYS_H
#include <stddef.h>
#include <stdint.h>

typedef struct {
    char base_path[512];  /* Directory containing the game executable/assets */
} filesys_t;

/* Initialize filesystem with the game's root directory */
void filesys_init(filesys_t *fs, const char *exe_path);

/* Override base path explicitly */
void filesys_set_base(filesys_t *fs, const char *path);

/* Resolve an asset path: base/subdir/name.ext */
size_t filesys_resolve(const filesys_t *fs, const char *subdir,
                       const char *name, const char *ext,
                       char *out, size_t out_size);

/* Check if file exists */
int filesys_file_exists(const char *path);

/* Read entire file into memory. Caller must free *data. Returns 0 on success. */
int filesys_read_file(const char *path, uint8_t **data, size_t *size);

#endif