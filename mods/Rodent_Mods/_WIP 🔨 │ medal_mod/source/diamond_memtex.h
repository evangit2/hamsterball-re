/* ================================================================
 * diamond_memtex.h — Route 2 (fully in-memory) diamond weasel art.
 *
 * The diamond medal + mini icon are embedded as XOR-encrypted raw
 * A8R8G8B8 pixels (diamond_pixels.h). At runtime we:
 *   1. decrypt the pixels in memory,
 *   2. create a D3D8 texture via the game's device (CreateTexture,
 *      LockRect, row memcpy, UnlockRect),
 *   3. build a texture-cache object with the EXACT layout the game's
 *      loader (FUN_00476770) produces, using the game's own vtable
 *      0x4DA648 so the draw path is byte-identical,
 *   4. append it to the game's texture cache fixed list (gfx+0x2e4).
 *
 * When the medal sprite is requested by name ("diamondweasel.png"),
 * FUN_00455c50 finds our pre-seeded object and the file loader NEVER
 * runs. No PNG, no decrypted bytes, no temp file ever touches disk.
 * ================================================================ */
#ifndef DIAMOND_MEMTEX_H
#define DIAMOND_MEMTEX_H

#include <windows.h>
#include <string.h>

/* Game constants */
#define EXE_BASE_MT        0x00400000
#define APP_PTR_MT         0x005341E0
#define APP_BOARD_MT       0x178      /* App -> gfx */
#define GFX_DEVICE_MT      0x154      /* gfx -> IDirect3DDevice8* */
#define GFX_TEXCACHE_MT    0x2E4      /* gfx -> texture cache AthenaList (fixed) */
#define TEXCACHE_VTABLE    0x004DA648 /* game's texture-wrapper vtable */

/* D3D8 enums */
#define D3DFMT_A8R8G8B8   0x15
#define D3DPOOL_MANAGED   1

/* D3D8 device vtable slots (this_ = IDirect3DDevice8*) */
#define D3DDEV_CreateTexture 0x50
/* D3D8 texture vtable slots (tex_ = IDirect3DTexture8*) */
#define TEX_LockRect         0x3C
#define TEX_UnlockRect       0x38

typedef struct { INT Pitch; void *pBits; } D3DLOCKED_RECT_MT;

/* Texture-cache object layout (0x74 bytes), as built by FUN_00476770 */
typedef struct {
    DWORD vtable;   /* +0  = 0x4DA648 */
    void *d3dTex;   /* +4  = IDirect3DTexture8* */
    char *name;     /* +8  = malloc'd filename copy */
    void *gfx;      /* +0xC = graphics context */
    int   refcount; /* +0x10 */
    int   width;    /* +0x14 */
    int   height;   /* +0x18 */
    BYTE  alphaFmt; /* +0x1C (1 if A8R8G8B8) */
    BYTE  loaded;   /* +0x1D */
    BYTE  pad1E;    /* +0x1E */
    BYTE  pad1F;    /* +0x1F */
    int   mips;     /* +0x20 */
    BYTE  rest[0x74 - 0x24]; /* +0x24..+0x73 unused padding */
} TexCacheObj;

/* XOR-decrypt embedded pixels into a caller buffer. Returns bytes written. */
static DWORD diamond_decrypt(const unsigned char *enc, DWORD size, DWORD key,
                             unsigned char *out) {
    DWORD i;
    for (i = 0; i < size; i++)
        out[i] = enc[i] ^ (unsigned char)(key >> ((i & 3) * 8));
    return size;
}

/* Build a D3D8 texture from raw A8R8G8B8 pixels via the game's device.
 * dev = IDirect3DDevice8* (from gfx+0x154). Returns texture or NULL. */
static void *diamond_create_d3dtex(void *dev, const unsigned char *px,
                                   int w, int h) {
    void *(*CreateTexture)(void*, unsigned, unsigned, unsigned, DWORD,
                           DWORD, DWORD, void**) = NULL;
    int (*LockRect)(void*, unsigned, D3DLOCKED_RECT_MT*, const void*, DWORD) = NULL;
    int (*UnlockRect)(void*, unsigned) = NULL;
    void *tex = NULL;
    D3DLOCKED_RECT_MT rect;
    void **vt;
    int y, hres;

    if (!dev || !px) return NULL;
    vt = *(void***)dev;
    CreateTexture = (void*)(vt)[D3DDEV_CreateTexture/4];
    if (!CreateTexture) return NULL;
    /* CreateTexture(width, height, levels=1, usage=0, A8R8G8B8, MANAGED, &tex) */
    hres = (int)CreateTexture(dev, (unsigned)w, (unsigned)h, 1, 0,
                              D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &tex);
    if (hres < 0 || !tex) return NULL;

    vt = *(void***)tex;
    LockRect   = (int(*)(void*,unsigned,D3DLOCKED_RECT_MT*,const void*,DWORD))vt[TEX_LockRect/4];
    UnlockRect = (int(*)(void*,unsigned))vt[TEX_UnlockRect/4];
    if (!LockRect || !UnlockRect) return NULL;
    if (LockRect(tex, 0, &rect, NULL, 0) < 0) return NULL;
    /* Copy row by row (source tightly-packed BGRA, dest has Pitch stride) */
    for (y = 0; y < h; y++)
        memcpy((char*)rect.pBits + (size_t)y * (size_t)rect.Pitch,
               px + (size_t)y * (size_t)w * 4u, (size_t)w * 4u);
    UnlockRect(tex, 0);
    return tex;
}

/* Build a texture-cache object in the game's exact layout and append it to
 * the texture cache list at gfx+0x2E4. Returns the object or NULL. */
static TexCacheObj *diamond_seed_one(void *gfx, void *dev, const char *name,
                                     const unsigned char *enc, DWORD encsize,
                                     DWORD key, int w, int h) {
    TexCacheObj *obj;
    unsigned char *px;
    char *namecopy;
    void *tex;
    void *list;      /* embedded AthenaList at gfx+0x2E4 */
    void (*AthenaList_Append)(void*, void*) = (void*)0x00453780;

    if (!gfx || !dev) return NULL;
    px = (unsigned char*)malloc(encsize);
    if (!px) return NULL;
    diamond_decrypt(enc, encsize, key, px);
    tex = diamond_create_d3dtex(dev, px, w, h);
    free(px);
    if (!tex) return NULL;

    obj = (TexCacheObj*)malloc(0x74);
    namecopy = (char*)malloc(strlen(name) + 1);
    if (!obj || !namecopy) { if (obj) free(obj); return NULL; }
    strcpy(namecopy, name);

    memset(obj, 0, 0x74);
    obj->vtable   = TEXCACHE_VTABLE;
    obj->d3dTex   = tex;
    obj->name     = namecopy;
    obj->gfx      = gfx;
    obj->refcount = 1;
    obj->width    = w;
    obj->height   = h;
    obj->alphaFmt = 1;   /* A8R8G8B8 */
    obj->loaded   = 1;
    obj->mips     = 1;

    /* Append to the embedded AthenaList at gfx+0x2E4 using the game's own
     * AthenaList_Append (0x453780, __thiscall ecx=list, item on stack).
     * This matches how FUN_00455c50 appends cache objects and handles the
     * items-array pointer (+0x40C) and growth correctly. */
    list = (char*)gfx + GFX_TEXCACHE_MT;
    __asm__ volatile(
        "pushl %1\n\t"
        "movl %0, %%ecx\n\t"
        "call *%2\n\t"
        : : "r"(list), "r"(obj), "r"(AthenaList_Append)
        : "eax", "ecx", "edx", "memory"
    );
    return obj;
}

/* Seed both diamond textures (medal 256x256 + mini 32x32) into the cache.
 * gfx = [App+0x174]. Returns nonzero if both seeded (or already present). */
static int diamond_seed_cache(void *gfx) {
    void *dev;
    TexCacheObj *a, *b;
    if (!gfx) return 0;
    dev = *(void**)((char*)gfx + GFX_DEVICE_MT);
    if (!dev) return 0;

    a = diamond_seed_one(gfx, dev, "diamondweasel.png",
                         diamondweasel_data, diamondweasel_SIZE,
                         diamondweasel_XORKEY,
                         diamondweasel_WIDTH, diamondweasel_HEIGHT);
    b = diamond_seed_one(gfx, dev, "diamondweasel-icon.png",
                         diamondweasel_icon_data, diamondweasel_icon_SIZE,
                         diamondweasel_icon_XORKEY,
                         diamondweasel_icon_WIDTH, diamondweasel_icon_HEIGHT);
    return (a && b) ? 1 : 0;
}

#endif /* DIAMOND_MEMTEX_H */