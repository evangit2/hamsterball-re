/*
 * custom_entities.c — Hamsterball Custom Entities Mod v11
 *
 * bass.dll proxy mod. Runtime mesh loading:
 *   Reads testcube.MESHWORLD from mod directory.
 *   Parses vertices (24×32 bytes) and strips (6 strips).
 *   Creates MeshBuffer objects via game's CreateMeshBuffer.
 *   Adds them to the level's MeshWorld at S1 GRID positions.
 *   Colors each cube differently (Red, Orange, Yellow, Green, Blue).
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll custom_entities.c \
 *     -I../shared -lwinmm -Wl,--enable-stdcall-fixup -O2 -static \
 *     -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse -lshlwapi
 */

#include "bass_proxy.h"
#include <shlwapi.h>
#include <ctype.h>
#include <stdio.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * Game function pointers
 * ═══════════════════════════════════════════════════════════════════════════ */

/* CreateMeshBuffer function pointer */
typedef void* (__fastcall *CreateMeshBuffer_t)(void* alloc);
static CreateMeshBuffer_t pfn_CreateMeshBuffer = (CreateMeshBuffer_t)0x00458600;

/* AthenaList_Append is declared in bass_proxy.h */
static AthenaList_Append_t  pfn_AthenaList_Append = (AthenaList_Append_t)0x00453810;

/* ═══════════════════════════════════════════════════════════════════════════
 * Structure offsets
 * ═══════════════════════════════════════════════════════════════════════════ */

#define BOARD_LEVEL             0x8AC
#define SCENEOBJ_S1_LIST        0x894
#define LEVEL_MESHWORLD_PTR     0x08   /* level+0x08 = MeshWorld pointer (NOT sceneobj+0x08!) */

#define S1ENTRY_NAME             0x00
#define S1ENTRY_POS_X            0x04
#define S1ENTRY_POS_Y            0x08
#define S1ENTRY_POS_Z            0x0C
#define MW_MB_COUNT             0x24   /* MeshBuffer file count (RC array size) */
#define MW_RENDERCTX_ARRAY      0x28   /* RenderContext array (allocated) */
#define MW_MBLIST               0x2C   /* AthenaList of MeshBuffer objects */
#define MW_MBLIST_COUNT         0x30   /* AthenaList count (= MW_MBLIST + 0x04) */
#define MW_MBLIST_DATA          0x438  /* AthenaList data ptr (= MW_MBLIST + 0x40C) */
#define MW_VERTEX_DATA          0x45C  /* Vertex data start (24 byte header + bbox) */

/* Level layout */
#define LEVEL_MESHWORLD_PTR     0x08   /* level+0x08 = MeshWorld pointer */
#define LEVEL_SUBLIST           0x18   /* AthenaList of sub-levels (when S1 count >= 1) */
#define LEVEL_SCENEOBJECT       0x480
#define LEVEL_HAS_SUBLIST       0x430  /* byte: 1 = has sub-levels, 0 = direct mesh */

/* MeshBuffer layout (0x874 bytes) */
#define MB_NAME_PTR             0x864  /* char* name */
#define MB_FLAG_NOSHADOW        0x85E  /* byte */
#define MB_STRIP_LIST           0x424  /* AthenaList of strip entries (0x109*4) */
#define MB_RENDERCTX_INDEX      0x04  /* DWORD index into MW_RENDERCTX_ARRAY */

/* RenderContext layout (0x50 bytes) */
#define RC_MATRIX_START         0x04   /* 4 matrices × 4 floats = 64 bytes */
#define RC_SCALE_FLAGS          0x4C   /* bool: scale != 1.0 */
#define RC_SHINE                0x44   /* float */
#define RC_HAS_TEX              0x48   /* DWORD: texture pointer or 0 */
#define RC_BOOL_FLAG            0x4D   /* byte */

/* ═══════════════════════════════════════════════════════════════════════════
 * State
 * ═══════════════════════════════════════════════════════════════════════════ */

static HANDLE g_thread = NULL;
static volatile int g_running = 1;
static char g_game_dir[MAX_PATH] = {0};
static DWORD g_last_level = 0;

/* Parsed testcube mesh data */
typedef struct {
    float vertices[24 * 8];  /* 24 verts × 8 floats (pos3 + norm3 + uv2) */
    int vertex_count;
    float material[16];      /* 4 groups × 4 floats */
    float shine;
    int has_texture;
    int bool_flag;
    int strip_count;
    int strips[12][2];       /* up to 12 strips, each (vref, count) */
} MeshData;

static MeshData g_mesh_data;
static int g_mesh_loaded = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * Helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD get_level(DWORD board) {
    if (!board) return 0;
    if (IsBadReadPtr((void*)(board + BOARD_LEVEL), 4)) return 0;
    DWORD level = *(DWORD*)(board + BOARD_LEVEL);
    if (!level || level < 0x10000) return 0;
    return level;
}

static DWORD get_sceneobj(DWORD board) {
    DWORD level = get_level(board);
    if (!level) return 0;
    if (IsBadReadPtr((void*)(level + LEVEL_SCENEOBJECT), 4)) return 0;
    DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
    if (!sceneobj || sceneobj < 0x10000) return 0;
    return sceneobj;
}

static DWORD get_meshworld(DWORD board) {
    DWORD level = get_level(board);
    if (!level) return 0;
    /* MeshWorld pointer is at LEVEL+0x08.
     * Verified via Ghidra decompilation of Scene_LoadMeshWorld (0x461890):
     *   *(undefined4 **)((int)this + 8) = puVar2;  // this = Level object
     * Note: For levels with sub-levels (S1 count >= 1), this MeshWorld has
     * MW+0x24 uninitialized. The actual mesh data is in sub-level MeshWorlds. */
    if (IsBadReadPtr((void*)(level + LEVEL_MESHWORLD_PTR), 4)) return 0;
    DWORD mw = *(DWORD*)(level + LEVEL_MESHWORLD_PTR);
    if (!mw || mw < 0x10000) return 0;
    return mw;
}

/* Find a usable MeshWorld — one with valid mb_count (MW+0x24).
 * For levels with sub-levels (S1 count >= 1 in the MESHWORLD file),
 * the parent level's MeshWorld has MW+0x24 uninitialized.
 * We scan the sub-levels at level+0x18 for one with valid mesh data. */
static DWORD find_meshworld(DWORD board, FILE* logf) {
    DWORD level = get_level(board);
    if (!level) return 0;

    DWORD mw = get_meshworld(board);
    if (mw) {
        /* Check if MW+0x24 is valid */
        int mb_count = 0;
        if (!IsBadReadPtr((void*)(mw + MW_MB_COUNT), 4))
            mb_count = *(int*)(mw + MW_MB_COUNT);

        if (mb_count >= 0 && mb_count < 10000) {
            /* Direct mesh level — MW+0x24 is valid */
            if (logf) fprintf(logf, "  GRID: Found MeshWorld at level+0x08 = 0x%08X (mb_count=%d)\n", mw, mb_count);
            return mw;
        }

        /* MW+0x24 is garbage — this level has sub-levels.
         * Scan the sub-level AthenaList at level+0x18 for a MeshWorld with valid mb_count. */
        if (logf) fprintf(logf, "  GRID: level+0x08 MW=0x%08X has bad mb_count=%d (0x%08X), scanning sub-levels...\n",
                mw, mb_count, mb_count);

        /* AthenaList at level+0x18: count at +0x04, data at +0x40C */
        if (!IsBadReadPtr((void*)(level + LEVEL_SUBLIST + 0x04), 4)) {
            int sub_count = *(int*)(level + LEVEL_SUBLIST + 0x04);
            if (sub_count > 0 && sub_count < 1000) {
                if (!IsBadReadPtr((void*)(level + LEVEL_SUBLIST + 0x40C), 4)) {
                    DWORD* sub_data = *(DWORD**)(level + LEVEL_SUBLIST + 0x40C);
                    if (sub_data && !IsBadReadPtr(sub_data, sub_count * 4)) {
                        int i;
                        for (i = 0; i < sub_count; i++) {
                            DWORD sub_level = sub_data[i];
                            if (!sub_level || sub_level < 0x10000) continue;
                            if (IsBadReadPtr((void*)(sub_level + LEVEL_MESHWORLD_PTR), 4)) continue;
                            DWORD sub_mw = *(DWORD*)(sub_level + LEVEL_MESHWORLD_PTR);
                            if (!sub_mw || sub_mw < 0x10000) continue;
                            if (IsBadReadPtr((void*)(sub_mw + MW_MB_COUNT), 4)) continue;
                            int sub_mb_count = *(int*)(sub_mw + MW_MB_COUNT);
                            if (sub_mb_count >= 0 && sub_mb_count < 10000) {
                                if (logf) fprintf(logf, "  GRID: Found valid MeshWorld in sub-level[%d] = 0x%08X (mb_count=%d)\n",
                                        i, sub_mw, sub_mb_count);
                                return sub_mw;
                            }
                        }
                    }
                }
            }
        }
    }

    if (logf) fprintf(logf, "  GRID: no usable MeshWorld found\n");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * GRID: Parse (GRIDxx) from name
 * ═══════════════════════════════════════════════════════════════════════════ */

static int parse_grid_flag(const char* name) {
    const char* p = name;
    while ((p = strstr(p, "(GRID")) != NULL) {
        const char* digits = p + 5;
        if (isdigit((unsigned char)digits[0]) && isdigit((unsigned char)digits[1]) &&
            digits[2] == ')') {
            int num = (digits[0] - '0') * 10 + (digits[1] - '0');
            if (num >= 1 && num <= 99) return num;
        }
        if (isdigit((unsigned char)digits[0]) && digits[1] == ')') {
            int num = digits[0] - '0';
            if (num >= 1 && num <= 9) return num;
        }
        p++;
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Load testcube.MESHWORLD from mod directory
 * ═══════════════════════════════════════════════════════════════════════════ */

static int load_testcube_mesh(void) {
    char path[MAX_PATH];
    /* Try mod dir first, then Levels subdirectory */
    snprintf(path, MAX_PATH, "%s\\testcube.MESHWORLD", g_game_dir);
    FILE* f = NULL;
    fopen_s(&f, path, "rb");
    if (!f) {
        snprintf(path, MAX_PATH, "%s\\Levels\\testcube.MESHWORLD", g_game_dir);
        fopen_s(&f, path, "rb");
    }
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize < 100 || fsize > 100000) { fclose(f); return 0; }

    unsigned char* data = (unsigned char*)malloc(fsize);
    if (!data) { fclose(f); return 0; }
    fread(data, 1, fsize, f);
    fclose(f);

    /* Parse: S1(4) + S2(4) + S3(4) + bbox(24) + vc(4) + vertices(vc*32) + octree */
    int pos = 0;
    /* S1/S2/S3 counts (all 0 for testcube) */
    pos += 12;
    /* bbox */
    pos += 24;
    /* vertex count */
    int vc = *(int*)(data + pos);
    pos += 4;
    if (vc < 1 || vc > 1000) { free(data); return 0; }

    g_mesh_data.vertex_count = vc;

    /* Read vertices: each is 32 bytes (pos3 + norm3 + pad2 + uv2 = 8 floats) */
    int v;
    for (v = 0; v < vc && v < 24; v++) {
        float* vdata = (float*)(data + pos + v * 32);
        g_mesh_data.vertices[v * 8 + 0] = vdata[0];  /* posX */
        g_mesh_data.vertices[v * 8 + 1] = vdata[1];  /* posY */
        g_mesh_data.vertices[v * 8 + 2] = vdata[2];  /* posZ */
        g_mesh_data.vertices[v * 8 + 3] = vdata[3];  /* normX */
        g_mesh_data.vertices[v * 8 + 4] = vdata[4];  /* normY */
        g_mesh_data.vertices[v * 8 + 5] = vdata[5];  /* normZ */
        g_mesh_data.vertices[v * 8 + 6] = vdata[6];  /* uvU */
        g_mesh_data.vertices[v * 8 + 7] = vdata[7];  /* uvV */
    }
    pos += vc * 32;

    /* Parse octree: root(bbox24 + sc4) → if sc=1, child(bbox24 + sc4=0 + gc4=1) → mesh */
    /* Root */
    pos += 24;  /* bbox */
    int root_sc = *(int*)(data + pos);
    pos += 4;

    if (root_sc > 0) {
        /* Child */
        pos += 24;  /* bbox */
        int child_sc = *(int*)(data + pos);
        pos += 4;

        if (child_sc == 0) {
            int gc = *(int*)(data + pos);
            pos += 4;

            if (gc >= 1) {
                /* Parse mesh entry */
                int nl = *(int*)(data + pos);
                pos += 4;
                /* Skip name */
                pos += nl;

                /* Material: 4 groups × 4 floats = 64 bytes */
                memcpy(g_mesh_data.material, data + pos, 64);
                pos += 64;

                /* Shine */
                g_mesh_data.shine = *(float*)(data + pos);
                pos += 4;

                /* has_texture */
                g_mesh_data.has_texture = *(int*)(data + pos);
                pos += 4;

                /* Skip texture name if present */
                if (g_mesh_data.has_texture) {
                    int tl = *(int*)(data + pos);
                    pos += 4 + tl;
                }

                /* bool_flag */
                g_mesh_data.bool_flag = *(int*)(data + pos);
                pos += 4;

                /* strip_count */
                g_mesh_data.strip_count = *(int*)(data + pos);
                pos += 4;

                /* Read strips */
                int s;
                for (s = 0; s < g_mesh_data.strip_count && s < 12; s++) {
                    g_mesh_data.strips[s][0] = *(int*)(data + pos);      /* vref */
                    g_mesh_data.strips[s][1] = *(int*)(data + pos + 4);  /* count */
                    pos += 8;
                }
            }
        }
    }

    free(data);
    g_mesh_loaded = 1;
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Spawn a MeshBuffer at a given position with a given color
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Color presets for GRID01-05: Red, Orange, Yellow, Green, Blue */
static const float GRID_COLORS[5][3] = {
    {0.8f, 0.0f, 0.0f},   /* GRID01 = Red */
    {1.0f, 0.5f, 0.0f},   /* GRID02 = Orange */
    {1.0f, 1.0f, 0.0f},   /* GRID03 = Yellow */
    {0.0f, 0.8f, 0.0f},   /* GRID04 = Green */
    {0.0f, 0.0f, 0.8f},   /* GRID05 = Blue */
};

static void spawn_testcube_at(DWORD meshworld, float px, float py, float pz, int grid_num, FILE* logf) {
    if (!g_mesh_loaded || !meshworld) return;

    /* Read current MeshBuffer count — this is also the next RC index */
    int rc_count = 0;
    if (!IsBadReadPtr((void*)(meshworld + MW_MB_COUNT), 4)) {
        rc_count = *(int*)(meshworld + MW_MB_COUNT);
    }
    if (rc_count < 0 || rc_count > 10000) {
        if (logf) fprintf(logf, "  GRID: bad rc_count=%d, skipping\n", rc_count);
        return;
    }

    /* ── Reallocate the RenderContext array ──
     * The game allocates RC array as operator_new(mb_count * 0x50 + 4).
     * We need (rc_count + 1) entries, so reallocate and copy.
     * The +4 at the start stores the count (it's the first DWORD before the array).
     */
    DWORD* old_rc = *(DWORD**)(meshworld + MW_RENDERCTX_ARRAY);
    int new_count = rc_count + 1;
    int new_alloc_size = new_count * 0x50 + 4;
    DWORD* new_rc = (DWORD*)LocalAlloc(LPTR, new_alloc_size);
    if (!new_rc) {
        if (logf) fprintf(logf, "  GRID: failed to realloc RC array\n");
        return;
    }
    /* Copy old RC data (including the count DWORD at offset -4... actually the
     * game stores the count at [array - 4], i.e. the DWORD right before the array).
     * From decompilation: operator_new(count * 0x50 + 4), then piVar3 = local_15c + 1,
     * and *local_15c = count. So the structure is: [count DWORD][RC entries...].
     */
    if (old_rc) {
        /* old_rc points to the first RC entry. The count DWORD is at old_rc - 1. */
        int old_alloc_size = rc_count * 0x50 + 4;
        if (!IsBadReadPtr((void*)((DWORD*)old_rc - 1), old_alloc_size)) {
            memcpy((DWORD*)new_rc, (DWORD*)old_rc - 1, old_alloc_size);
        }
    }
    /* Update count in the new allocation */
    *new_rc = new_count;
    /* Update the RC array pointer in MeshWorld (points to first entry, after count) */
    *(DWORD**)(meshworld + MW_RENDERCTX_ARRAY) = new_rc + 1;

    /* ── Allocate a MeshBuffer (0x874 bytes) ── */
    void* alloc = (void*)LocalAlloc(LPTR, 0x874);
    if (!alloc) {
        if (logf) fprintf(logf, "  GRID: failed to allocate MeshBuffer for grid=%d\n", grid_num);
        return;
    }

    /* Create the MeshBuffer using game's constructor */
    void* mb = pfn_CreateMeshBuffer(alloc);
    if (!mb) {
        LocalFree(alloc);
        if (logf) fprintf(logf, "  GRID: CreateMeshBuffer failed for grid=%d\n", grid_num);
        return;
    }

    /* Set the "has geometry" flag at +0x217 */
    *(BYTE*)((char*)mb + 0x217) = 1;

    /* Set name */
    char name_buf[32];
    snprintf(name_buf, 32, "testcube(GRID%02d)", grid_num);
    char* name = (char*)LocalAlloc(LPTR, strlen(name_buf) + 1);
    if (name) {
        strcpy(name, name_buf);
        *(char**)((char*)mb + MB_NAME_PTR) = name;
    }

    /* Add to MeshWorld's MeshBuffer list */
    void* mblist = (void*)(meshworld + MW_MBLIST);
    pfn_AthenaList_Append(mblist, mb);

    /* Set MeshBuffer's RenderContext index */
    *(int*)((char*)mb + MB_RENDERCTX_INDEX) = rc_count;

    /* ── Write position into the new RC entry ──
     * From Scene_LoadMeshWorld decompilation:
     *   posZ at RC+0x14, posX at RC+0x18, posY at RC+0x1C, scale at RC+0x20
     *   diffuse at RC+0x04..0x10 (RGBA)
     *   specular at RC+0x24..0x30
     *   emissive at RC+0x34..0x40
     *   shine at RC+0x44, texture at RC+0x48, bool at RC+0x4D
     */
    char* rc = (char*)(new_rc + 1) + rc_count * 0x50;

    /* Position (stored in "ambient" group, which the game also uses as position) */
    *(float*)(rc + 0x14) = pz;   /* posZ */
    *(float*)(rc + 0x18) = px;   /* posX */
    *(float*)(rc + 0x1C) = py;   /* posY */
    *(float*)(rc + 0x20) = 1.0f; /* scale = 1.0 */

    /* Scale flag (RC+0x4C = bool: scale != 1.0) */
    *(BYTE*)(rc + 0x4C) = 0;

    /* Diffuse color */
    int color_idx = grid_num - 1;
    if (color_idx < 0) color_idx = 0;
    if (color_idx > 4) color_idx = 4;

    *(float*)(rc + 0x04) = GRID_COLORS[color_idx][0];  /* R */
    *(float*)(rc + 0x08) = GRID_COLORS[color_idx][1];  /* G */
    *(float*)(rc + 0x0C) = GRID_COLORS[color_idx][2];  /* B */
    *(float*)(rc + 0x10) = 1.0f;                        /* A */

    /* Specular (zero) */
    *(float*)(rc + 0x24) = 0.0f;
    *(float*)(rc + 0x28) = 0.0f;
    *(float*)(rc + 0x2C) = 0.0f;
    *(float*)(rc + 0x30) = 0.0f;

    /* Emissive (zero) */
    *(float*)(rc + 0x34) = 0.0f;
    *(float*)(rc + 0x38) = 0.0f;
    *(float*)(rc + 0x3C) = 0.0f;
    *(float*)(rc + 0x40) = 0.0f;

    /* Shine */
    *(float*)(rc + 0x44) = 0.0f;

    /* No texture */
    *(DWORD*)(rc + 0x48) = 0;

    /* Bool flag */
    *(BYTE*)(rc + 0x4D) = 0;

    /* Update MeshBuffer count */
    *(int*)(meshworld + MW_MB_COUNT) = new_count;

    if (logf) {
        fprintf(logf, "  GRID: spawned testcube(GRID%02d) at (%.1f,%.1f,%.1f) mb=0x%08X rc_idx=%d\n",
                grid_num, px, py, pz, (DWORD)mb, rc_count);
        fflush(logf);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Scan S1 ref points and spawn testcubes at GRID positions
 * ═══════════════════════════════════════════════════════════════════════════ */

static void apply_grid_visibility(DWORD board, FILE* logf) {
    DWORD sceneobj = get_sceneobj(board);
    if (!sceneobj) {
        if (logf) fprintf(logf, "  GRID: no sceneobj\n");
        return;
    }

    DWORD meshworld = find_meshworld(board, logf);
    if (!meshworld) {
        if (logf) fprintf(logf, "  GRID: no meshworld found\n");
        return;
    }

    /* S1 list is embedded AthenaList at sceneobj+0x894 */
    DWORD s1_list = sceneobj + SCENEOBJ_S1_LIST;
    if (IsBadReadPtr((void*)(s1_list + 0x04), 4)) return;
    int s1_count = *(int*)(s1_list + 0x04);
    if (s1_count < 1 || s1_count > 10000) return;

    if (IsBadReadPtr((void*)(s1_list + 0x40C), 4)) return;
    DWORD* s1_array = *(DWORD**)(s1_list + 0x40C);
    if (!s1_array || IsBadReadPtr(s1_array, s1_count * 4)) return;

    if (logf) fprintf(logf, "  GRID: scanning %d S1 ref points, meshworld=0x%08X\n", s1_count, meshworld);

    int found = 0;
    int i;
    for (i = 0; i < s1_count; i++) {
        DWORD entry = s1_array[i];
        if (!entry || entry < 0x10000) continue;
        if (IsBadReadPtr((void*)entry, 0x20)) continue;

        char* name = *(char**)(entry + S1ENTRY_NAME);
        if (!name || IsBadReadPtr(name, 4)) continue;

        int grid_num = parse_grid_flag(name);
        if (grid_num == 0) continue;

        found++;

        float px = *(float*)(entry + S1ENTRY_POS_X);
        float py = *(float*)(entry + S1ENTRY_POS_Y);
        float pz = *(float*)(entry + S1ENTRY_POS_Z);

        /* Spawn testcube at this position */
        spawn_testcube_at(meshworld, px, py, pz, grid_num, logf);
    }

    if (logf) fprintf(logf, "  GRID: found %d GRID ref points\n", found);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Main mod thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static void init_game_dir(void) {
    char path[MAX_PATH];
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&init_game_dir, &hSelf);
    if (hSelf && GetModuleFileNameA(hSelf, path, MAX_PATH) > 0) {
        char *p = strrchr(path, '\\');
        if (p) { *p = '\0'; strcpy(g_game_dir, path); return; }
    }
    if (GetCurrentDirectoryA(MAX_PATH, path) > 0) {
        strcpy(g_game_dir, path);
    }
}

static DWORD WINAPI mod_thread(LPVOID param) {
    Sleep(3000);

    {
        char log_path[MAX_PATH];
        snprintf(log_path, MAX_PATH, "%s\\custom_entities.log", g_game_dir);
        FILE* f = NULL;
        fopen_s(&f, log_path, "a");
        if (f) {
            fprintf(f, "=== Custom Entities Mod v11 Started ===\n");
            fprintf(f, "Game dir: %s\n", g_game_dir);
            fclose(f);
        }
    }

    /* Load testcube mesh data */
    if (load_testcube_mesh()) {
        char log_path[MAX_PATH];
        snprintf(log_path, MAX_PATH, "%s\\custom_entities.log", g_game_dir);
        FILE* f = NULL;
        fopen_s(&f, log_path, "a");
        if (f) {
            fprintf(f, "Loaded testcube.MESHWORLD: %d verts, %d strips\n",
                    g_mesh_data.vertex_count, g_mesh_data.strip_count);
            fclose(f);
        }
    } else {
        char log_path[MAX_PATH];
        snprintf(log_path, MAX_PATH, "%s\\custom_entities.log", g_game_dir);
        FILE* f = NULL;
        fopen_s(&f, log_path, "a");
        if (f) {
            fprintf(f, "FAILED to load testcube.MESHWORLD from %s\n", g_game_dir);
            fclose(f);
        }
    }

    while (g_running) {
        DWORD board = get_board();
        if (!board) {
            Sleep(100);
            continue;
        }

        DWORD level = get_level(board);
        if (level != g_last_level && level) {
            g_last_level = level;

            /* Poll until sceneobj is ready (up to 10 seconds) */
            int poll;
            for (poll = 0; poll < 200; poll++) {
                DWORD so = get_sceneobj(board);
                if (so) break;
                Sleep(50);
            }

            char log_path[MAX_PATH];
            snprintf(log_path, MAX_PATH, "%s\\custom_entities.log", g_game_dir);
            FILE* logf = NULL;
            fopen_s(&logf, log_path, "a");

            if (logf) fprintf(logf, "\n--- Level loaded (board=0x%08X, level=0x%08X, after %dms poll) ---\n", board, level, poll * 50);

            /* Apply GRID: spawn testcubes at GRID positions */
            if (g_mesh_loaded) {
                apply_grid_visibility(board, logf);
            } else {
                if (logf) fprintf(logf, "  GRID: testcube mesh not loaded\n");
            }

            if (logf) {
                fprintf(logf, "Done.\n\n");
                fclose(logf);
            }
        }

        Sleep(2000);
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        load_real_bass();
        init_game_dir();
        g_thread = CreateThread(NULL, 0, mod_thread, NULL, 0, NULL);
    }
    else if (reason == DLL_PROCESS_DETACH) {
        g_running = 0;
        if (g_thread) {
            WaitForSingleObject(g_thread, 2000);
            CloseHandle(g_thread);
        }
    }
    return TRUE;
}
