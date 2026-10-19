/* nostdio_test.h — TEST-ONLY: neutralize stdio FILE streams to restore Wine
 * input for local reproduction. NOT used in the shipped build.
 *
 * The mod's 246 fopen/fprintf call sites still compile (stdio.h stays
 * included), but at runtime every FILE* stream call becomes a no-op:
 *   fopen() → NULL, fprintf() → no-op, fclose() → no-op, fflush() → no-op.
 * snprintf/sscanf are KEPT (path building + config parsing need them).
 */
#ifndef NOSTDIO_TEST_H
#define NOSTDIO_TEST_H

#include <stdio.h>

#define fopen(path, mode)      ((FILE*)0)
#define fopen_s(ppf, p, m)     (*(ppf) = NULL, 0)
#define fprintf(f, ...)        (0)
#define fclose(f)              (0)
#define fflush(f)              (0)
#define fread(buf, sz, n, f)   (0)
#define fwrite(buf, sz, n, f)  (0)
#define fseek(f, o, w)         (0)

#endif
