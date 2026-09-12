/* entnorm.h -- v1do: negate S5 normals in a TEMP copy (Mouse authored inverted).
 *
 * MAKYUNI verified in-editor: the Mouse mesh normals point inward; flipping
 * them there looks right. This replicates that flip at runtime: read source
 * (read-only, never written), negate nrm xyz of every S5 vert in a malloc'd
 * copy, write levels\<tmp>.MESHWORLD, caller ctors from the temp then
 * deletes it. Any failure -> 0 (caller falls back stock). v1g: generic temp
 * name via entnorm_make_inv_as for flip_normals defs (Mouse keeps its own).
 *
 * Why temp file, not in-memory verts: the MeshWorld runtime vert store
 * offset is not RE-verified; every byte walked here is file layout shared
 * with gridmesh.h (parse-verified byte-exact against real files).
 *
 * Needs from includer: log_mod, gm_read_file/gm_write_file + GmCur +
 * gm_skip_to_s6 (gridmesh.h), malloc/free/memcpy, windows.h (MAX_PATH),
 * snprintf (nocrt). Include AFTER gridmesh.h.
 */
#ifndef ENTNORM_H
#define ENTNORM_H

#define ENTNORM_TMP_NOEXT "Mouse_mknpNORM"
#define ENTNORM_FLIP_NOEXT "mknpFLIP"   /* v1g: shared temp for flip_normals
                                         * defs (sequential make->ctor->del) */

static int entnorm_make_inv_as(const char* srcAbs, const char* tmpNoExt,
                               char* outCtor, unsigned ctorCap,
                               char* outAbs, unsigned absCap,
                               const char* levelsDir, int* outN) {
    unsigned len = 0;
    unsigned char* data = 0;
    unsigned char* copy = 0;
    GmCur c;
    const unsigned char* vbuf = 0;
    int nverts = 0;
    unsigned voff = 0;
    int k = 0;
    if (outN) *outN = 0;
    if (!srcAbs || !srcAbs[0] || !tmpNoExt || !tmpNoExt[0] ||
        !outCtor || !ctorCap || !outAbs || !absCap ||
        !levelsDir || !levelsDir[0])
        return 0;
    data = gm_read_file(srcAbs, &len);
    if (!data || !len) {
        if (data) free(data);
        return 0;
    }
    copy = (unsigned char*)malloc(len);
    if (!copy) {
        free(data);
        return 0;
    }
    memcpy(copy, data, len);
    free(data);
    data = 0;
    c.p = copy;
    c.end = copy + len;
    if (!gm_skip_to_s6(&c, &vbuf, &nverts) || !vbuf || nverts <= 0) {
        free(copy);
        return 0;
    }
    voff = (unsigned)(vbuf - copy);
    if (voff + (unsigned)nverts * 32u > len) {
        free(copy);
        return 0;
    }
    for (k = 0; k < nverts; k++) {
        float* nrm = (float*)(copy + voff + (unsigned)k * 32u + 12);
        nrm[0] = -nrm[0];
        nrm[1] = -nrm[1];
        nrm[2] = -nrm[2];
    }
    snprintf(outAbs, absCap, "%s%s.MESHWORLD", levelsDir, tmpNoExt);
    if (!gm_write_file(outAbs, copy, len)) {
        free(copy);
        return 0;
    }
    free(copy);
    copy = 0;
    snprintf(outCtor, ctorCap, "levels\\%s", tmpNoExt);
    if (outN) *outN = nverts;
    return 1;
}

/* Mouse entry (unchanged behaviour, old temp name). */
static int entnorm_make_inv(const char* srcAbs, char* outCtor, unsigned ctorCap,
                            char* outAbs, unsigned absCap,
                            const char* levelsDir, int* outN) {
    return entnorm_make_inv_as(srcAbs, ENTNORM_TMP_NOEXT,
                               outCtor, ctorCap, outAbs, absCap,
                               levelsDir, outN);
}

#endif /* ENTNORM_H */
