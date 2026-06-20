/*
 * Function: Scene_RenderReflectiveObjects7
 * Address: 0x00411380
 * Signature: Scene_RenderReflectiveObjects7(...)
 *
 * Patterns: vtable dispatch, SEH frame, rendering, scene, level. Calls: Scene_RenderReflectiveObjects7, Level_RenderDynamicObjects, Graphics_SetProjection, RenderContext_Init, Matrix4_Identity. Offsets: 8, Lines: 76
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Scene_RenderReflectiveObjects7(void *this,void *param_1)

{
  float fVar1;
  float fVar2;
  void *this_00;
  float *pfVar3;
  int iVar4;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  float local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  float local_3c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined1 local_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  this_00 = param_1;
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c98c8;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  Level_RenderDynamicObjects((int)this);
  fVar1 = *(float *)((int)param_1 + 0x790) + (float)_DAT_004cf308;
  *(float *)((int)param_1 + 0x790) = fVar1;
  fVar2 = *(float *)((int)param_1 + 0x794) + (float)_DAT_004cf308;
  *(float *)((int)param_1 + 0x794) = fVar2;
  Graphics_SetProjection(param_1,fVar1,fVar2);
  iVar4 = (int)this + 0x436c;
  pfVar3 = (float *)((int)this + 0x642c);
  param_1 = (void *)0x8;
  do {
    if (*pfVar3 != _DAT_004cf368) {
      RenderContext_Init(&local_5c);
      local_4c = *pfVar3;
      local_4 = 0;
      local_3c = *pfVar3;
      local_58 = 0x3f800000;
      local_54 = 0x3f800000;
      local_50 = 0x3f800000;
      local_10 = local_4c != (float)_DAT_004cf3c8;
      local_48 = 0x3f800000;
      local_44 = 0x3f800000;
      local_40 = 0x3f800000;
      local_28 = 0x3f800000;
      local_24 = 0x3f800000;
      local_20 = 0x3f800000;
      local_1c = 0x3f800000;
      *(undefined4 **)((int)this_00 + 0x7c0) = &local_5c;
      (**(code **)(**(int **)((int)this + 0x8ac) + 0x50))(iVar4);
      local_4 = 0xffffffff;
      Matrix4_Identity(&local_5c);
    }
    pfVar3 = pfVar3 + 1;
    iVar4 = iVar4 + 0x418;
    param_1 = (void *)((int)param_1 + -1);
  } while (param_1 != (void *)0x0);
  *(undefined4 *)((int)this_00 + 0x7c0) = 0;
  fVar1 = *(float *)((int)this_00 + 0x790) - (float)_DAT_004cf308;
  *(float *)((int)this_00 + 0x790) = fVar1;
  fVar2 = *(float *)((int)this_00 + 0x794) - (float)_DAT_004cf308;
  *(float *)((int)this_00 + 0x794) = fVar2;
  Graphics_SetProjection(this_00,fVar1,fVar2);
  ExceptionList = pvStack_c;
  return;
}
