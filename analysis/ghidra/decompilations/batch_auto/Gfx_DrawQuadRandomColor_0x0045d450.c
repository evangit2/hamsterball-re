
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
Gfx_DrawQuadRandomColor
          (void *this,undefined4 param_1,float param_2,float param_3,float param_4,float param_5,
          undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9,
          undefined4 param_10)

{
  int *piVar1;
  bool bVar2;
  float10 fVar3;
  float10 fVar4;
  float10 fVar5;
  float10 fVar6;
  float in_stack_0000003c;
  float local_7c;
  float local_78;
  undefined4 local_70;
  uint local_6c;
  undefined4 local_68;
  undefined4 local_64;
  float local_60;
  float local_5c;
  undefined4 local_54;
  uint local_50;
  undefined4 local_4c;
  undefined4 local_48;
  float local_44;
  float local_40;
  undefined4 local_38;
  uint local_34;
  undefined4 local_30;
  undefined4 local_2c;
  float local_28;
  float local_24;
  void *pvStack_20;
  undefined4 local_1c;
  uint local_18;
  undefined4 local_14;
  undefined4 local_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cce73;
  pvStack_c = ExceptionList;
  local_4 = 2;
  ExceptionList = &pvStack_c;
  fVar3 = Gfx_TransformY(*(void **)((int)this + 4),param_5 + param_3);
  fVar4 = Gfx_TransformX(*(void **)((int)this + 4),param_4 + param_2);
  fVar5 = Gfx_TransformY(*(void **)((int)this + 4),param_3);
  fVar6 = Gfx_TransformX(*(void **)((int)this + 4),param_2);
  local_7c = (float)fVar6;
  local_44 = (float)fVar6;
  local_68 = param_7;
  local_64 = param_8;
  local_4c = param_9;
  local_48 = param_8;
  local_30 = param_7;
  local_2c = param_10;
  local_4 = CONCAT31(local_4._1_3_,3);
  local_70 = 0x3f800000;
  local_54 = 0x3f800000;
  local_38 = 0x3f800000;
  local_1c = 0x3f800000;
  local_14 = param_9;
  local_10 = param_10;
  local_78 = (float)fVar5;
  local_60 = (float)fVar4;
  local_5c = (float)fVar5;
  local_40 = (float)fVar3;
  local_28 = (float)fVar4;
  local_24 = (float)fVar3;
  local_6c = Color_RandomRGBA();
  bVar2 = in_stack_0000003c != _DAT_004cf310;
  *(undefined1 *)((int)this + 0x54) = 0;
  if (bVar2) {
    *(undefined1 *)((int)this + 0x54) = 1;
  }
  local_50 = local_6c;
  local_34 = local_6c;
  local_18 = local_6c;
  Graphics_SetRenderMode(*(void **)((int)this + 4),*(char *)((int)this + 0xd1));
  Graphics_ApplyMaterialAndDraw(*(void **)((int)this + 4),(undefined4 *)((int)this + 8));
  piVar1 = *(int **)(*(int *)((int)this + 4) + 0x154);
  (**(code **)(*piVar1 + 0x120))(piVar1,5,2,&local_7c,0x1c);
  local_18 = 0xffffffff;
  Matrix_Identity(&param_6);
  ExceptionList = pvStack_20;
  return;
}

