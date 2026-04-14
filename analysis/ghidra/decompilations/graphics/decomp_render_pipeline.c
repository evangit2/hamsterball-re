// Scene_RenderAllObjects (0x45E0E0) — Complete decompilation
// 3-pass rendering: opaque → alpha → shadow, with D3D8 state management

void __thiscall Scene_RenderAllObjects(void *this, char do_sorting, int render_mode)
{
    IDirect3DDevice8 *device;
    int obj, sub_obj, idx;
    
    Graphics_BeginFrame(this->graphics, this->render_ctx);
    
    if (do_sorting) {
        // Clear sort lists
        AthenaList_Free(&this->shadow_list);   // +0x488
        AthenaList_Free(&this->alpha_list);     // +0x8A0
        AthenaList_Free(&this->deferred_list);  // +0xCB8
        
        // Init ball render state if ball rendering enabled
        if (this->scene_parent->flags & 0x434) {
            Ball_InitRenderState(this->graphics);
        }
    }
    
    if (this->skip_render == 0) {  // +0x430
        
        // === CLASSIFICATION PASS ===
        // Iterate all scene objects, classify by flags
        if (this->scene_parent->flags & 0x434) {
            // With ball rendering (normal game)
            for each object in mesh_list (at this->mesh_data):
                if (obj->skip_flag) continue;        // +0x863
                if (obj->deferred_flag) {            // +0x862
                    AthenaList_Append(&this->deferred_list, obj);
                    obj->material_ptr = obj->index * 0x50 + mesh_base;
                    continue;
                }
                if (obj->shadow_flag) {              // +0x85F
                    AthenaList_Append(&this->shadow_list, obj);
                    obj->material_ptr = obj->index * 0x50 + mesh_base;
                    continue;
                }
                if (obj->alpha_flag) {               // +0x860
                    AthenaList_Append(&this->alpha_list, obj);
                    obj->material_ptr = obj->index * 0x50 + mesh_base;
                    continue;
                }
                
                // === OPAQUE PASS (inline) ===
                obj->frame_id = this->graphics->frame_counter; // +0x83C = +0x7C4
                Graphics_ApplyMaterialAndDraw(this->graphics, obj->material_ptr);
                
                // Sub-material handling
                Scene *scene = this->scene_parent;
                if (scene->ball_cutscene == 0) {  // +0x10C4
                    // Normal: SetStreamSource per sub-material
                    for each sub in obj->sub_materials:
                        device = this->graphics->device; // +0x154
                        device->SetStreamSource(5, sub->stride, sub->offset); // vtable[0x118]
                        this->graphics->draw_counter++; // +0x7CC
                } else if (this->graphics->value_1C != 0.0) {
                    // Cutscene: set vertex declaration
                    scene->vertex_decl->Set(obj->vertex_buf_idx); // +0x44C vtable[0xC]
                    this->graphics->draw_counter++;
                }
            }
        } else {
            // Without ball rendering (simple walk mode)
            for each object in this->object_list:
                if (render_mode == 2) {
                    obj->vtable[0x48](0, 2); // render with mode 2
                } else {
                    int result = obj->vtable[0x24](); // test visibility
                    if (result != 0) {
                        obj->vtable[0x48](0, 2); // render
                    }
                }
        }
        
        // === ALPHA PASS ===
        if (do_sorting && AthenaList_GetSize(&this->alpha_list) > 0) {
            // Disable Z-write
            device = this->graphics->device;
            if (this->graphics->zwrite_cached != 0) { // +0x70C
                device->SetRenderState(D3DRS_ZENABLE, 0); // vtable[200](0xE, 0)
                this->graphics->zwrite_cached = 0;
                this->graphics->render_counter++; // +0x7C8
            }
            
            for each obj in alpha_list:
                obj->frame_id = this->graphics->frame_counter;
                Graphics_ApplyMaterialAndDraw(this->graphics, obj->material_ptr);
                // Sub-materials same as opaque pass
            
            // Re-enable Z-write
            if (this->graphics->zwrite_cached != 1) {
                device->SetRenderState(D3DRS_ZENABLE, 1);
                this->graphics->zwrite_cached = 1;
                this->graphics->render_counter++;
            }
        }
        
        // === SHADOW PASS ===
        if (do_sorting && AthenaList_GetSize(&this->shadow_list) > 0) {
            // Add depth bias to projection
            Graphics *gfx = this->graphics;
            float bias = DAT_004CF308;
            float new_proj_x = gfx->proj_offset_x + bias; // +0x790
            float new_proj_y = gfx->proj_offset_y + bias; // +0x794
            gfx->proj_offset_x = new_proj_x;
            gfx->proj_offset_y = new_proj_y;
            Graphics_SetProjection(gfx, new_proj_x, new_proj_y);
            
            // Disable Z-write
            if (gfx->zwrite_cached != 0) {
                device->SetRenderState(D3DRS_ZENABLE, 0);
                gfx->zwrite_cached = 0;
                gfx->render_counter++;
            }
            
            // Enable stencil
            device->SetRenderState(D3DRS_STENCILENABLE, 1); // vtable[200](0x37, 1)
            
            for each obj in shadow_list:
                // Depth bias objects need Z-write toggle
                if (obj->depth_bias_flag) { // +0x861
                    if (gfx->zwrite_cached != 1) {
                        device->SetRenderState(D3DRS_ZENABLE, 1);
                        gfx->zwrite_cached = 1;
                        gfx->render_counter++;
                    }
                }
                
                obj->frame_id = gfx->frame_counter;
                Graphics_ApplyMaterialAndDraw(gfx, obj->material_ptr);
                // Sub-materials...
                
                // Restore Z-write for depth bias objects
                if (obj->depth_bias_flag) {
                    if (gfx->zwrite_cached != 0) {
                        device->SetRenderState(D3DRS_ZENABLE, 0);
                        gfx->zwrite_cached = 0;
                        gfx->render_counter++;
                    }
                }
            
            // Disable stencil
            device->SetRenderState(D3DRS_STENCILENABLE, 3); // vtable[200](0x37, 3)
            
            // Ensure Z-write is on
            if (gfx->zwrite_cached != 1) {
                device->SetRenderState(D3DRS_ZENABLE, 1);
                gfx->zwrite_cached = 1;
                gfx->render_counter++;
            }
            
            // Remove depth bias from projection
            gfx->proj_offset_x -= bias;
            gfx->proj_offset_y -= bias;
            Graphics_SetProjection(gfx, gfx->proj_offset_x, gfx->proj_offset_y);
        }
    }
    
    // Post-render child cleanup
    for each child in this->scene_parent->child_list:
        child->vtable[0x48](1, 1); // cleanup callback
}