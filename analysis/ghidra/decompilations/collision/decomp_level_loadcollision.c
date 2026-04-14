// Level_LoadCollision @ 0x00465260
// Collision geometry loader from .COL binary file
//
// File format (read via __read from file handle):
//   Header: 24 bytes into MeshWorld+0x45C (transform/flags data)
//   int32: sublevel_count
//     if count < 1 (single-mesh collision):
//       int32: object_count
//       for each object:
//         CreateMeshBuffer(0x874 bytes) → scene+0x2C list
//         int32: name_length
//         char[name_length]: name string (e.g., "N:WALL", "E:SWITCH")
//           - "N:" prefix → set obj+0x85D = 1 (interactive flag)
//           - "E:" prefix → set obj+0x85D = 1, obj+0x863 = 1 (event+no_render)
//         int32: face_count
//         for each face:
//           Allocate CollisionFace (0x60 bytes)
//           Read 9 floats: v0(x,y,z), v1(x,y,z), v2(x,y,z) at offsets 0,8,0x10
//           Compute face normal via cross product:
//             edge1 = v0 - v1, edge2 = v0 - v2
//             normal = cross(edge2, edge1)  (note: reversed from typical)
//           Normalize via Graphics_InitShaderDispatch (SIMD normalize)
//           Store same normal for all 3 vertices of the face
//           Append to object's face list
//     else (multi-level collision):
//       Set scene+0x430 = 1 (sprite/multi-level mode)
//       for each sublevel:
//         Level_ctor(new(0x10D0), gfx)
//         Set vtable to Level_DeletingDtor2 (0x4D9068)
//         Append to scene+0x18 (sublevel list)
//         Call level->vtable[0x60](file_handle) (LoadSubLevel)
//
// CollisionFace layout (0x60 bytes):
//   +0x00: float v0.x
//   +0x04: float v0.y
//   +0x08: float v0.z
//   +0x0C: float normal.x  (computed from cross product, then normalized)
//   +0x10: float normal.y
//   +0x14: float normal.z
//   +0x18: float v1.x
//   +0x1C: float v1.y
//   +0x20: float v1.z
//   +0x24: float normal.x  (same as v0's normal - flat shading)
//   +0x28: float normal.y
//   +0x2C: float normal.z
//   +0x30: float v2.x
//   +0x34: float v2.y
//   +0x38: float v2.z
//   +0x3C: float normal.x  (same again)
//   +0x40: float normal.y
//   +0x44: float normal.z
//   (remaining: padding/AABB data)
//
// MeshBuffer layout (0x874 bytes):
//   +0x000: vtable
//   +0x00C: AthenaList face_list (at obj+3*4 = +0x0C)
//   +0x219*4 (+0x864): name string pointer
//   +0x217: render_flag (=0)
//   +0x85D: interactive_flag (1 for N:/E: prefixes)
//   +0x863: no_render_flag (1 for E: prefix only)

void __thiscall Level_LoadCollision(Scene *scene, int file_handle)
{
  // Create MeshWorld for collision
  MeshWorld *meshworld = MeshWorld_ctor(new(0x488), scene->gfx);
  scene->meshworld = meshworld;  // +0x8
  meshworld->flag_459 = 0;
  scene->has_collision = 1;  // +0xD
  
  // Read header
  __read(file_handle, meshworld + 0x45C, 0x18);
  __read(file_handle, &sublevel_count, 4);
  
  if (sublevel_count < 1) {
    // Single-mesh mode: read objects + faces
    __read(file_handle, &object_count, 4);
    for (int i = 0; i < object_count; i++) {
      MeshBuffer *buf = CreateMeshBuffer(new(0x874));
      buf->render_flag = 0;
      AthenaList_Append(meshworld->object_list, buf);  // +0x2C
      
      // Read name
      __read(file_handle, &name_len, 4);
      char *name = malloc(name_len);
      buf->name = name;
      __read(file_handle, name, name_len);
      if (strnicmp(name, "N:", 2) == 0) buf->interactive = 1;  // +0x85D
      if (strnicmp(name, "E:", 2) == 0) { buf->interactive = 1; buf->no_render = 1; } // +0x85D, +0x863
      
      // Read faces
      __read(file_handle, &face_count, 4);
      for (int j = 0; j < face_count; j++) {
        CollisionFace *face = new(0x60);
        __read(file_handle, &face->v0, 12);     // 3 floats
        __read(file_handle, &face->v1, 12);     // 3 floats at +0x20
        __read(file_handle, &face->v2, 12);     // 3 floats at +0x40
        
        // Compute face normal
        Vec3 e1 = v0 - v1, e2 = v0 - v2;
        Vec3 normal = cross(e2, e1);
        normalize(&normal);  // via Graphics_InitShaderDispatch (SIMD)
        
        // Store same normal for all 3 verts (flat shading)
        face->n0 = face->n1 = face->n2 = normal;
        AthenaList_Append(buf->face_list, face);
      }
    }
  } else {
    // Multi-level mode
    scene->is_sprite_mode = 1;  // +0x430
    for (int i = 0; i < sublevel_count; i++) {
      Level *sub = Level_ctor(new(0x10D0), scene->gfx);
      sub->vtable = Level_DeletingDtor2_vtable;
      AthenaList_Append(scene->sublevel_list, sub);  // +0x18
      sub->vtable[0x60](file_handle);  // LoadSubLevel
    }
  }
}