/*
 * Ball_GetInputForce decompilation (0x46EC30)
 * Converts input device state to 2D force vector for ball physics
 * Mode 1=keyboard, 2=mouse, 4-7=joystick
 */

void Ball_GetInputForce(void *this, float *output) {
    float force_x, force_y, scale;
    int key_state_base;
    
    force_y = 0.0f;        // _DAT_004cf368 = 0.0f
    force_x = 0.0f;        // default no force
    
    switch (*(int*)(this + 8)) {  // input mode
    
    case 1:  // KEYBOARD
        key_state_base = *(int*)(*(int*)(this + 4) + 0x434);
        // Check UP key (DIK code at +0x514)
        if (*(byte*)(*(int*)(key_state_base + 0x514) + 0xC + key_state_base) & 0x80)
            force_y = -0.5f;  // _DAT_004d0250 = half speed forward
        // Check DOWN key (DIK code at +0x518)
        if (*(byte*)(*(int*)(key_state_base + 0x518) + 0xC + key_state_base) & 0x80)
            force_y = 1.0f;   // _DAT_004cf310 = full speed backward
        // Check LEFT key (DIK code at +0x50C)
        if (*(byte*)(*(int*)(key_state_base + 0x50C) + 0xC + key_state_base) & 0x80)
            force_x = -1.0f;
        // Check RIGHT key (DIK code at +0x510)
        if (*(byte*)(*(int*)(key_state_base + 0x510) + 0xC + key_state_base) & 0x80)
            force_x = 1.0f;
        break;
    
    case 2:  // MOUSE
        GetCursorPos(&cursor_pos);
        {
            int *gfx = *(int*)(*(int*)(this + 4) + 4);
            int center_x = *(int*)(gfx + 0x15C) / 2;
            int center_y = *(int*)(gfx + 0x160) / 2;
            force_x = (float)(cursor_pos.x - center_x);
            force_y = (float)(cursor_pos.y - center_y);
            // Re-center cursor if mouse capture is active
            if (*(char*)(gfx + 0x15A) != 0) {
                SetCursorPos(center_x, center_y);
            }
        }
        break;
    
    case 4: case 5: case 6: case 7:  // JOYSTICK (4 devices)
        {
            int *joy = *(int*)(this + 0x10);
            if (joy != NULL) {
                // Axis values at joy+0x10C (X) and joy+0x110 (Y), divided by 100
                Vec2 pos;
                pos.x = (float)(*(int*)(joy + 0x10C) / 100);
                pos.y = (float)(*(int*)(joy + 0x110) / 100);
                // Normalize then scale to unit circle
                Vec3_NormalizeAndScale(&pos, 1.0f);
                force_x = pos.x;
                force_y = pos.y;
            }
        }
        break;
    }
    
    // Apply per-ball speed scale (this+0xC)
    scale = *(float*)(this + 0xC);
    output[0] = scale * force_x;
    output[1] = scale * force_y;
}

/*
 * InputDevice_SetType (0x6DFC0)
 * Sets input device type: 1=keyboard, 2=mouse, 4=joy1, 5=joy2
 * Called from App_Initialize_Full (0x429530) steps 16-22
 */

/*
 * App_CreateInputDevice (0x46C050)
 * Allocates 0x14-byte InputDevice, calls NetworkConnection_Ctor
 * Maps to InputDevice struct:
 *   +0x00: vtable
 *   +0x04: type (1=kb, 2=mouse, 4+5=joystick)
 *   +0x08: DIDEVICEINSTANCE data
 *   +0x0C-0x14: DirectInput device state
 */

/*
 * InputDevice_PollAndRelease (0x46EBD0)
 * Polls DirectInput device (GetDeviceState/GetDeviceData)
 * Releases any buffered input events
 */

/*
 * Scene_HandleInput (0x4692F0)
 * Dispatches input to scene objects:
 * 1. Clears active_input_object (Scene+0x864)
 * 2. Iterates Scene+0x858 (gadget list) via AthenaList
 * 3. For each gadget that is input_active (+0x16 != 0):
 *    - Call gadget->vtable[1](msg1, msg2) - input handler
 *    - If gadget captures input (+0x14 != 0): set as active
 *    - If gadget is player1 (+0x19 != 0): use App+0x1E4 channel
 *    - If gadget is player2 (+0x15 != 0): use App+0x1E8 channel
 * 4. Call App->vtable[0x7C](sound_id) - play input sound
 */