/*
 * meshworld_merger.c — Runtime MESHWORLD file merger for Custom Entities mod
 *
 * At DLL load time (DllMain), scans Levels/*.MESHWORLD for CE: ref points
 * in Section 1, loads matching CustomEntities/<name>.MESHWORLD files, and
 * merges their mesh geometry (vertices + octree leaves) into the level file
 * before the game loads it.
 *
 * This is a file-level merge — happens once at DLL load, before the game
 * loads any levels. The game then loads the merged file normally.
 */

#include <windows.h>
#include <shlwapi.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * MESHWORLD binary format helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    const BYTE* data;
    DWORD size;
    DWORD pos;
    int error;
} MWReader;

static void mw_init(MWReader* r, const BYTE* data, DWORD size) {
    r->data = data; r->size = size; r->pos = 0; r->error = 0;
}

static DWORD mw_u32(MWReader* r) {
    if (r->error || r->pos + 4 > r->size) { r->error = 1; return 0; }
    DWORD v = *(DWORD*)(r->data + r->pos);
    r->pos += 4;
    return v;
}

static float mw_read_f32(MWReader* r) {
    if (r->error || r->pos + 4 > r->size) { r->error = 1; return 0.0f; }
    float v = *(float*)(r->data + r->pos);
    r->pos += 4;
    return v;
}

static void mw_skip(MWReader* r, DWORD len) {
    if (r->error || r->pos + len > r->size) { r->error = 1; return; }
    r->pos += len;
}

/* Read a length-prefixed string. Optionally copies to out_buf. */
static void mw_string(MWReader* r, char* out_buf, int out_size) {
    DWORD len = mw_u32(r);
    if (r->error) return;
    if (out_buf && out_size > 0) {
        int cl = (int)len;
        if (cl >= out_size) cl = out_size - 1;
        if (cl > 0 && r->pos + cl <= r->size)
            memcpy(out_buf, r->data + r->pos, cl);
        out_buf[cl] = '\0';
    }
    mw_skip(r, len);
}

/* Skip material block */
static void mw_skip_material(MWReader* r) {
    mw_skip(r, 64);  /* 4x4f */
    mw_skip(r, 4);   /* power */
    mw_skip(r, 4);   /* has_refl */
    DWORD has_tex = mw_u32(r);
    if (has_tex) mw_string(r, NULL, 0);
}

/* Skip S1 (ref points). Returns count. */
static int mw_skip_s1(MWReader* r) {
    int count = (int)mw_u32(r);
    int i;
    for (i = 0; i < count; i++) {
        mw_string(r, NULL, 0);
        mw_skip(r, 24);
        if (mw_u32(r)) mw_skip_material(r);
    }
    return count;
}

/* Skip S2 (splines) */
static void mw_skip_s2(MWReader* r) {
    int count = (int)mw_u32(r);
    int i;
    for (i = 0; i < count; i++) {
        mw_string(r, NULL, 0);
        mw_skip(r, mw_u32(r) * 12);
    }
}

/* Skip S3 (lights) */
static void mw_skip_s3(MWReader* r) {
    int count = (int)mw_u32(r);
    int i;
    for (i = 0; i < count; i++) {
        if (mw_u32(r) == 0) mw_skip(r, 36);
    }
}

/* Walk octree to find end offset */
static void mw_walk_octree(MWReader* r) {
    mw_skip(r, 24);  /* cube */
    int sub = (int)mw_u32(r);
    if (sub > 0) {
        int i;
        for (i = 0; i < sub; i++) mw_walk_octree(r);
    } else {
        int gc = (int)mw_u32(r);
        int j;
        for (j = 0; j < gc; j++) {
            mw_string(r, NULL, 0);
            mw_skip_material(r);
            int sc = (int)mw_u32(r);
            mw_skip(r, sc * 8);
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Entity mesh collection (iterative, no nested functions)
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    DWORD tri_count;
    DWORD vtx_offset;
} EntStrip;

typedef struct {
    char name[256];      /* Original geom name (e.g. "CE:Rotator") */
    DWORD mat_offset;   /* Offset into material buffer */
    DWORD mat_size;
    EntStrip* strips;
    int strip_count;
} EntGeom;

typedef struct {
    BYTE* vertex_data;
    DWORD vertex_count;
    EntGeom* geoms;
    int geom_count;
    BYTE* material_buf;   /* All materials stored contiguously */
    DWORD material_buf_size;
} EntMesh;

/* Recursively collect geoms from entity octree (non-nested, passed context) */
static void collect_geoms_rec(MWReader* r, EntMesh* mesh, DWORD vtx_off) {
    mw_skip(r, 24);  /* cube */
    int sub = (int)mw_u32(r);
    if (sub > 0) {
        int i;
        for (i = 0; i < sub; i++)
            collect_geoms_rec(r, mesh, vtx_off);
    } else {
        int gc = (int)mw_u32(r);
        int j;
        for (j = 0; j < gc; j++) {
            /* Capture the original geom name (e.g. "CE:Rotator") */
            mw_string(r, mesh->geoms[mesh->geom_count].name, 256);

            /* Read material */
            DWORD mat_start = r->pos;
            mw_skip_material(r);
            DWORD mat_size = r->pos - mat_start;

            EntGeom* g = &mesh->geoms[mesh->geom_count++];
            g->mat_offset = mesh->material_buf_size;
            g->mat_size = mat_size;

            /* Grow material buffer */
            BYTE* new_buf = (BYTE*)realloc(mesh->material_buf,
                                            mesh->material_buf_size + mat_size);
            if (new_buf) {
                memcpy(new_buf + mesh->material_buf_size,
                       r->data + mat_start, mat_size);
                mesh->material_buf = new_buf;
                mesh->material_buf_size += mat_size;
            }

            int sc = (int)mw_u32(r);
            g->strip_count = sc;
            g->strips = (EntStrip*)malloc(sc * sizeof(EntStrip));
            int k;
            for (k = 0; k < sc; k++) {
                g->strips[k].tri_count = mw_u32(r);
                g->strips[k].vtx_offset = mw_u32(r) + vtx_off;
            }
        }
    }
}

/* Count geoms in entity octree (recursive) */
static int count_geoms_rec(MWReader* r) {
    mw_skip(r, 24);
    int sub = (int)mw_u32(r);
    int total = 0;
    if (sub > 0) {
        int i;
        for (i = 0; i < sub; i++)
            total += count_geoms_rec(r);
    } else {
        int gc = (int)mw_u32(r);
        total = gc;
        int j;
        for (j = 0; j < gc; j++) {
            mw_string(r, NULL, 0);
            mw_skip_material(r);
            int sc = (int)mw_u32(r);
            mw_skip(r, sc * 8);
        }
    }
    return total;
}

static void collect_entity_mesh(const BYTE* ent_data, DWORD ent_size,
                                  DWORD vtx_offset_base, const float* pos,
                                  EntMesh* mesh) {
    memset(mesh, 0, sizeof(EntMesh));

    MWReader r;
    mw_init(&r, ent_data, ent_size);
    mw_skip_s1(&r);
    mw_skip_s2(&r);
    mw_skip_s3(&r);
    mw_skip(&r, 24);  /* S4 */

    mesh->vertex_count = mw_u32(&r);
    if (mesh->vertex_count == 0 || mesh->vertex_count > 100000) {
        mesh->vertex_count = 0;
        return;
    }

    const BYTE* vtx_ptr = ent_data + r.pos;
    mw_skip(&r, mesh->vertex_count * 32);

    /* Count geoms */
    MWReader counter = r;
    int gc = count_geoms_rec(&counter);

    if (gc > 0) {
        mesh->geoms = (EntGeom*)calloc(gc, sizeof(EntGeom));
        collect_geoms_rec(&r, mesh, vtx_offset_base);
    }

    /* Copy vertex data and translate each vertex by the S1 ref point position.
     * The entity MESHWORLD has vertices at local coordinates (centered around
     * origin). The game's level exporter puts ALL vertices in world space.
     * We must do the same: translate each vertex by (pos_x, pos_y, pos_z). */
    mesh->vertex_data = (BYTE*)malloc(mesh->vertex_count * 32);
    if (mesh->vertex_data) {
        memcpy(mesh->vertex_data, vtx_ptr, mesh->vertex_count * 32);
        /* Translate each vertex's position (first 3 floats = x, y, z) */
        for (DWORD i = 0; i < mesh->vertex_count; i++) {
            float* v = (float*)(mesh->vertex_data + i * 32);
            v[0] += pos[0];  /* x */
            v[1] += pos[1];  /* y */
            v[2] += pos[2];  /* z */
        }
    }
}

static void free_entity_mesh(EntMesh* mesh) {
    int i;
    for (i = 0; i < mesh->geom_count; i++)
        free(mesh->geoms[i].strips);
    free(mesh->geoms);
    free(mesh->vertex_data);
    free(mesh->material_buf);
}

typedef struct {
    char name[256];
    float pos[3];  /* World-space position from S1 ref point (x, y, z) */
} CEEntityRef;

static int scan_level_for_ce(const BYTE* data, DWORD size,
                              CEEntityRef* refs, int max_refs) {
    MWReader r;
    mw_init(&r, data, size);

    int s1_count = (int)mw_u32(&r);
    int found = 0;
    int i;
    for (i = 0; i < s1_count && found < max_refs; i++) {
        char name[256] = {0};
        mw_string(&r, name, sizeof(name));
        /* S1 stores position as 3 floats: x, y, z (despite spec claiming x,z,y) */
        float px = mw_read_f32(&r);
        float py = mw_read_f32(&r);
        float pz = mw_read_f32(&r);
        mw_skip(&r, 12);  /* rotation (3 floats) */
        if (mw_u32(&r)) mw_skip_material(&r);

        /* Strip "REF:" prefix if present (some MESHWORLD files use REF:CE:Name) */
        const char* effective_name = name;
        if (_strnicmp(name, "REF:", 4) == 0) {
            effective_name = name + 4;
        }

        if (_strnicmp(effective_name, "CE:", 3) == 0) {
            const char* suffix = effective_name + 3;
            int j;
            for (j = 0; suffix[j] && suffix[j] != '(' && j < 255; j++)
                refs[found].name[j] = suffix[j];
            refs[found].name[j] = '\0';
            refs[found].pos[0] = px;
            refs[found].pos[1] = py;
            refs[found].pos[2] = pz;
            found++;
        }
    }
    return found;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Merge one level file
 * ═══════════════════════════════════════════════════════════════════════════ */

static int merge_level_file(const char* level_path, const char* entities_dir) {
    /* Check if a backup of the original (un-merged) file exists.
     * If so, always merge from the backup to prevent double-merging.
     * If not, create the backup from the current file (first run). */
    char backup_path[MAX_PATH];
    snprintf(backup_path, MAX_PATH, "%s.orig", level_path);

    const char* source_path = level_path;
    BYTE* level_data = NULL;
    DWORD file_size = 0;

    if (GetFileAttributesA(backup_path) != INVALID_FILE_ATTRIBUTES) {
        /* Backup exists — use it as the source (prevents re-merge) */
        source_path = backup_path;
    } else {
        /* First run: create backup of the original file */
        if (!CopyFileA(level_path, backup_path, FALSE)) {
            /* Copy failed — proceed with the level file itself */
        }
    }

    /* Read source file */
    {
        HANDLE hFile = CreateFileA(source_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                                   OPEN_EXISTING, 0, NULL);
        if (hFile == INVALID_HANDLE_VALUE) return 0;

        file_size = GetFileSize(hFile, NULL);
        if (file_size == 0 || file_size > 50*1024*1024) {
            CloseHandle(hFile);
            return 0;
        }

        level_data = (BYTE*)malloc(file_size);
        if (!level_data) { CloseHandle(hFile); return 0; }

        DWORD bytes_read = 0;
        ReadFile(hFile, level_data, file_size, &bytes_read, NULL);
        CloseHandle(hFile);
        if (bytes_read != file_size) { free(level_data); return 0; }
    }

    /* Scan for CE: entities (now captures position too) */
    CEEntityRef ce_refs[32];
    int ce_count = scan_level_for_ce(level_data, file_size, ce_refs, 32);
    if (ce_count == 0) { free(level_data); return 0; }

    /* Find S5 vertex count (needed to offset entity vertex references) */
    MWReader lr_pre;
    mw_init(&lr_pre, level_data, file_size);
    mw_skip_s1(&lr_pre);
    mw_skip_s2(&lr_pre);
    mw_skip_s3(&lr_pre);
    mw_skip(&lr_pre, 24);  /* S4 */
    DWORD level_vtx_count = mw_u32(&lr_pre);

    /* Load entity meshes.
     * vtx_base starts at level_vtx_count so that entity vertex references
     * (which are offsets into the global vertex buffer) point past the
     * level's vertices into the entity's appended vertices. */
    EntMesh meshes[32];
    int valid = 0;
    DWORD vtx_base = level_vtx_count;

    for (int i = 0; i < ce_count; i++) {
        char mw_path[MAX_PATH];
        snprintf(mw_path, MAX_PATH, "%s\\%s.MESHWORLD", entities_dir, ce_refs[i].name);

        HANDLE hEnt = CreateFileA(mw_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                                   OPEN_EXISTING, 0, NULL);
        if (hEnt == INVALID_HANDLE_VALUE) continue;

        DWORD ent_size = GetFileSize(hEnt, NULL);
        if (ent_size == 0 || ent_size > 50*1024*1024) {
            CloseHandle(hEnt);
            continue;
        }

        BYTE* ent_data = (BYTE*)malloc(ent_size);
        if (!ent_data) { CloseHandle(hEnt); continue; }

        DWORD ent_read = 0;
        ReadFile(hEnt, ent_data, ent_size, &ent_read, NULL);
        CloseHandle(hEnt);
        if (ent_read != ent_size) { free(ent_data); continue; }

        collect_entity_mesh(ent_data, ent_size, vtx_base, ce_refs[i].pos, &meshes[valid]);
        free(ent_data);

        if (meshes[valid].vertex_count > 0) {
            vtx_base += meshes[valid].vertex_count;
            valid++;
        }
    }

    if (valid == 0) { free(level_data); return 0; }

    /* Calculate totals */
    DWORD total_entity_vtx = 0;
    DWORD total_entity_geoms = 0;
    for (int i = 0; i < valid; i++) {
        total_entity_vtx += meshes[i].vertex_count;
        total_entity_geoms += meshes[i].geom_count;
    }

    /* Find S5 count position and S6 start.
     * level_vtx_count was already read above (before entity loading). */
    MWReader lr;
    mw_init(&lr, level_data, file_size);
    mw_skip_s1(&lr);
    mw_skip_s2(&lr);
    mw_skip_s3(&lr);
    mw_skip(&lr, 24);  /* S4 */

    DWORD s5_count_pos = lr.pos;
    /* level_vtx_count already computed above */
    mw_skip(&lr, 4 + level_vtx_count * 32);
    DWORD s6_start = lr.pos;

    int root_sub_count = *(int*)(level_data + s6_start + 24);

    /* Find octree end */
    MWReader or_;
    mw_init(&or_, level_data, file_size);
    or_.pos = s6_start;
    mw_walk_octree(&or_);
    DWORD octree_end = or_.pos;

    /* Build merged file. Extra 65536 bytes for octree overhead (LEAF wrapping,
     * bounding cube, sub_count, geom_count, etc.) */
    DWORD merged_buf_size = file_size + total_entity_vtx * 32 + 65536;
    BYTE* merged = (BYTE*)malloc(merged_buf_size);
    if (!merged) {
        for (int i = 0; i < valid; i++) free_entity_mesh(&meshes[i]);
        free(level_data);
        return 0;
    }

    DWORD mpos = 0;

    /* Part 1: S1-S4 (before S5 count) */
    memcpy(merged + mpos, level_data, s5_count_pos);
    mpos += s5_count_pos;

    /* Part 2: New vertex count */
    *(DWORD*)(merged + mpos) = level_vtx_count + total_entity_vtx;
    mpos += 4;

    /* Part 3: Level vertices + entity vertices */
    memcpy(merged + mpos, level_data + s5_count_pos + 4, level_vtx_count * 32);
    mpos += level_vtx_count * 32;
    for (int i = 0; i < valid; i++) {
        memcpy(merged + mpos, meshes[i].vertex_data, meshes[i].vertex_count * 32);
        mpos += meshes[i].vertex_count * 32;
    }

    /* Part 4: Octree with +1 root submesh
     *
     * There are two cases:
     * A) Root is a BRANCH (sub_count > 0): simply add +1 to sub_count and
     *    append a new child leaf after the existing children.
     * B) Root is a LEAF (sub_count == 0): we cannot just change sub_count to 1
     *    because the data after sub_count is geom_count + geoms, not a child node.
     *    Instead, we must wrap the original LEAF as a child: write the original
     *    cube + sub_count=0 + geom_count + original geoms as the first child,
     *    then append our entity leaf as the second child.
     */
    /* (root cube is at level_data + s6_start, sub_count at +24) */

    /* Compute the bounding cube for entity geoms from their translated vertices.
     * The entity file's default octree has an infinitely large bounding cube
     * (-1000000 to 1000000). If we copy that, the game's collision system will
     * find entity geoms in EVERY collision query, causing broken collisions,
     * spikes, and player ball burial.
     * Instead, compute the tight AABB from the translated vertex data. */
    float ent_min[3] = { 1e30f, 1e30f, 1e30f };
    float ent_max[3] = { -1e30f, -1e30f, -1e30f };
    for (int i = 0; i < valid; i++) {
        for (DWORD v = 0; v < meshes[i].vertex_count; v++) {
            float* vp = (float*)(meshes[i].vertex_data + v * 32);
            for (int ax = 0; ax < 3; ax++) {
                if (vp[ax] < ent_min[ax]) ent_min[ax] = vp[ax];
                if (vp[ax] > ent_max[ax]) ent_max[ax] = vp[ax];
            }
        }
    }
    /* Add small margin to avoid floating-point edge cases */
    for (int ax = 0; ax < 3; ax++) {
        ent_min[ax] -= 1.0f;
        ent_max[ax] += 1.0f;
    }

    if (root_sub_count > 0) {
        /* Case A: Root is BRANCH — add +1 to sub_count, copy children, append leaf */
        memcpy(merged + mpos, level_data + s6_start, 24);  /* Root cube */
        mpos += 24;
        *(int*)(merged + mpos) = root_sub_count + 1;
        mpos += 4;
        DWORD children_size = octree_end - (s6_start + 28);
        memcpy(merged + mpos, level_data + s6_start + 28, children_size);
        mpos += children_size;
    } else {
        /* Case B: Root is LEAF — wrap original leaf as child[0], entity leaf as child[1]
         * Write root cube (copied from original, but we could also use the merged AABB).
         * Use the original root cube since it already bounds the level geometry. */
        memcpy(merged + mpos, level_data + s6_start, 24);  /* Root cube */
        mpos += 24;
        *(int*)(merged + mpos) = 2;  /* 2 children: original leaf + entity leaf */
        mpos += 4;

        /* Child 0: original leaf (cube + sub_count=0 + geom_count + geoms) */
        DWORD orig_leaf_start = s6_start;  /* original root IS the leaf */
        DWORD orig_leaf_size = octree_end - s6_start;
        memcpy(merged + mpos, level_data + orig_leaf_start, orig_leaf_size);
        mpos += orig_leaf_size;
    }

    /* Part 5: New entity leaf with correct bounding cube */
    float ent_cube[6] = { ent_min[0], ent_min[1], ent_min[2],
                          ent_max[0], ent_max[1], ent_max[2] };
    memcpy(merged + mpos, ent_cube, 24);
    mpos += 24;
    *(int*)(merged + mpos) = 0;  /* Leaf */
    mpos += 4;
    *(int*)(merged + mpos) = (int)total_entity_geoms;
    mpos += 4;

    for (int i = 0; i < valid; i++) {
        for (int j = 0; j < meshes[i].geom_count; j++) {
            EntGeom* g = &meshes[i].geoms[j];
            /* Write the original geom name (e.g. "CE:Rotator") so the game
             * links this mesh to the S1 EntityTransform for positioning */
            DWORD name_len = (DWORD)strlen(g->name) + 1;  /* include NUL */
            *(DWORD*)(merged + mpos) = name_len;
            mpos += 4;
            memcpy(merged + mpos, g->name, name_len);
            mpos += name_len;

            /* Write material block VERBATIM from the entity file.
             *
             * The material block stores RGBA colors (ambient, diffuse, specular,
             * emissive) + power + has_refl + has_tex + texture name. These are
             * standard D3D material properties — the game reads them as colors,
             * NOT as EntityTransform data.
             *
             * The EntityTransform is a RUNTIME struct at MeshWorld+0x28+idx*0x50,
             * populated by the game's binary loader AFTER parsing the material
             * block. It is NOT stored in the MESHWORLD file's material data.
             *
             * Previous versions of this code overwrote ambient/diffuse with zeros
             * ("identity EntityTransform"), which set diffuse alpha to 0.0 —
             * making the mesh fully transparent and invisible. */
            BYTE* mat_src = meshes[i].material_buf + g->mat_offset;
            memcpy(merged + mpos, mat_src, g->mat_size);
            mpos += g->mat_size;

            *(DWORD*)(merged + mpos) = g->strip_count;
            mpos += 4;
            for (int k = 0; k < g->strip_count; k++) {
                *(DWORD*)(merged + mpos) = g->strips[k].tri_count;
                mpos += 4;
                *(DWORD*)(merged + mpos) = g->strips[k].vtx_offset;
                mpos += 4;
            }
        }
    }

    /* Part 6: Trailing data after octree */
    DWORD trailing = file_size - octree_end;
    if (trailing > 0) {
        memcpy(merged + mpos, level_data + octree_end, trailing);
        mpos += trailing;
    }

    /* Write merged file */
    char tmp_path[MAX_PATH];
    snprintf(tmp_path, MAX_PATH, "%s.tmp", level_path);

    HANDLE hOut = CreateFileA(tmp_path, GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, 0, NULL);
    if (hOut == INVALID_HANDLE_VALUE) {
        free(merged);
        for (int i = 0; i < valid; i++) free_entity_mesh(&meshes[i]);
        free(level_data);
        return 0;
    }

    DWORD written = 0;
    WriteFile(hOut, merged, mpos, &written, NULL);
    CloseHandle(hOut);

    /* Replace original */
    DeleteFileA(level_path);
    MoveFileA(tmp_path, level_path);

    /* Delete .cached files */
    {
        char cached_path[MAX_PATH];
        snprintf(cached_path, MAX_PATH, "%s.cached", level_path);
        DeleteFileA(cached_path);
    }

    /* Cleanup */
    free(merged);
    for (int i = 0; i < valid; i++) free_entity_mesh(&meshes[i]);
    free(level_data);

    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Merge all level files in Levels/ directory
 * ═══════════════════════════════════════════════════════════════════════════ */

void merge_all_levels(const char* game_dir) {
    char levels_dir[MAX_PATH];
    char search_path[MAX_PATH];
    char entities_dir[MAX_PATH];

    snprintf(levels_dir, MAX_PATH, "%s\\Levels", game_dir);
    snprintf(entities_dir, MAX_PATH, "%s\\CustomEntities", game_dir);
    snprintf(search_path, MAX_PATH, "%s\\*.MESHWORLD", levels_dir);

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search_path, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        char level_path[MAX_PATH];
        snprintf(level_path, MAX_PATH, "%s\\%s", levels_dir, fd.cFileName);
        merge_level_file(level_path, entities_dir);
    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);
}
