// Scene_RenderFrame @ 0x00460DA0
// Per-frame scene update: transform sprites, build vertex buffers, render text
//
// Flow:
//   1. Set scene+0x10C4 = 1 (render frame active flag)
//   2. Allocate MeshWorld (0x30 bytes) if scene+0x47C == this (root scene)
//      - MeshWorld_ctor(gfx, vertex_count*4, vertex_data)
//      - Allocate transform buffer: obj_count * 0x60 bytes
//   3. If this+0x430 (is_sprite_mode): call vtable[0x3C] for each sprite
//   4. For each mesh object in main list:
//      - SpriteAnim_SetRange(meshworld, g_renderIndex++)
//      - Store mesh's anim range pointer
//      - For each sub-strip in object's strip list:
//        - Select vertex array (indexed or interleaved based on ball_mode)
//        - Build triangle strip: alternating vertex triples for zigzag strip
//        - Track vertex counts in obj+0x840
//   5. If root scene: Font_RenderToTextureComplex() for text->texture
//      - Store font texture pointer at scene+0x44C
//      - Mesh_SaveAndFree + free the temp MeshWorld
//
// Triangle strip construction:
//   For each group in sub-strip:
//     bVar10 alternates false/true per group
//     local_18[0] = current vertex base
//     if (bVar10) { [2]=prev, [1]=current }
//     else        { [1]=prev, [2]=current }
//     For each of 3 vertices: Mesh_AddVertex(meshworld, vertex_ptr)
//     Advance by 0x20 bytes per vertex (32 bytes = pos + normal + texcoord)
//
// This produces a zigzag triangle strip pattern typical of D3D
// optimized mesh rendering with degenerate triangles between groups.

void __thiscall Scene_RenderFrame(void *this, MeshWorld *meshworld)
{
  scene->render_frame_active = 1;  // +0x10C4
  
  // Root scene allocates temp MeshWorld
  if (this->root_scene == this) {
    g_renderIndex = 0;
    meshworld = MeshWorld_ctor(new(0x30), gfx, vertex_count*4, vertex_data);
    scene->transform_buffer = new(obj_count * 0x60);
    scene->transform_count = 0;  // +0x10CC
  }
  
  // Sprite update
  if (this->is_sprite_mode) {
    for each (sprite in sprite_list)
      sprite->vtable[0x3C](meshworld);  // UpdateAnim
  }
  
  // Build vertex buffers for mesh objects
  for each (obj in scene_object_list) {
    SpriteAnim_SetRange(meshworld, g_renderIndex++);
    obj->anim_range = meshworld->range_ptr;  // +0x858
    
    for each (strip in obj->strip_list) {
      // Select vertex source based on render mode
      if (!scene->is_ball_mode) {
        vert_base = strip->vertex_offset_8;
        vert_array = meshworld->vertex_array_0x448;
      } else {
        vert_base = strip->vertex_offset_C;
        vert_array = scene->vertex_array_0x440;
      }
      
      base = vert_base * 0x20 + vert_array;
      bool flip = false;
      
      for (int i = 0; i < strip->count; i++) {
        verts[0] = base;
        if (flip) { verts[2] = base-0x20; verts[1] = base+0x20; }
        else      { verts[1] = base-0x20; verts[2] = base+0x20; }
        flip = !flip;
        obj->vertex_count++;  // +0x840
        
        for (int v = 0; v < 3; v++)
          Mesh_AddVertex(meshworld, verts[v]);
        
        base += 0x20;
      }
    }
  }
  
  // Render text to texture (root scene only)
  if (this->root_scene == this) {
    Font *font = Font_RenderToTextureComplex(meshworld);
    scene->font_texture = font;  // +0x44C
    Mesh_SaveAndFree(meshworld);
    free(meshworld);
  }
}