/*
 * Function: GameObject2_dtor
 * Address: 0x00443F50
 * Signature: void __fastcall GameObject2_dtor(undefined4 *param_1)
 * Parameters:
 *   param_1: GameObject2* this — the game object to destroy
 *
 * Description:
 * Destructor for GameObject2 (base class for dialogs). Sets vtable to
 * PTR_GameObject2_DeletingDtor (0x4D62C0). Calls BaseObject_Init on two
 * sub-objects at +0x226 and +0x21F, then calls SceneObject_dtor.
 *
 * Cross-references:
 *   Called from Unwind handler (0x4CC0B3) — UNCONDITIONAL_CALL
 *   Called from GameObject2_DeletingDtor (0x446843) — UNCONDITIONAL_CALL
 *   Called from QuitToDesktop_ScalarDtor (0x444876) — UNCONDITIONAL_CALL
 *   Called from QuitAbortDialog_ScalarDtor (0x445406) — UNCONDITIONAL_CALL
 *   Called from TourneyContinueDialog_ScalarDtor — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
