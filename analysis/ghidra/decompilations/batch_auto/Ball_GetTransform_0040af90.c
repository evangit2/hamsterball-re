/*
 * Function: Ball_GetTransform
 * Address: 0x0040af90
 * Signature: void __thiscall Ball_GetTransform(void *this,undefined4 *param_1)
 *
 * Patterns: ball. Calls: Ball_GetTransform. Offsets: 4, Lines: 16
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Ball_GetTransform(void *this,undefined4 *param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  
  uVar1 = *(undefined4 *)((int)this + 0x70);
  uVar2 = *(undefined4 *)((int)this + 0xc0);
  uVar3 = *(undefined4 *)((int)this + 0xc4);
  param_1[1] = *(undefined4 *)((int)this + 0x6c);
  *param_1 = &PTR_LAB_004cf584;
  param_1[2] = uVar1;
  param_1[3] = uVar2;
  param_1[4] = uVar3;
  return;
}
