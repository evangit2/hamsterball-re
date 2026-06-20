/*
 * Function: Ball_InitBattleMode
 * Address: 0x00456cd0
 * Signature: void __fastcall ...(void *param_1)
 * Parameters:
 *   param_1: Ball* this
 *
 * Description:
 * Initializes ball for arena/battle mode. Sets: +0xC60=3 (mode), +0xC68=0.555f (friction), +0xC6C=1.0f, +0xC70=1000.0f (max_speed), +0xC78=25.0f (radius), +0xC7C=1 (use_gravity). Gravity vector at +0xC8C=(0,-1,0). Sets +0x14=1 (active). Calls Ball_SetSpeed with computed value. 2 refs.
 *
 * Struct offsets:
 *   +0xC60 (mode=3), +0xC68 (friction=0.555), +0xC6C (1.0), +0xC70 (max_speed=1000), +0xC78 (radius=25.0), +0xC7C (use_gravity=1), +0xC8C (grav_dir=(0,-1,0)), +0x14 (active=1)
 *
 * Cross-references:
 *   2 refs: CollisionMesh_ctor, vtable DATA
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
