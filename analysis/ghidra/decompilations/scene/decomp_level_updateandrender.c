// Level_UpdateAndRender @ 0x004B600
// 6-phase render pipeline for level geometry and objects
//
// Phase 1: Build visible_list from ball_lists (primary + secondary)
//   - Clear visible_list (this+0x3a48)
//   - Append all objects from primary ball list (this+0x29D4)
//   - Append all objects from secondary ball list (this+0x3204)
//
// Phase 2: Opaque render pass
//   - Set alpha test OFF: ball->vtable[0x1C](0xE, 0)  [D3DRS_ALPHATESTENABLE=FALSE]
//   - For each object in primary list: call vtable[0x1C]() (RenderOpaque)
//   - For each object in secondary list: call vtable[0x1C]() (RenderOpaque)
//
// Phase 3: Alpha render pass  
//   - Set alpha test ON: ball->vtable[0x1C](0xE, 1)  [D3DRS_ALPHATESTENABLE=TRUE]
//   - For each object in primary list: call vtable[0x1C]() (RenderAlpha)
//   - For each object in secondary list: call vtable[0x1C]() (RenderAlpha)
//
// Phase 4: Waypoint arrow render
//   - If race is active (scene+0x878 flags set) and not game-over
//   - Set next waypoint on WaypointList (scene+0x910)
//   - Set arrow timer to 0.45 (0x3EE66666)
//   - Call waypoint arrow vtable[0x08]() (Render)
//
// Phase 5: Visible list render (all objects sorted by render order)
//   - Iterate visible_list calling vtable[0x08]() (RenderPostAlpha)
//
// Phase 6: Ball shadow render
//   - If ripple list (scene+0x8AC+0xCB8) has entries
//   - For each ball in both lists: Ball_RenderShadow()

void __thiscall Level_UpdateAndRender(void *this, int ball)
{
  // Phase 1: Build visible list from both ball object lists
  AthenaList_Free(this->visible_list);  // +0x3A48
  // ... iterate primary list (0x29D4), append to visible
  // ... iterate secondary list (0x3204), append to visible
  
  // Phase 2: Opaque pass
  if (ball->alpha_state != 0) {  // +0x70C
    ball->vtable[0x1C](ball, 0xE, 0);  // SetAlphaTest(FALSE)
    ball->alpha_state = 0;
    ball->render_counter++;  // +0x7C8
  }
  // Render opaque for primary list objects: vtable[0x1C]()
  // Render opaque for secondary list objects: vtable[0x1C]()
  
  // Phase 3: Alpha pass
  if (ball->alpha_state != 1) {
    ball->vtable[0x1C](ball, 0xE, 1);  // SetAlphaTest(TRUE)  
    ball->alpha_state = 1;
    ball->render_counter++;
  }
  // Render alpha for primary list objects
  // Render alpha for secondary list objects
  
  // Phase 4: Waypoint arrow
  if (scene->race_active && !scene->game_over && scene->waypoint_list) {
    WaypointList_SetNextWaypoint(scene->waypoint_list, scene->waypoint_arrow);
    scene->waypoint_arrow->timer = 0.45f;  // 0x3EE66666
    scene->waypoint_arrow->vtable[0x08]();  // Render
  }
  
  // Phase 5: Visible list post-alpha render
  // iterate visible_list: call vtable[0x08]()
  
  // Phase 6: Ball shadows (only if ripples exist)
  if (AthenaList_GetSize(scene->ripple_list) != 0) {
    // For each ball in both lists: Ball_RenderShadow()
  }
}