
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
Sprite_RenderQuad(void *this,float param_1,float param_2,float param_3,float param_4,
                 undefined4 *param_5)

{
  int iVar1;
  int *piVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float local_80;
  float local_7c;
  float local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  undefined4 local_64;
  float local_60;
  float local_5c;
  float local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  float local_40;
  float local_3c;
  float local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  float local_20;
  float local_1c;
  float local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  undefined4 local_4;
  
  iVar1 = *(int *)((int)*(void **)((int)this + 4) + 0x744);
  fVar3 = (float)*(int *)((int)this + 200) * param_4 * _DAT_004cf3f0;
  fVar4 = *(float *)(iVar1 + 0x5c) * fVar3;
  fVar5 = fVar3 * *(float *)(iVar1 + 0x60);
  fVar3 = fVar3 * *(float *)(iVar1 + 100);
  local_74 = 0x3f800000;
  local_70 = 0x3f800000;
  local_6c = 0xbf800000;
  local_68 = 0x3f800000;
  local_64 = 0x3f800000;
  local_54 = 0x3f800000;
  local_50 = 0x3f800000;
  local_4c = 0xbf800000;
  local_48 = 0;
  local_18 = (float)*(int *)((int)this + 0xcc) * param_4 * _DAT_004cf3f0;
  local_44 = 0x3f800000;
  local_20 = *(float *)(iVar1 + 0x68) * local_18;
  local_1c = *(float *)(iVar1 + 0x6c) * local_18;
  local_18 = local_18 * *(float *)(iVar1 + 0x70);
  local_80 = (param_1 + fVar4) - local_20;
  local_7c = (fVar5 + param_2) - local_1c;
  local_78 = (fVar3 + param_3) - local_18;
  local_60 = (param_1 - local_20) - fVar4;
  local_5c = (param_2 - local_1c) - fVar5;
  local_58 = (param_3 - local_18) - fVar3;
  local_40 = local_20 + fVar4 + param_1;
  local_34 = 0x3f800000;
  local_30 = 0x3f800000;
  local_2c = 0xbf800000;
  local_28 = 0x3f800000;
  local_24 = 0;
  local_14 = 0x3f800000;
  local_10 = 0x3f800000;
  local_c = 0xbf800000;
  local_3c = local_1c + fVar5 + param_2;
  local_8 = 0;
  local_4 = 0;
  local_38 = local_18 + fVar3 + param_3;
  local_20 = (param_1 - fVar4) + local_20;
  local_1c = (param_2 - fVar5) + local_1c;
  local_18 = (param_3 - fVar3) + local_18;
  Ball_InitRenderState(*(void **)((int)this + 4));
  if (param_5 == (undefined4 *)0x0) {
    param_5 = (undefined4 *)((int)this + 8);
  }
  else {
    param_5[0x12] = *(undefined4 *)((int)this + 0x50);
  }
  Graphics_ApplyMaterialAndDraw(*(void **)((int)this + 4),param_5);
  Graphics_BeginFrame(*(void **)((int)this + 4),0);
  piVar2 = *(int **)(*(int *)((int)this + 4) + 0x154);
  (**(code **)(*piVar2 + 0x120))(piVar2,5,2,&local_80,0x20);
  return;
}

