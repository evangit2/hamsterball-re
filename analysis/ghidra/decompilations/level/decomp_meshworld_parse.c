/*
 * MeshWorld_Parse (0x470930) — Full decompilation
 * 
 * Parses .ASE (3DS Max ASCII Scene Export) format files.
 * This is the core level/mesh loader for Hamsterball.
 * Levels and meshes are stored as .ASE text files.
 */

/*
 * .ASE FILE FORMAT (3DS Max ASCII Scene Export)
 * 
 * Parsed line-by-line via strtok with space/tab delimiters.
 * All values are text-format floats or integers.
 * 
 * KEYWORDS PARSED:
 * 
 * *MATERIAL_COUNT <n>     — Set number of materials
 *   - Allocates array of n RenderContext structs (0x50 bytes each)
 *   - Stored at MeshWorld+0x24 (count) and MeshWorld+0x28 (array)
 * 
 * *MATERIAL <n>            — Start material definition for index n
 *   - Sets local_5b8 = n (current material index)
 *   - Uses material n * 0x50 as offset into the array
 * 
 * *MATERIAL_AMBIENT <r> <g> <b>
 *   - Ambient color (3 floats) → material[n].ambient (+0x14..+0x20)
 *   - Alpha = 1.0 (0x3F800000) forced
 * 
 * *MATERIAL_DIFFUSE <r> <g> <b>  
 *   - Diffuse color (3 floats) → material[n].diffuse (+0x04..+0x10)
 *   - Alpha = 1.0 forced
 * 
 * *MATERIAL_SPECULAR <r> <g> <b>
 *   - Specular color (3 floats) → material[n].specular (+0x24..+0x30)
 *   - Alpha = 1.0 forced
 * 
 * *MATERIAL_SHINE <value>
 *   - Shininess → material[n].shine (+0x44)
 *   - Stored as value * _DAT_004CF454 (constant multiplier)
 * 
 * *MATERIAL_SHINESTRENGTH <value>
 *   - Multiplies specular by value: spec *= value for each component
 * 
 * *MATERIAL_TRANSPARENCY <value>
 *   - Inverts transparency to opacity: 1.0 - value
 *   - Sets diffuse alpha (+0x10), ambient alpha (+0x20), 
 *     specular alpha (+0x30), and global alpha (+0x40)
 *   - If opacity ≠ 1.0: sets has_alpha flag (+0x4C = 1)
 * 
 * *MAP_REFLECT             — Enables reflection map flag
 *   - Sets material[n].reflect_flag (+0x4D = 1)
 * 
 * *MATERIAL_REF <refid>   — Set material reference ID
 *   - local_5a8[1] = refid
 * 
 * *BITMAP "<path>"         — Load texture
 *   - Strips quotes from filename
 *   - Calls Graphics_FindOrCreateTexture(gfx, path, 1) 
 *   - Stored at material[n].texture (+0x48)
 * 
 * *GEOMOBJECT             — Start new geometry object
 *   - Allocates CreateMeshBuffer (0x874 bytes)
 *   - Appends to MeshWorld+0x2C (mesh list)
 *   - Clears face list
 * 
 * *NODE_NAME "<name>"     — Set object name
 *   - Strips quotes from name string
 * 
 * *TM_POS <x> <y> <z>     — Set object position
 *   - Stored at mesh_buf[0x21A] (x, z, y — Y/Z swapped!)
 *   - Note: Y axis in file = Z in engine (Z-up convention)
 * 
 * *MESH_NUMVERTEX <n>     — Allocate vertex array
 *   - n * 0x20 bytes (32 bytes per vertex)
 *   - Vertex struct: float x, float z, float y, float nx, float ny, float nz, float u, float v
 * 
 * *MESH_NUMTVERTEX <n>    — Allocate texture coord array
 *   - n * 8 bytes (2 floats per UV: u, v)
 * 
 * *MESH_VERTEX <idx> <x> <z> <y>
 *   - Store position at vertex[idx].x = x, vertex[idx].y = z, vertex[idx].z = y
 *   - Note: Y/Z axes are SWAPPED (file Y = engine Z, file Z = engine Y)
 *   - Updates AABB bounds at MeshWorld+0x45C..0x470 (min/max X/Z/Y)
 * 
 * *MESH_NUMFACES <n>      — Allocate face array
 *   - n * 0x60 bytes (96 bytes per face)
 * 
 * *MESH_FACE <idx> A: <v0> B: <v1> C: <v2>
 *   - Store 3 vertex positions from vertex array into face struct
 *   - Each face copies: pos(x,y), normal(u,v), and UV(u,v) for each of 3 vertices
 *   - face.A = vertex[v0], face.B = vertex[v1], face.C = vertex[v2]
 * 
 * *MESH_TVERT <idx> <u> <v>
 *   - Texture coordinate (u, v) for vertex[idx]
 *   - UV v-coordinate is inverted (1.0 - v) → _DAT_004CF3C8 = 1.0
 *   - Stored as 2 floats at tvertex[idx*8]
 * 
 * *MESH_TFACE <fidx> <t0> <t1> <t2>
 *   - Texture face: maps face to texture vertices
 *   - For each: face[fidx].tv[i] = { tvertex[ti].u, 1.0 - tvertex[ti].v }
 *   - Note: V = 1.0 - v (texture V is flipped)
 * 
 * *MESH_FACENORMAL <idx> <nx> <ny> <nz>
 *   - Face normal vector + 3 vertex normals
 *   - Stored in face struct at offsets +0x0C..+0x14, +0x2C..+0x34, +0x4C..+0x54
 * 
 * *MESH_VERTEXNORMAL <vref> <nx> <ny> <nz>
 *   - Per-vertex normal
 *   - Component swizzle: vertex 0 stays, vertex 1↔2 swapped
 * 
 * MeshWorld Struct Offsets:
 *   +0x04  GfxEngine*       graphics engine
 *   +0x24  long             material_count
 *   +0x28  RenderContext*   material array (n * 0x50 bytes)
 *   +0x2C  AthenaList       mesh buffer list
 *   +0x30  int              mesh count
 *   +0x34  int              current mesh index (iterator)
 *   +0x438 AthenaList*      mesh data array
 *   +0x45C float            AABB min X
 *   +0x460 float            AABB min Z (file Y)
 *   +0x464 float            AABB min Y (file Z)
 *   +0x468 float            AABB max X
 *   +0x46C float            AABB max Z (file Y)
 *   +0x470 float            AABB max Y (file Z)
 * 
 * RenderContext (Material) Struct (0x50 bytes):
 *   +0x04  float[3+1]      diffuse color (R,G,B,A=1.0)
 *   +0x10  float            diffuse alpha (1.0 - transparency)
 *   +0x14  float[3+1]      ambient color (R,G,B,A=1.0)
 *   +0x20  float            ambient alpha (1.0 - transparency)
 *   +0x24  float[3+1]      specular color (R,G,B,A=1.0)
 *   +0x30  float            specular alpha (1.0 - transparency)
 *   +0x40  float            global alpha (1.0 - transparency)
 *   +0x44  float            shine (value * _DAT_004CF454)
 *   +0x48  Texture*         texture pointer
 *   +0x4C  byte             has_alpha flag
 *   +0x4D  byte             reflect_map flag
 * 
 * Face Struct (0x60 bytes):
 *   Each face stores 3 vertices, each with position + normal + UV:
 *   Vertex 0: +0x00 pos(x,y), +0x08 normal(u,v), +0x0C normal_xyz, +0x10 ?, +0x14 ?
 *   Vertex 1: +0x18 pos(x,y), +0x20 normal(u,v), +0x24 ?, +0x28 ?, +0x2C normal.xyz
 *   Vertex 2: +0x30 pos(x,y), +0x38 normal(u,v), +0x3C ?, +0x40 ?, +0x44 normal.xyz
 *   Texture UV: interleaved (u, 1.0-v) per vertex
 * 
 * MeshBuffer Struct (0x874 bytes):
 *   +0x00  vtable*
 *   +0x04  GfxEngine*
 *   +0x0C  AthenaList       face list (sorted, 0x418 bytes)
 *   +0x424 int             next_index
 *   +0x428 int             count
 *   +0x430 int             capacity
 *   +0x438 void**          data array
 *   +0x21A float           position x (from *TM_POS)
 *   +0x21B float           position y (from *TM_POS, Z in file)
 *   +0x21C float           position z (from *TM_POS, Y in file)
 *   +0x217 byte            active flag (0 = inactive)
 *   +0x43C RenderContext   per-mesh material override
 */