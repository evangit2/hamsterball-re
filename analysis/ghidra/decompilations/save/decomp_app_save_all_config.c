/*
 * App_SaveAllConfig (0x4284C0)
 * Writes all game settings to Windows registry
 * Called on game exit and when settings change
 */

void App_SaveAllConfig(int app_ptr) {
    // Write display settings (resolution, quality, etc.)
    App_WriteDisplaySettings(app_ptr);
    
    // Open registry key
    RegKey_Open(*(int*)(app_ptr + 0x54));
    
    // Mouse sensitivity (DWORD)
    RegKey_WriteDWORD(app_ptr + 0x54, "MouseSensitivity", *(int*)(app_ptr + 0x84C));
    
    // Tournament mirror mode (BOOL)
    RegKey_WriteBool(app_ptr + 0x54, "MirrorTournament", *(char*)(app_ptr + 0x850));
    
    // Race unlock flags (12 races)
    RegKey_WriteBool(app_ptr + 0x54, "DizzyRace", *(char*)(app_ptr + 0x851));
    RegKey_WriteBool(app_ptr + 0x54, "TowerRace", *(char*)(app_ptr + 0x852));
    RegKey_WriteBool(app_ptr + 0x54, "UpRace", *(char*)(app_ptr + 0x853));
    RegKey_WriteBool(app_ptr + 0x54, "ExpertRace", *(char*)(app_ptr + 0x854));
    RegKey_WriteBool(app_ptr + 0x54, "OddRace", *(char*)(app_ptr + 0x855));
    RegKey_WriteBool(app_ptr + 0x54, "ToobRace", *(char*)(app_ptr + 0x856));
    RegKey_WriteBool(app_ptr + 0x54, "WobblyRace", *(char*)(app_ptr + 0x857));
    RegKey_WriteBool(app_ptr + 0x54, "SkyRace", *(char*)(app_ptr + 0x858));
    RegKey_WriteBool(app_ptr + 0x54, "MasterRace", *(char*)(app_ptr + 0x859));
    
    // Right button pause toggle
    RegKey_WriteBool(app_ptr + 0x54, "RightButtonPause", *(char*)(app_ptr + 0x238));
    
    // Arena unlock flags (12 arenas)
    RegKey_WriteBool(app_ptr + 0x54, "DizzyArena", *(char*)(app_ptr + 0x85A));
    RegKey_WriteBool(app_ptr + 0x54, "TowerArena", *(char*)(app_ptr + 0x85B));
    RegKey_WriteBool(app_ptr + 0x54, "UpArena", *(char*)(app_ptr + 0x85C));
    RegKey_WriteBool(app_ptr + 0x54, "ExpertArena", *(char*)(app_ptr + 0x85D));
    RegKey_WriteBool(app_ptr + 0x54, "OddArena", *(char*)(app_ptr + 0x85E));
    RegKey_WriteBool(app_ptr + 0x54, "ToobArena", *(char*)(app_ptr + 0x85F));
    RegKey_WriteBool(app_ptr + 0x54, "WobblyArena", *(char*)(app_ptr + 0x860));
    RegKey_WriteBool(app_ptr + 0x54, "SkyArena", *(char*)(app_ptr + 0x861));
    RegKey_WriteBool(app_ptr + 0x54, "MasterArena", *(char*)(app_ptr + 0x862));
    
    // Additional race unlocks (Neon, Glass, Impossible)
    RegKey_WriteBool(app_ptr + 0x54, "NeonRace", *(char*)(app_ptr + 0x863));
    RegKey_WriteBool(app_ptr + 0x54, "GlassRace", *(char*)(app_ptr + 0x864));
    RegKey_WriteBool(app_ptr + 0x54, "ImpossibleRace", *(char*)(app_ptr + 0x865));
    
    // Additional arena unlocks (Neon, Glass, Impossible)
    RegKey_WriteBool(app_ptr + 0x54, "NeonArena", *(char*)(app_ptr + 0x866));
    RegKey_WriteBool(app_ptr + 0x54, "GlassArena", *(char*)(app_ptr + 0x867));
    RegKey_WriteBool(app_ptr + 0x54, "ImpossibleArena", *(char*)(app_ptr + 0x868));
    
    // Best times (80-byte binary blob)
    Registry_SetValue(app_ptr + 0x54, "BestTime", app_ptr + 0x86C, 0x50);
    
    // Medal status (80-byte binary blob)
    Registry_SetValue(app_ptr + 0x54, "Medals", app_ptr + 0x8BC, 0x50);
    
    // 2P controller mappings
    RegKey_WriteDword(app_ptr + 0x54, "2PController1", *(int*)(app_ptr + 0xB28));
    RegKey_WriteDword(app_ptr + 0x54, "2PController2", *(int*)(app_ptr + 0xB2C));
    RegKey_WriteDword(app_ptr + 0x54, "2PController3", *(int*)(app_ptr + 0xB30));
    RegKey_WriteDword(app_ptr + 0x54, "2PController4", *(int*)(app_ptr + 0xB34));
    
    // Close registry
    RegKey_Close(*(int*)(app_ptr + 0x54));
}