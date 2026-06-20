/*
 * Function: Level_SetObjectTransform
 * Address: 0x0040b9c0
 * Signature: Level_SetObjectTransform(...)
 *
 * Patterns: level. Calls: Level_SetObjectTransform. Offsets: 17, Lines: 48
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Level_SetObjectTransform(void *this,int param_1)

{
  float fVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  float fVar5;
  
  fVar1 = *(float *)(param_1 + 0x10);
  uVar2 = *(undefined4 *)(param_1 + 0xc);
  uVar3 = *(undefined4 *)(param_1 + 8);
  *(undefined4 *)((int)this + 4) = *(undefined4 *)(param_1 + 4);
  *(undefined4 *)((int)this + 8) = uVar3;
  *(undefined4 *)((int)this + 0xc) = uVar2;
  fVar5 = (float)_DAT_004cf3c8;
  *(float *)((int)this + 0x10) = fVar1;
  *(bool *)((int)this + 0x4c) = fVar1 != fVar5;
  uVar2 = *(undefined4 *)(param_1 + 0x20);
  uVar3 = *(undefined4 *)(param_1 + 0x1c);
  uVar4 = *(undefined4 *)(param_1 + 0x18);
  *(undefined4 *)((int)this + 0x14) = *(undefined4 *)(param_1 + 0x14);
  *(undefined4 *)((int)this + 0x18) = uVar4;
  *(undefined4 *)((int)this + 0x1c) = uVar3;
  *(undefined4 *)((int)this + 0x20) = uVar2;
  *(bool *)((int)this + 0x4c) = fVar1 != (float)_DAT_004cf3c8;
  uVar2 = *(undefined4 *)(param_1 + 0x40);
  uVar3 = *(undefined4 *)(param_1 + 0x3c);
  uVar4 = *(undefined4 *)(param_1 + 0x38);
  *(undefined4 *)((int)this + 0x34) = *(undefined4 *)(param_1 + 0x34);
  *(undefined4 *)((int)this + 0x38) = uVar4;
  *(undefined4 *)((int)this + 0x3c) = uVar3;
  *(undefined4 *)((int)this + 0x40) = uVar2;
  *(bool *)((int)this + 0x4c) = fVar1 != (float)_DAT_004cf3c8;
  uVar2 = *(undefined4 *)(param_1 + 0x30);
  uVar3 = *(undefined4 *)(param_1 + 0x2c);
  uVar4 = *(undefined4 *)(param_1 + 0x28);
  *(undefined4 *)((int)this + 0x24) = *(undefined4 *)(param_1 + 0x24);
  *(undefined4 *)((int)this + 0x28) = uVar4;
  *(undefined4 *)((int)this + 0x2c) = uVar3;
  *(undefined4 *)((int)this + 0x30) = uVar2;
  *(bool *)((int)this + 0x4c) = fVar1 != (float)_DAT_004cf3c8;
  *(undefined1 *)((int)this + 0x4c) = *(undefined1 *)(param_1 + 0x4c);
  *(undefined4 *)((int)this + 0x48) = *(undefined4 *)(param_1 + 0x48);
  *(undefined1 *)((int)this + 0x4d) = *(undefined1 *)(param_1 + 0x4d);
  return;
}
