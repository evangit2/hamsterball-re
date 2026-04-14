// Scene_RenderAllObjects @ 0x0045E0E0
// Master 3-pass render pipeline for all scene objects
//
// Parameters:
//   this: scene render context
//   param_1: rebuild_lists flag (0=just render, 1=rebuild+render)
//   param_2: render mode (0=check visibility, 2=force render)
//
// Three rendering passes (sorted by alpha):
//   Pass 1: OPAQUE - objects with no transparency flags
//     - AlphaBlend OFF, AlphaTest OFF
//     - Graphics_ApplyMaterialAndDraw for each opaque object
//     - Draw texture sub-objects via vtable[0x118] (SetTextureStageState)
//
//   Pass 2: TRANSLUCENT (alpha blended) - objects with alpha flag 0x862=1
//     - Listed at scene+0x8A0
//     - AlphaBlend ON, AlphaTest OFF  
//     - Graphics_ApplyMaterialAndDraw for each translucent object
//     - Uses D3DRS_ALPHABLENDENABLE (0x0E) toggle
//
//   Pass 3: DECAL/REFLECTIVE - objects with flag 0x85F=1 (decals)  
//     - Listed at scene+0x488
//     - Depth bias applied (projection offset +_DAT_004CF308)
//     - D3DRS_STENCILENABLE (0x37) = 1 for stencil-based rendering
//     - Each decal object gets per-object alpha test state
//     - Projection restored after pass
//
// Object classification flags (on SceneObject, offset from object base):
//   +0x863: has_bounding_sphere (skip if set)
//   +0x862: is_translucent (alpha blend needed)
//   +0x85F: is_decal (stencil/decal render)
//   +0x860: is_translucent_alpha_test (sorted into translucent bucket)
//   +0x861: has_per_object_alpha (needs per-object alpha test toggle)
//   +0x83C: render_frame_counter (set to gfx->frame_counter each draw)
//
// MeshWorld rendering (if scene+0x10C4 flag set):
//   - Non-stripped mode: iterate submeshes, call vtable[0x118] (SetTexture)
//   - Stripped mode: vtable[0x14C](0, texID, 0x20) + vtable[0x0C](palette)
//
// Sprite rendering (non-MeshWorld mode, this+0x430=1):
//   - For each sprite: call vtable[0x24]() check, then vtable[0x48](0, mode)
//
// Post-alpha: iterate scene+0x480's object_list calling vtable[0x48](1,1)

void __thiscall Scene_RenderAllObjects(void *this, char rebuild_lists, int render_mode)
{
  Graphics_BeginFrame(this->gfx, this->view_id);
  
  if (rebuild_lists) {
    AthenaList_Free(this->decal_list);      // +0x488
    AthenaList_Free(this->translucent_list); // +0x8A0
    AthenaList_Free(this->shadow_list);     // +0xCB8
    if (scene->is_ball_mode)
      Ball_InitRenderState(this->gfx);
  }
  
  // === CLASSIFY OBJECTS INTO RENDER BUCKETS ===
  if (!this->is_sprite_mode) {
    if (!scene->is_ball_mode) {
      SceneObject_RenderFull(this, 0);
      return;
    }
    
    for each (obj in meshworld_obj_list) {
      if (obj->has_bounding_sphere) continue;  // +0x863
      
      if (obj->is_translucent) {                // +0x862
        AthenaList_Append(scene->shadow_list, obj);
        obj->render_data = mesh_data_at[obj->index * 0x50];
      }
      else if (obj->is_decal) {                // +0x85F
        AthenaList_Append(scene->decal_list, obj);
        obj->render_data = mesh_data_at[obj->index * 0x50];
      }
      else if (obj->is_alpha_test) {           // +0x860
        AthenaList_Append(scene->translucent_list, obj);
        obj->render_data = mesh_data_at[obj->index * 0x50];
      }
      else {
        // OPAQUE - render immediately
        obj->render_frame = gfx->frame_counter;
        Graphics_ApplyMaterialAndDraw(gfx, mesh_data[obj->index * 0x50]);
        // Draw sub-textures
      }
    }
  } else {
    // Sprite mode: iterate sprite list
    for each (sprite in sprite_list) {
      if (render_mode == 2)
        sprite->vtable[0x48](0, 2);
      else if (sprite->vtable[0x24]() != 0)
        sprite->vtable[0x48](0, 2);
    }
  }
  
  // === PASS 2: TRANSLUCENT OBJECTS ===
  if (rebuild_lists && translucent_count > 0) {
    SetAlphaTest(0xE, FALSE);  // Disable alpha test for blending
    for each (obj in translucent_list) {
      obj->render_frame = gfx->frame_counter;
      Graphics_ApplyMaterialAndDraw(gfx, obj->render_data);
      // Draw sub-textures
    }
    SetAlphaTest(0xE, TRUE);   // Re-enable alpha test
  }
  
  // === PASS 3: DECAL OBJECTS ===
  if (rebuild_lists && decal_count > 0) {
    // Offset projection for depth bias
    gfx->proj_near += depth_bias;
    gfx->proj_far += depth_bias;
    Graphics_SetProjection(gfx, gfx->proj_near, gfx->proj_far);
    
    SetAlphaTest(0xE, FALSE);
    SetStencilEnable(0x37, 1);
    
    for each (obj in decal_list) {
      if (obj->has_per_object_alpha && !alpha_test_enabled)
        SetAlphaTest(0xE, TRUE);
      
      obj->render_frame = gfx->frame_counter;
      Graphics_ApplyMaterialAndDraw(gfx, obj->render_data);
      // Draw sub-textures
      
      if (obj->has_per_object_alpha && alpha_test_enabled)
        SetAlphaTest(0xE, FALSE);
    }
    
    SetStencilEnable(0x37, 3);  // Stencil cleanup
    SetAlphaTest(0xE, TRUE);
    
    // Restore projection
    gfx->proj_near -= depth_bias;
    gfx->proj_far -= depth_bias;
    Graphics_SetProjection(gfx, gfx->proj_near, gfx->proj_far);
  }
  
  // Post-alpha objects
  for each (obj in scene->post_alpha_list)
    obj->vtable[0x48](1, 1);  // RenderPostAlpha
}