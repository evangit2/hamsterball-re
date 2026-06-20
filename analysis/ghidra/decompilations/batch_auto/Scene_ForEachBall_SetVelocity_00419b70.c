/*
 * Function: Scene_ForEachBall_SetVelocity
 * Address: 0x00419b70
 * Signature: void __thiscall Scene_ForEachBall_SetVelocity(void *this,float param_1,float param_2,float param_3)
 *
 * Patterns: ball, scene. Calls: Scene_ForEachBall_SetVelocity, AthenaList_NextIndex, Ball_CheckProximity. Offsets: 4, Lines: 36
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Scene_ForEachBall_SetVelocity(void *this,float param_1,float param_2,float param_3)

{
  int iVar1;
  int iVar2;
  void *this_00;
  float in_stack_ffffffec;
  float in_stack_fffffff0;
  float fVar3;
  
  fVar3 = 6.025089e-39;
  iVar2 = AthenaList_NextIndex((int)this + 0x29d4);
  *(undefined4 *)((int)this + iVar2 * 4 + 0x29dc) = 0;
  if (*(int *)((int)this + 0x29d8) < 1) {
    this_00 = (void *)0x0;
  }
  else {
    this_00 = (void *)**(undefined4 **)((int)this + 0x2de0);
    *(undefined4 *)((int)this + iVar2 * 4 + 0x29dc) = 1;
  }
  while( true ) {
    if (this_00 == (void *)0x0) {
      return;
    }
    if ((float *)&stack0xffffffec != &param_1) {
      in_stack_ffffffec = param_1;
      in_stack_fffffff0 = param_2;
      fVar3 = param_3;
    }
    Ball_CheckProximity(this_00,in_stack_ffffffec,in_stack_fffffff0,fVar3);
    iVar1 = *(int *)((int)this + iVar2 * 4 + 0x29dc);
    if (*(int *)((int)this + 0x29d8) <= iVar1) break;
    this_00 = *(void **)(*(int *)((int)this + 0x2de0) + iVar1 * 4);
    *(int *)((int)this + iVar2 * 4 + 0x29dc) = iVar1 + 1;
  }
  return;
}
