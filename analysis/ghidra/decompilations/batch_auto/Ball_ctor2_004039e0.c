/*
 * Function: Ball_ctor2
 * Address: 0x004039e0
 * Signature: Ball_ctor2(...)
 *
 * Patterns: allocates, vtable dispatch, SEH frame, collision, rendering, ball, board. Calls: object, Ball_ctor2, UITimer_Ctor, Timer_Init, RenderContext_Init, RumbleBoard_InitTimer, Vec3_Init, AthenaList_Init. Offsets: 112, Lines: 179
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* Ball_ctor2 - constructs Ball object (size 0xC98). Fields: vtable@0, collision_result@8,
   string_timer@C, app_state@10, scene@14, player_index@18, render_callback@1C, UITimer@1C-107,
   timer@108, accumulated_time@150, prev_pos@158, pos@164, vel@170, max_speed@188, speed_scale@18C,
   physics_body@1A4, gravity@1A8, render_ctx@1B8, render_alpha@1C8, render_ctx2@208, color_RGBA@20C,
   uses_alpha@254, boost@260, rumble_timer@264, gravity_scale@278, is_falling@281, radius@284,
   spin_timer@290, speed_modifier@2A8, accel@2B8, checkpoint@2DC, event_flag@2E8, on_ramp@2E9,
   timer_bf@2FC, display_string@C28, teleport@C3C, matrix@C88 */

Ball * __thiscall Ball_ctor2(void *this,int param_1)

{
  float fVar1;
  void *pvVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined4 local_18 [3];
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9348;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *(undefined ***)this = &PTR_GameObject_sub2_dtor_004cf314;
  UITimer_Ctor((int *)((int)this + 0x1c));
  local_4 = 0;
  Timer_Init((undefined4 *)((int)this + 0x108));
  local_4._0_1_ = 1;
  RenderContext_Init((undefined4 *)((int)this + 0x1b8));
  local_4._0_1_ = 2;
  RenderContext_Init((undefined4 *)((int)this + 0x208));
  local_4._0_1_ = 3;
  RumbleBoard_InitTimer((undefined4 *)((int)this + 0x264));
  local_4._0_1_ = 4;
  Vec3_Init((undefined4 *)((int)this + 0x2a8));
  local_4._0_1_ = 5;
  AthenaList_Init((void *)((int)this + 0x32c),0);
  local_4._0_1_ = 6;
  AthenaList_Init((void *)((int)this + 0x810),0);
  local_4._0_1_ = 7;
  *(undefined1 *)((int)this + 0x769) = 0;
  *(int *)((int)this + 0x14) = param_1;
  *(undefined1 *)((int)this + 0x31e) = 0;
  pvVar2 = operator_new(0xcb0);
  local_4._0_1_ = 8;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = CollisionMesh_ctor(pvVar2,this);
  }
  *(void **)((int)this + 0x1a4) = pvVar2;
  local_4 = CONCAT31(local_4._1_3_,7);
  *(undefined4 *)((int)this + 0x18) = 0xffffffff;
  *(undefined4 *)((int)this + 0x748) = 0;
  if ((undefined4 *)((int)pvVar2 + 0xc8c) != local_18) {
    *(undefined4 *)((int)pvVar2 + 0xc8c) = 0;
    *(undefined4 *)((int)pvVar2 + 0xc90) = 0xbf800000;
    *(undefined4 *)((int)pvVar2 + 0xc94) = 0;
  }
  (**(code **)(*(int *)((int)this + 0x1c) + 4))();
  *(undefined4 *)((int)this + 0xf8) = *(undefined4 *)((int)this + 0x748);
  *(undefined4 *)((int)this + 800) = 0;
  *(undefined4 *)((int)this + 0xc50) = 0;
  *(undefined1 *)((int)this + 0xc58) = 0;
  iVar3 = RNG_Rand(&PTR_OBJ_VTABLE,3,'\0');
  *(int *)((int)this + 0x154) = iVar3;
  *(undefined4 *)((int)this + 0xc5c) = 0;
  if ((undefined4 *)((int)this + 0x1a8) != local_18) {
    *(undefined4 *)((int)this + 0x1a8) = 0;
    *(undefined4 *)((int)this + 0x1ac) = 0x3f800000;
    *(undefined4 *)((int)this + 0x1b0) = 0;
  }
  *(undefined4 *)((int)this + 0xc38) = 0xffffffff;
  *(undefined4 *)((int)this + 0x328) = 0xffffffff;
  *(undefined4 *)((int)this + 0x744) = 0;
  *(undefined4 *)((int)this + 0x74c) = 0;
  *(undefined4 *)((int)this + 0x750) = 0;
  *(undefined4 *)((int)this + 0x2a4) = 0x40a00000;
  *(undefined1 *)((int)this + 0xc3c) = 0;
  *(undefined1 *)((int)this + 0xc4c) = 0;
  *(undefined1 *)((int)this + 4) = 0;
  *(undefined1 *)((int)this + 5) = 0;
  *(undefined4 *)((int)this + 0x278) = 0x3dcccccd;
  *(undefined4 *)((int)this + 0x27c) = 0;
  *(undefined4 *)((int)this + 0x288) = 0;
  *(undefined4 *)((int)this + 0x294) = 0;
  *(undefined4 *)((int)this + 0x298) = 0;
  *(undefined1 *)((int)this + 0x2e8) = 0;
  *(undefined1 *)((int)this + 0x324) = 0;
  *(undefined1 *)((int)this + 0x2d5) = 0;
  *(undefined4 *)((int)this + 0x2d8) = 0;
  *(undefined1 *)((int)this + 0x2cc) = 0;
  *(undefined1 *)((int)this + 0x2d4) = 0;
  *(undefined1 *)((int)this + 0x2e9) = 0;
  *(undefined4 *)((int)this + 0x2ec) = 0;
  *(undefined4 *)((int)this + 0x2f0) = 0;
  *(undefined4 *)((int)this + 0x2f4) = 0;
  *(undefined4 *)((int)this + 0x300) = 0;
  if ((undefined4 *)((int)this + 0x2c0) != local_18) {
    *(undefined4 *)((int)this + 0x2c0) = 0;
    *(undefined4 *)((int)this + 0x2c4) = 0;
    *(undefined4 *)((int)this + 0x2c8) = 0;
  }
  *(undefined1 *)((int)this + 0x76a) = 0;
  *(undefined4 *)((int)this + 0x314) = 0;
  *(undefined1 *)((int)this + 0x31c) = 0;
  *(undefined4 *)((int)this + 0x318) = 0;
  *(undefined1 *)((int)this + 0x768) = 1;
  *(undefined4 *)((int)this + 0x764) = 0x3f800000;
  *(undefined4 *)((int)this + 0x10) = *(undefined4 *)(param_1 + 0x878);
  *(undefined4 *)((int)this + 0x164) = 0;
  *(undefined4 *)((int)this + 0x168) = 0;
  *(undefined4 *)((int)this + 0x16c) = 0;
  *(undefined4 *)((int)this + 0x170) = 0;
  *(undefined4 *)((int)this + 0x174) = 0;
  *(undefined4 *)((int)this + 0x178) = 0;
  *(undefined4 *)((int)this + 0x188) = 0x459c4000;
  *(undefined4 *)((int)this + 0x18c) = 0x3f800000;
  *(undefined1 *)((int)this + 0x19c) = 0;
  *(undefined4 *)((int)this + 0x1b4) = 0;
  *(undefined1 *)((int)this + 0x260) = 0;
  *(undefined1 *)((int)this + 0x281) = 1;
  *(undefined1 *)((int)this + 0x280) = 0;
  *(undefined4 *)((int)this + 0x284) = 0x41d80000;
  *(undefined4 *)((int)this + 0x26c) = 0x14;
  *(undefined4 *)((int)this + 600) = 0;
  *(undefined4 *)((int)this + 0xc54) = 0;
  *(undefined4 *)((int)this + 0x1a0) = 0x3f800000;
  *(undefined1 *)((int)this + 0x28c) = 0;
  *(undefined1 *)((int)this + 0x2f9) = 0;
  *(undefined4 *)((int)this + 0x2fc) = 0x3f800000;
  *(undefined4 *)((int)this + 400) = 0xbf800000;
  *(undefined4 *)((int)this + 0x194) = 0xbf800000;
  puVar4 = (undefined4 *)((int)this + 0x7c8);
  for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar4 = 0;
    puVar4 = puVar4 + 1;
  }
  *(undefined4 *)((int)this + 0x808) = 0;
  *(undefined4 *)((int)this + 0x80c) = 0;
  *(undefined4 *)((int)this + 0x778) = 0;
  *(undefined4 *)((int)this + 0x77c) = 0;
  *(undefined4 *)((int)this + 0x780) = 0;
  *(undefined4 *)((int)this + 0x784) = 0;
  puVar4 = (undefined4 *)((int)this + 0x788);
  for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar4 = 0;
    puVar4 = puVar4 + 1;
  }
  *(undefined1 *)((int)this + 0x310) = 1;
  *(undefined4 *)((int)this + 0xc28) = 0;
  *(undefined4 *)((int)this + 0xc) = 0;
  *(undefined1 *)((int)this + 0x14c) = 0;
  *(undefined1 *)((int)this + 0x2f8) = 0;
  *(undefined4 *)((int)this + 0x29c) = 0x3f800000;
  *(undefined4 *)((int)this + 0x1c8) = 0x3f400000;
  *(undefined1 *)((int)this + 0x204) = 1;
  *(undefined4 *)((int)this + 0x1bc) = 0x3e800000;
  *(undefined4 *)((int)this + 0x1c0) = 0x3e800000;
  *(undefined4 *)((int)this + 0x1c4) = 0x3e800000;
  *(undefined4 *)((int)this + 0x20c) = 0x3f800000;
  *(undefined4 *)((int)this + 0x210) = 0x3f800000;
  *(undefined4 *)((int)this + 0x214) = 0x3f800000;
  *(undefined4 *)((int)this + 0x218) = 0x3f800000;
  *(undefined1 *)((int)this + 0x254) = 0;
  *(undefined4 *)((int)this + 0x23c) = 0x3f800000;
  *(undefined4 *)((int)this + 0x240) = 0x3f800000;
  fVar1 = (float)_DAT_004cf3c8;
  *(undefined4 *)((int)this + 0x244) = 0x3f800000;
  *(undefined4 *)((int)this + 0x248) = 0x3f800000;
  *(bool *)((int)this + 0x254) = *(float *)((int)this + 0x218) != fVar1;
  *(undefined4 *)((int)this + 0x754) = 0;
  *(undefined4 *)((int)this + 0x150) = 0;
  *(undefined1 *)((int)this + 0xc2c) = 0;
  ExceptionList = pvStack_c;
  return this;
}
