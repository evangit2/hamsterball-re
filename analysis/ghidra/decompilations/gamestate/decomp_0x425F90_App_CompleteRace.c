// Function: App_CompleteRace
// Address: 0x00425F90
// Calling Convention: __fastcall (param_1 = App object)
// Xrefs: Called from Graphics_SetupLights, MusicPlayer_Render, LoadingScreenGadget_Render (×2),
//        TourneyResultsScreen_Render, DifficultyMenu_Render, Scene_Render3DObjects
//
// PURPOSE: Finalizes a completed race. Called from multiple render contexts to ensure
// race completion is processed regardless of which screen is active.
//
// APP OBJECT FIELDS:
//   param_1+0x154 → Graphics/D3D device pointer (vtable at its [0])
//   param_1+0x704 → race completion pending flag (1 = race just finished, needs finalization)
//   param_1+0x7C8 → race completion counter (total races completed)
//
// ALGORITHM:
// 1. Check if race completion is pending (param_1+0x704 != 0)
// 2. If so:
//    a. Increment race completion counter (param_1+0x7C8++)
//    b. Call graphics vtable+0xFC with (0, 0xD, 1) — likely D3DRS_ZENABLE or render state change
//    c. Call graphics vtable+0xFC with (0, 0xE, 1) — second render state change
//    d. Clear the pending flag (param_1+0x704 = 0)
//
// The vtable offset 0xFC (252 bytes = index 63) on the graphics object likely corresponds
// to a SetRenderState wrapper. Parameters 0xD (13) and 0xE (14) map to D3DRS values:
//   D3DRS_ZENABLE = 7, D3DRS_ZWRITEENABLE = 14, D3DRS_ALPHATESTENABLE = 15
// So 0xD=13=D3DRS_ZENABLE and 0xE=14=D3DRS_ZWRITEENABLE are being enabled (value=1)
// to restore proper depth testing after the race overlay rendering.

void __fastcall App_CompleteRace(int param_1)
{
  // ... decompiled body ...
}
