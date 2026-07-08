/*
 * meshworld_merger.c — Runtime MESHWORLD file merger for Custom Entities mod v4
 *
 * At DLL load time (DllMain), scans Levels/*.MESHWORLD for S1 ref points
 * whose names contain "CE" (uppercase). For each, loads the matching
 * <name>.MESHWORLD from Levels/CustomEntities/ and merges its mesh geometry
 * (vertices + octree leaves) into the level file before the game loads it.
 *
 * v4 changes from v3:
 *   - CE: prefix system removed. Now matches any S1 name containing "CE"
 *   - CustomEntities folder moved inside Levels/ (Levels/CustomEntities/)
 *   - Entity names are plain (e.g. "CErotator"), no prefix stripping
 */

#include <windows.h>
#include <shlwapi.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

static void mw_skip_material(MWReader* r) {
    mw_skip(r, 64);
    mw_skip(r, 4);
    mw_skip(r, 4);
    DWORD has_tex = mw_u32(r);
    if (has_tex) mw_string(r, NULL, 0);
}

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

static void mw_skip_s2(MWReader* r) {
    int count = (int)mw_u32(r);
    int i;
    for (i = 0; i < count; i++) {
        mw_string(r, NULL, 0);
        mw_skip(r, mw_u32(r) * 12);
    }
}

static void mw_skip_s3(MWReader* r) {
    int count = (int)mw_u32(r);
    int i;
    for (i = 0; i < count; i++) {
        if (mw_u32(r) == 0) mw_skip(r, 36);
    }
}

static void mw_walk_octree(MWReader* r) {
    mw_skip(r, 24);
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

typedef struct {
    DWORD tri_count;
    DWORD vtx_offset;
} EntStrip;

typedef struct {
    char name[256];
    DWORD mat_offset;
    DWORD mat_size;
    EntStrip* strips;
    int strip_count;
} EntGeom;

typedef struct {
    BYTE* vertex_data;
    DWORD vertex_count;
    EntGeom* geoms;
    int geom_count;
    BYTE* material_buf;
    DWORD material_buf_size;
} EntMesh;

static void collect_geoms_rec(MWReader* r, EntMesh* mesh, DWORD vtx_off) {
    mw_skip(r, 24);
    int sub = (int)mw_u32(r);
    if (sub > 0) {
        int i;
        for (i = 0; i < sub; i++)
            collect_geoms_rec(r, mesh, vtx_off);
    } else {
        int gc = (int)mw_u32(r);
        int j;
        for (j = 0; j < gc; j++) {
            mw_string(r, mesh->geoms[mesh->geom_count].name, 256);
            DWORD mat_start = r->pos;
            mw_skip_material(r);
            DWORD mat_size = r->pos - mat_start;
            EntGeom* g = &mesh->geoms[mesh->geom_count++];
            g->mat_offset = mesh->material_buf_size;
            g->mat_size = mat_size;
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
    mw_skip(&r, 24);
    mesh->vertex_count = mw_u32(&r);
    if (mesh->vertex_count == 0 || mesh->vertex_count > 100000) {
        mesh->vertex_count = 0;
        return;
    }
    const BYTE* vtx_ptr = ent_data + r.pos;
    mw_skip(&r, mesh->vertex_count * 32);
    MWReader counter = r;
    int gc = count_geoms_rec(&counter);
    if (gc > 0) {
        mesh->geoms = (EntGeom*)calloc(gc, sizeof(EntGeom));
        collect_geoms_rec(&r, mesh, vtx_offset_base);
    }
    mesh->vertex_data = (BYTE*)malloc(mesh->vertex_count * 32);
    if (mesh->vertex_data) {
        memcpy(mesh->vertex_data, vtx_ptr, mesh->vertex_count * 32);
        for (DWORD i = 0; i < mesh->vertex_count; i++) {
            float* v = (float*)(mesh->vertex_data + i * 32);
            v[0] += pos[0];
            v[1] += pos[1];
            v[2] += pos[2];
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
    float pos[3];
} CEEntityRef;

/* Check if name contains "CE" (uppercase) */
static int is_custom_entity_name(const char* name) {
    return (strstr(name, "CE") != NULL);
}

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
        float px = mw_read_f32(&r);
        float py = mw_read_f32(&r);
        float pz = mw_read_f32(&r);
        mw_skip(&r, 12);
        if (mw_u32(&r)) mw_skip_material(&r);

        if (is_custom_entity_name(name)) {
            strncpy(refs[found].name, name, 255);
            refs[found].name[255] = '\0';
            refs[found].pos[0] = px;
            refs[found].pos[1] = py;
            refs[found].pos[2] = pz;
            found++;
        }
    }
    return found;
}

/* Write a single geom leaf into the octree buffer */
static DWORD write_geom_leaf(BYTE* buf, DWORD pos, const EntGeom* g,
                              DWORD vtx_base, const BYTE* mat_buf) {
    /* Name */
    DWORD name_len = (DWORD)strlen(g->name) + 1;
    *(DWORD*)(buf + pos) = name_len; pos += 4;
    memcpy(buf + pos, g->name, name_len); pos += name_len;
    /* Material */
    memcpy(buf + pos, mat_buf + g->mat_offset, g->mat_size);
    pos += g->mat_size;
    /* Strips */
    *(DWORD*)(buf + pos) = (DWORD)g->strip_count; pos += 4;
    int k;
    for (k = 0; k < g->strip_count; k++) {
        *(DWORD*)(buf + pos) = g->strips[k].tri_count; pos += 4;
        *(DWORD*)(buf + pos) = g->strips[k].vtx_offset; pos += 4;
    }
    return pos;
}

static int merge_level_file(const char* level_path, const char* entities_dir) {
    BYTE* level_data = NULL;
    DWORD file_size = 0;
    HANDLE hFile = CreateFileA(level_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return 0;
    file_size = GetFileSize(hFile, NULL);
    if (file_size == 0 || file_size > 50*1024*1024) { CloseHandle(hFile); return 0; }
    level_data = (BYTE*)malloc(file_size);
    if (!level_data) { CloseHandle(hFile); return 0; }
    DWORD bytes_read = 0;
    ReadFile(hFile, level_data, file_size, &bytes_read, NULL);
    CloseHandle(hFile);
    if (bytes_read != file_size) { free(level_data); return 0; }

    CEEntityRef ce_refs[32];
    int ce_count = scan_level_for_ce(level_data, file_size, ce_refs, 32);
    if (ce_count == 0) { free(level_data); return 0; }

    MWReader lr_pre;
    mw_init(&lr_pre, level_data, file_size);
    mw_skip_s1(&lr_pre);
    mw_skip_s2(&lr_pre);
    mw_skip_s3(&lr_pre);
    mw_skip(&lr_pre, 24);
    DWORD level_vtx_count = mw_u32(&lr_pre);

    EntMesh meshes[32];
    int valid = 0;
    DWORD vtx_base = level_vtx_count;
    int i;
    for (i = 0; i < ce_count; i++) {
        char mw_path[MAX_PATH];
        snprintf(mw_path, MAX_PATH, "%s\\%s.MESHWORLD", entities_dir, ce_refs[i].name);
        HANDLE hEnt = CreateFileA(mw_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                                   OPEN_EXISTING, 0, NULL);
        if (hEnt == INVALID_HANDLE_VALUE) continue;
        DWORD ent_size = GetFileSize(hEnt, NULL);
        if (ent_size == 0 || ent_size > 50*1024*1024) { CloseHandle(hEnt); continue; }
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

    DWORD total_entity_vtx = 0;
    DWORD total_entity_geoms = 0;
    for (i = 0; i < valid; i++) {
        total_entity_vtx += meshes[i].vertex_count;
        total_entity_geoms += meshes[i].geom_count;
    }

    MWReader lr;
    mw_init(&lr, level_data, file_size);
    mw_skip_s1(&lr);
    mw_skip_s2(&lr);
    mw_skip_s3(&lr);
    mw_skip(&lr, 24);
    DWORD s5_count_pos = lr.pos;
    mw_skip(&lr, 4 + level_vtx_count * 32);
    DWORD s6_start = lr.pos;

    int root_sub_count = *(int*)(level_data + s6_start + 24);

    MWReader or_;
    mw_init(&or_, level_data, file_size);
    or_.pos = s6_start;
    mw_walk_octree(&or_);
    DWORD octree_end = or_.pos;

    DWORD merged_buf_size = file_size + total_entity_vtx * 32 + 65536;
    BYTE* merged = (BYTE*)malloc(merged_buf_size);
    if (!merged) {
        for (i = 0; i < valid; i++) free_entity_mesh(&meshes[i]);
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

    /* Part 3: Original vertices */
    memcpy(merged + mpos, level_data + s5_count_pos + 4, level_vtx_count * 32);
    mpos += level_vtx_count * 32;

    /* Part 4: Entity vertices */
    for (i = 0; i < valid; i++) {
        memcpy(merged + mpos, meshes[i].vertex_data, meshes[i].vertex_count * 32);
        mpos += meshes[i].vertex_count * 32;
    }

    /* Part 5: Original octree */
    DWORD orig_octree_size = octree_end - s6_start;
    memcpy(merged + mpos, level_data + s6_start, orig_octree_size);
    mpos += orig_octree_size;

    /* Part 6: Entity octree leaves.
     * If the root has sub_count > 0, we add the entity geoms as new
     * leaf nodes at the root level (siblings of existing children).
     * If root is a leaf (sub_count == 0), we convert it to a branch
     * with the original leaf + entity leaves as children. */
    if (root_sub_count > 0) {
        /* Root is a branch — add entity leaves as new children */
        /* Update sub_count */
        int new_sub_count = root_sub_count + valid;
        *(int*)(merged + s6_start + 24) = new_sub_count;

        /* For each entity mesh, write a leaf node: cube(24) + sub_count=0(4) + geom_count(4) + geoms */
        for (i = 0; i < valid; i++) {
            /* Use the entity's S1 ref point position as the cube bounds */
            float cx = ce_refs[i].pos[0];
            float cy = ce_refs[i].pos[1];
            float cz = ce_refs[i].pos[2];
            /* Cube: 6 floats (min xyz, max xyz) */
            float cube[6] = { cx-200, cy-200, cz-200, cx+200, cy+200, cz+200 };
            memcpy(merged + mpos, cube, 24); mpos += 24;
            /* sub_count = 0 (leaf) */
            *(DWORD*)(merged + mpos) = 0; mpos += 4;
            /* geom_count */
            *(DWORD*)(merged + mpos) = (DWORD)meshes[i].geom_count; mpos += 4;
            /* Geoms */
            int j;
            for (j = 0; j < meshes[i].geom_count; j++) {
                mpos = write_geom_leaf(merged, mpos, &meshes[i].geoms[j],
                                        vtx_base, meshes[i].material_buf);
            }
        }
    } else {
        /* Root is a leaf — need to convert to branch.
         * This is more complex. For now, just append entity geoms
         * to the existing leaf's geom list. */
        /* Read original geom_count */
        DWORD orig_geom_count = *(DWORD*)(level_data + s6_start + 28);
        /* Update geom_count in merged buffer */
        *(DWORD*)(merged + s6_start + 28) = orig_geom_count + total_entity_geoms;
        /* Skip original geoms to find append position */
        MWReader gr;
        mw_init(&gr, level_data, file_size);
        gr.pos = s6_start + 32; /* past cube(24) + sub_count(4) + geom_count(4) */
        for (i = 0; i < (int)orig_geom_count; i++) {
            mw_string(&gr, NULL, 0);
            mw_skip_material(&gr);
            int sc = (int)mw_u32(&gr);
            mw_skip(&gr, sc * 8);
        }
        /* Append entity geoms at the end */
        mpos = s6_start + 32 + (gr.pos - (s6_start + 32));
        for (i = 0; i < valid; i++) {
            int j;
            for (j = 0; j < meshes[i].geom_count; j++) {
                mpos = write_geom_leaf(merged, mpos, &meshes[i].geoms[j],
                                        vtx_base, meshes[i].material_buf);
            }
        }
    }

    /* Write merged file */
    HANDLE hOut = CreateFileA(level_path, GENERIC_WRITE, 0, NULL,
                              CREATE_ALWAYS, 0, NULL);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(hOut, merged, mpos, &written, NULL);
        CloseHandle(hOut);
    }

    free(merged);
    for (i = 0; i < valid; i++) free_entity_mesh(&meshes[i]);
    free(level_data);
    return 1;
}

void merge_all_levels(const char* game_dir) {
    char levels_dir[MAX_PATH];
    char entities_dir[MAX_PATH];
    snprintf(levels_dir, MAX_PATH, "%s\\Levels", game_dir);
    snprintf(entities_dir, MAX_PATH, "%s\\Levels\\CustomEntities", game_dir);

    char search_path[MAX_PATH];
    snprintf(search_path, MAX_PATH, "%s\\*.MESHWORLD", levels_dir);
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search_path, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        char level_path[MAX_PATH];
        snprintf(level_path, MAX_PATH, "%s\\%s", levels_dir, fd.cFileName);
        merge_level_file(level_path, entities_dir);
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
}
