
void __thiscall
Ball_RenderWithMaterial
          (void *this,undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
          undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,
          undefined4 param_9,undefined4 param_10,undefined4 param_11,undefined4 param_12,
          undefined4 param_13,undefined4 param_14,undefined4 param_15,undefined4 *param_16)

{
  int *piVar1;
  undefined4 local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  undefined4 local_4;
  
  local_80 = param_1;
  local_7c = param_2;
  local_70 = param_14;
  local_64 = *(undefined4 *)((int)this + 0x70);
  local_78 = param_3;
  local_74 = param_13;
  local_68 = *(undefined4 *)((int)this + 0x6c);
  local_6c = param_15;
  local_58 = param_6;
  local_60 = param_4;
  local_5c = param_5;
  local_4c = param_15;
  local_54 = param_13;
  local_48 = *(undefined4 *)((int)this + 0x88);
  local_50 = param_14;
  local_44 = *(undefined4 *)((int)this + 0x8c);
  local_40 = param_7;
  local_34 = param_13;
  local_28 = *(undefined4 *)((int)this + 0xa4);
  local_3c = param_8;
  local_38 = param_9;
  local_30 = param_14;
  local_24 = *(undefined4 *)((int)this + 0xa8);
  local_2c = param_15;
  local_1c = param_11;
  local_20 = param_10;
  local_10 = param_14;
  local_4 = *(undefined4 *)((int)this + 0xc4);
  local_18 = param_12;
  local_14 = param_13;
  local_8 = *(undefined4 *)((int)this + 0xc0);
  local_c = param_15;
  Ball_InitRenderState(*(void **)((int)this + 4));
  if (param_16 == (undefined4 *)0x0) {
    param_16 = (undefined4 *)((int)this + 8);
  }
  else {
    param_16[0x12] = *(undefined4 *)((int)this + 0x50);
  }
  Graphics_ApplyMaterialAndDraw(*(void **)((int)this + 4),param_16);
  Graphics_BeginFrame(*(void **)((int)this + 4),0);
  piVar1 = *(int **)(*(int *)((int)this + 4) + 0x154);
  (**(code **)(*piVar1 + 0x120))(piVar1,5,2,&local_80,0x20);
  return;
}

