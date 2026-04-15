# Container & Utility Systems

## AthenaList — Dynamic Array / Linked Index Array

AthenaList is the primary container used throughout Hamsterball. It combines a
dynamic array with a 256-entry free index system for O(1) iteration.

### Structure Layout (0x418 bytes)

| Offset | Type | Description |
|--------|------|-------------|
| +0x00 | vtable* | IndexList_Dtor (0x4D875C) |
| +0x04 | int | Count (number of elements) |
| +0x08 | int[256] | Free index table (0x400 bytes, 256 entries) |
| +0x408 | int | Next free index (iterator, wraps at 0xFF) |
| +0x40C | void** | Data pointer (malloc'd array of void*, count*4 bytes) |
| +0x410 | int | Capacity hint (0 on init) |
| +0x414 | int | Sorted mode flag (1=sorted, 0=unsorted) |

### Key Functions

| Address | Function | Xrefs | Description |
|---------|----------|-------|-------------|
| 0x453210 | AthenaList_Init | 112 | Initialize list with sorted flag |
| 0x453280 | AthenaList_Free | 68 | Free data array, reset count |
| 0x4532B0 | AthenaList_NextIndex | 256 | Get next free index (wraps at 255) |
| 0x4532E0 | AthenaList_SortedInsert | 2 | Insert in sorted order (when +0x414=1) |
| 0x453610 | AthenaList_ContainsValue | 10 | Check if value exists |
| 0x453640 | AthenaList_FindByValue | 4 | Find value, return index |
| 0x4536A0 | AthenaList_GetSize | 60 | Return count at +0x04 |
| 0x4536B0 | AthenaList_InsertAt | 7 | Insert at specific index |
| 0x453780 | AthenaList_Append | 22 | Append raw (no sorted check) |
| 0x453810 | AthenaList_Append | 301 | Append with sorted-mode dispatch |
| 0x453820 | AthenaList_MergeSorted | 10 | Merge two sorted lists |
| 0x467E40 | AthenaList_Ctor | 4 | Constructor |
| 0x46B1A0 | AthenaListObj_ctor | 7 | Object list constructor |
| 0x475730 | AthenaList_WriteToFile | 1 | Serialize list to file |
| 0x489280 | AthenaList_FreeAllChunks | 3 | Free all chunks |
| 0x489540 | AthenaList_SplitChunk | 3 | Split a chunk |
| 0x488710 | AthenaList_IterateNext | 15 | Get next item in iteration |
| 0x4897A0 | AthenaList_WriteDword | 3 | Write DWORD to list file |

### Initialization (AthenaList_Init)
```c
AthenaList_Init(this, sorted_flag):
    this->sorted = sorted_flag;    // +0x414
    this->count = 0;               // +0x04
    this->next_index = 0;          // +0x08..+0x408 (256 entries zeroed)
    this->vtable = &IndexList_Dtor;
    for (i = 0; i < 256; i++)      // Clear free index table
        this->free_table[i] = 0;
    this->iter_idx = 0;            // +0x408
```

### Append (AthenaList_Append, 301 xrefs)
```c
AthenaList_Append(this, value):
    if (this->sorted != 0)         // +0x414
        return AthenaList_SortedInsert(this, value);
    
    if (this->count == 0) {        // +0x04
        this->count = 1;
        this->data = malloc(4);    // +0x40C
        // Clear free index table
        for (i = 0; i < 256; i++)
            this->free_table[i] = 0;
        this->data[0] = value;
    } else {
        this->count++;
        this->data = realloc(this->data, this->count * 4);
        this->data[this->count - 1] = value;
    }
```

### Free (AthenaList_Free)
```c
AthenaList_Free(this):
    if (this->data != NULL)        // +0x40C
        free(this->data);
    this->data = NULL;
    this->count = 0;               // +0x04
```

### NextIndex (AthenaList_NextIndex, 256 xrefs)
```c
AthenaList_NextIndex(this):
    this->iter_idx++;               // +0x408
    if (this->iter_idx > 0xFF)     // Wraps at 255
        this->iter_idx = 1;
    return this->iter_idx;
```

### Iteration Pattern (from decompilations)
```c
// Standard iteration pattern used throughout the codebase:
int idx = AthenaList_NextIndex(&list);
void *item = (list.count < 1) ? NULL : list.data[0];
while (item != NULL) {
    // Process item
    idx = list.free_table[idx];
    if (list.count <= idx) break;
    item = list.data[idx];
    list.free_table[idx]++;
}
AthenaList_Free(&list);
```

## Texture System

### Texture Structure (0x74 bytes)
Created by Graphics_FindOrCreateTexture (0x455C50):

| Offset | Type | Description |
|--------|------|-------------|
| +0x00 | vtable* | Texture vtable |
| +0x08 | char* | Texture name/path (strdup'd) |
| +0x0C | void** | D3D texture pointer array (from +0x6F0 in list) |
| +0x10 | int | Reference count (incremented on cache hit) |
| +0x14-0x70 | ... | D3D texture data, mip levels, format info |

### Graphics_FindOrCreateTexture (0x455C50)
7 xrefs — texture loading with caching:
```c
Texture* Graphics_FindOrCreateTexture(GfxEngine* this, char* path, char use_cache) {
    if (use_cache == 1) {
        // Linear search through texture list at this+0x2E4
        for each texture in this->texture_list:
            if (stricmp(texture->name, path) == 0) {
                texture->refcount++;  // +0x10
                return texture;       // Cache hit
            }
    }
    
    // Cache miss: load new texture
    Texture* tex = operator_new(0x74);
    Texture_LoadWithMips(tex, this, path);
    AthenaList_Append(this + 0x2E4, tex);  // Add to cache
    return tex;
}
```

### Texture Cache Location
- GfxEngine+0x2E4: AthenaList texture cache (all loaded textures)
- GfxEngine+0x6F0: Texture data array
- Lookup: case-insensitive strcmp (stricmp)
- Key: texture file path (e.g., "textures\hammy1.png")

### Texture Loading
- PNG/BMP files loaded via D3DXCreateTextureFromFile
- Alpha channel enabled for PNG
- Mipmaps generated via Texture_LoadWithMips
- 7 xrefs to FindOrCreateTexture

## AthenaString — Dynamic String

| Address | Function | Xrefs | Description |
|---------|----------|-------|-------------|
| 0x473500 | AthenaString_AssignCStr | 75 | Assign from C string |
| 0x4736B0 | AthenaString_dtor | 85 | Destructor |
| 0x473A50 | AthenaString_AssignCRLF | 21 | Assign with CRLF handling |
| 0x4BAE43 | AthenaString_SprintfToBuffer | 71 | sprintf to internal buffer |
| 0x4BBDFD | AthenaString_Sprintf | — | Format string |
| 0x466C70 | AthenaString_Format | — | Format with args |
| 0x4737F0 | AthenaString_Assign | — | Copy assignment |
| 0x473940 | FontFormatString_Parse | — | Parse printf-style formats |

### AthenaString Structure (estimated 0x1C bytes)
| Offset | Type | Description |
|--------|------|-------------|
| +0x00 | vtable* | AthenaString vtable |
| +0x04 | char* | String data (malloc'd) |
| +0x08 | int | Length |
| +0x0C | int | Capacity |
| +0x10 | int | Ref count (if shared) |

## Hash Table (MESHWORLD Lookup)

The game uses a hash table for level object lookup (Scene+0x8AC). Objects
are found by name strings like "START0-0", "SAFESPOT", "CAMERALOCUS".

Key hash table functions:
- Scene_FindObjectByName: resolves mesh name to object
- Used by Scene_SpawnBallsAndObjects for start positions
- Used by Level_InitScene for camera targets (CAMERALOCUS, CAMERALOOKAT)

## Graphics Utility Functions

| Address | Function | Xrefs | Description |
|---------|----------|-------|-------------|
| 0x455D60 | Graphics_DrawScreenRect | 63 | Draw 2D rectangle on screen |
| 0x455110 | Graphics_ApplyMaterialAndDraw | — | Set material + draw mesh |
| 0x454AB0 | Graphics_SetProjection | 31 | Set perspective projection |
| 0x454F10 | Graphics_SetViewport | 8 | Set viewport dimensions |
| 0x453B50 | Graphics_BeginFrame | — | Start frame, clear buffers |
| 0x455A90 | Graphics_PresentOrEnd | — | Present frame, swap buffers |
| 0x453900 | Graphics_ClearViewport | — | Clear color/depth |
| 0x453970 | Graphics_SetCullMode2 | — | Set cull mode |
| 0x4539A0 | Graphics_SetViewportZ | — | Set depth range |

## CRT / Standard Library Usage

The game uses the following CRT patterns:
- malloc/realloc/free for dynamic arrays (AthenaList)
- operator_new/operator_delete for object allocation
- fopen/fread/fclose for file I/O (font.description, .mesh, .meshworld)
- _open with O_RDONLY (0x8000) for font loading
- stricmp for case-insensitive string comparison (texture cache, menu items)
- sprintf for string formatting (AthenaString_Sprintf)
- GetTickCount for timing (game loop)
- PeekMessageA/TranslateMessage/DispatchMessageA for Windows message loop
- Sleep(0) for yielding between frames