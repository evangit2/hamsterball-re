
void __thiscall
Font_DrawGlyph3D(void *this,byte *param_1,float param_2,float param_3,float param_4,float param_5,
                float param_6,float param_7,undefined4 param_8,undefined4 param_9,
                undefined4 param_10,undefined4 param_11,undefined4 param_12,undefined4 param_13,
                undefined4 param_14,undefined4 param_15)

{
  byte *pbVar1;
  byte bVar2;
  int iVar3;
  float fVar4;
  float fVar5;
  float unaff_EBP;
  float unaff_ESI;
  float in_stack_ffffff44;
  float in_stack_ffffff48;
  float in_stack_ffffff4c;
  float in_stack_ffffff50;
  float in_stack_ffffff54;
  float in_stack_ffffff58;
  float in_stack_ffffff5c;
  float in_stack_ffffff60;
  float in_stack_ffffff64;
  float in_stack_ffffff68;
  float in_stack_ffffff6c;
  float fVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  float fStack_64;
  float local_54;
  float local_50;
  float local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  float fStack_3c;
  float fStack_38;
  float fStack_34;
  float fStack_30;
  float fStack_2c;
  float fStack_28;
  undefined1 auStack_24 [4];
  float fStack_20;
  float fStack_1c;
  void *pvStack_c;
  undefined1 *puStack_8;
  float local_4;
  
  puStack_8 = &LAB_004ccd68;
  pvStack_c = ExceptionList;
  local_4 = 0.0;
  ExceptionList = &pvStack_c;
  if (param_1 != (byte *)0x0) {
    local_50 = param_6;
    local_54 = param_5;
    local_4c = param_7;
    local_44 = param_9;
    local_48 = param_8;
    local_40 = param_10;
    ExceptionList = &pvStack_c;
    Vec3_NormalizeAndScale(&local_54,1.0);
    Vec3_NormalizeAndScale(&local_48,1.0);
    iVar3 = *(int *)((int)this + 4);
    (**(code **)(**(int **)(iVar3 + 0x154) + 200))();
    *(undefined4 *)(iVar3 + 0x708) = 1;
    fVar6 = 6.37952e-39;
    uVar7 = param_12;
    uVar8 = param_13;
    uVar9 = param_14;
    Graphics_SetViewportClip(*(void **)((int)this + 4),param_12,param_13,param_14,param_15);
    bVar2 = *param_1;
    while (bVar2 != 0) {
      bVar2 = *param_1;
      if (*(char *)((int)this + (uint)bVar2 * 0x14 + 0x42c) != '\0') {
        if (bVar2 == 0x20) {
          iVar3 = *(int *)((int)this + 0x420);
        }
        else {
          fVar4 = (float)*(int *)((int)this + (uint)bVar2 * 0x14 + 0x430) *
                  *(float *)((int)this + 0x428);
          fStack_3c = unaff_ESI + param_2 * fVar4;
          fStack_38 = fVar4 * param_3 + unaff_EBP;
          fStack_34 = fVar4 * param_4 + fStack_64;
          fVar5 = (float)*(int *)((int)this + 0x424) * *(float *)((int)this + 0x428);
          fStack_30 = local_54 * fVar5 + unaff_ESI;
          local_4 = local_50 * fVar5;
          fStack_2c = local_4 + unaff_EBP;
          fStack_28 = fVar5 * local_4c + fStack_64;
          fStack_20 = local_4 + fVar4 * param_3 + unaff_EBP;
          fStack_1c = fVar5 * local_4c + fVar4 * param_4 + fStack_64;
          if ((undefined4 *)&stack0xffffff74 != &param_8) {
            uVar7 = param_8;
            uVar8 = param_9;
            uVar9 = param_10;
          }
          if (&stack0xffffff68 != auStack_24) {
            in_stack_ffffff68 = local_54 * fVar5 + param_2 * fVar4 + unaff_ESI;
            in_stack_ffffff6c = fStack_20;
            fVar6 = fStack_1c;
          }
          if ((float *)&stack0xffffff5c != &fStack_30) {
            in_stack_ffffff5c = fStack_30;
            in_stack_ffffff60 = fStack_2c;
            in_stack_ffffff64 = fStack_28;
          }
          if ((float *)&stack0xffffff50 != &fStack_3c) {
            in_stack_ffffff50 = fStack_3c;
            in_stack_ffffff54 = fStack_38;
            in_stack_ffffff58 = fStack_34;
          }
          puStack_8 = &stack0xffffff44;
          if ((undefined4 *)&stack0xffffff44 != &local_48) {
            in_stack_ffffff44 = unaff_ESI;
            in_stack_ffffff48 = unaff_EBP;
            in_stack_ffffff4c = fStack_64;
          }
          Ball_RenderWithMaterial
                    (*(void **)((int)this + (uint)*param_1 * 0x14 + 0x43c),in_stack_ffffff44,
                     in_stack_ffffff48,in_stack_ffffff4c,in_stack_ffffff50,in_stack_ffffff54,
                     in_stack_ffffff58,in_stack_ffffff5c,in_stack_ffffff60,in_stack_ffffff64,
                     in_stack_ffffff68,in_stack_ffffff6c,fVar6,uVar7,uVar8,uVar9,(undefined4 *)0x0);
          iVar3 = *(int *)((int)this + (uint)*param_1 * 0x14 + 0x430);
        }
        fVar4 = (float)iVar3 * *(float *)((int)this + 0x428);
        unaff_ESI = fVar4 * param_2 + unaff_ESI;
        unaff_EBP = fVar4 * param_3 + unaff_EBP;
        fStack_64 = param_4 * fVar4 + fStack_64;
      }
      pbVar1 = param_1 + 1;
      param_1 = param_1 + 1;
      bVar2 = *pbVar1;
    }
    iVar3 = *(int *)((int)this + 4);
    if (*(char *)(iVar3 + 0x7d2) == '\0') {
      (**(code **)(**(int **)(iVar3 + 0x154) + 200))();
    }
    else {
      (**(code **)(**(int **)(iVar3 + 0x154) + 200))();
    }
    *(undefined4 *)(iVar3 + 0x708) = 3;
    *(undefined1 *)(*(int *)((int)this + 4) + 0x7a8) = 0;
  }
  local_4 = -NAN;
  Matrix_Identity(&param_14);
  ExceptionList = pvStack_c;
  return;
}

