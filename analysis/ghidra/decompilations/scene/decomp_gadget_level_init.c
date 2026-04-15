/*
 * Gadget_ctor (0x4690F0) - Base gadget/scene object constructor
 * Size: 0x870 bytes (2160)
 * 
 * Key offsets:
 *   +0x00  vtable (SceneObject_ScalarDtor at 0x4D9170)
 *   +0x04  field_4 (0)
 *   +0x08  field_8 (0)
 *   +0x0C  field_C (0)
 *   +0x10  field_10 (0)
 *   +0x14  App* (param_1 - scene pointer)
 *   +0x18  vtable_18 (PTR_LAB_004cf584)
 *   +0x1C-0x28  fields (0)
 *   +0x2C  byte_2C (0) - input active flag
 *   +0x30  field_30 (0)
 *   +0x34  AthenaList child_gadgets (offset 0x34, size 0x418)
 *   +0x44C  AthenaList sprite_slots (offset 0x44C, size variable)
 *   +0x864  field_864 (0)
 *   +0x868  name ("Generic Gadget")
 *   +0x86C  byte flags[4] (0)
 *   +0x870  App+0x1DC (sound channel)
 *   +0x874  byte_874 (0)
 */

undefined4* Gadget_ctor(void *this, int param_1) {
    // Zero-initialize key fields
    *(int*)(this + 0x10) = 0;
    *(int*)(this + 0x0C) = 0;
    *(int*)(this + 0x08) = 0;
    *(int*)(this + 0x04) = 0;
    
    // Set vtable
    *(void**)(this + 0x00) = &SceneObject_ScalarDtor;  // 0x4D9170
    *(void**)(this + 0x18) = &LAB_004CF584;
    
    // Zero more fields
    *(int*)(this + 0x28) = 0;
    *(int*)(this + 0x24) = 0;
    *(int*)(this + 0x20) = 0;
    *(int*)(this + 0x1C) = 0;
    
    // Initialize two AthenaLists
    AthenaList_Init(this + 0x34, 0);   // child gadgets
    AthenaList_Init(this + 0x44C, 0);  // sprite slots
    
    // Store scene pointer
    *(int*)(this + 0x14) = param_1;
    
    // Clear input flags
    *(byte*)(this + 0x2C) = 0;   // input_active
    *(int*)(this + 0x30) = 0;
    *(byte*)(this + 0x86C) = 0;
    *(byte*)(this + 0x86D) = 0;
    *(byte*)(this + 0x86E) = 0;
    *(byte*)(this + 0x86F) = 0;
    *(byte*)(this + 0x874) = 0;
    *(int*)(this + 0x864) = 0;
    
    // Set default name
    *(char**)(this + 0x868) = "Generic Gadget";
    
    // Store sound channel from App
    *(int*)(this + 0x870) = *(int*)(param_1 + 0x1DC);
    
    return this;
}

/*
 * Level_InitScene (0x40B090) - Initialize scene for level start
 * 
 * Sequence:
 * 1. Create SoundChannel (0x80 bytes) - audio for level
 * 2. Set sound volume (0xC2480000 = -50.0f dB)
 * 3. Set sound position (camera distance)
 * 4. Register sound with graphics engine
 * 5. If level mesh loaded (Scene+0x8AC):
 *    - Call level mesh reset (vtable[4])
 *    - Set random ambient colors via Gfx_SetVTable22/8B
 *    - Reset object slots
 *    - Clear path rail
 *    - Find "CAMERALOCUS" object (camera focus target)
 *    - If found: set camera path to follow CAMERALOCUS
 * 6. Copy App state to scene (cursor visibility, camera defaults)
 * 7. Level_SelectCameraProfile - set camera defaults per level type
 * 8. If 2-player split screen: apply alternate camera settings
 * 9. Set cull mode (backface culling on)
 * 10. Set projection: FOV=20.0, near=far*2*0.8+0.9, far=far+far*0.01+z_epsilon
 * 11. Set camera start position
 * 12. If camera override (Scene+0x299C != -2.0): snap camera
 * 13. Play level music:
 *     - If intro flag: play level_music + "_No_Intro" at speed 4.0
 *     - Normal: play level_music at speed 2.0
 *     - Music channel at App+0x53C
 * 
 * Key scene offsets:
 *   Scene+0x87C  SoundChannel*
 *   Scene+0x878  App*
 *   Scene+0x8AC  LevelMesh*
 *   Scene+0x3F1C path_active (Camera path rail)
 *   Scene+0x3F20 path_ptr
 *   Scene+0x3F24 path_t (parametric position)
 *   Scene+0x870  input state
 *   Scene+0x29C0 orbit_distance
 *   Scene+0x299C camera_start_override (-2.0f = no override)
 *   Scene+0x3F28 snap_flag
 *   Scene+0x3F38 snap_flag2
 *   Scene+0x3F34 snap_position
 *   Scene+0x4344 level_music_path
 */