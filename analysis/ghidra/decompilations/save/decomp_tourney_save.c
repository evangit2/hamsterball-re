/*
 * Tourney_SaveTournament (0x446730)
 * Saves tournament progress - deletes old file, writes new one
 */

void Tourney_SaveTournament(int *tourney) {
    // Flush pending writes
    (**(code **)(*tourney + 0x40))();
    
    // Delete old save file
    CRT_remove("DATA\\tournament.sav");
    
    // Write tournament data (DAT_004D48A0 = save header/format string)
    (**(code **)(*(int *)tourney[0x22E] + 0x54))(&DAT_004D48A0);
}

/*
 * LoadOrSaveConfig (0x4279F0)
 * Cleanup + save on exit. Frees all App sub-objects, saves config, shuts down.
 * - Calls vtable[0x243] and vtable[0x244] (music channel cleanup)
 * - Saves to DATA\HS.CFG via vtable[0x8C]+8
 * - Frees ~80+ object pointers at App+0x88..0x172
 * - Clears string arrays at App+0x173..0x212 (0xA0 bytes, 4 strings)
 * - Opens raptisoft.com if App+0x80 flag is 0
 * - Calls App_Shutdown
 */