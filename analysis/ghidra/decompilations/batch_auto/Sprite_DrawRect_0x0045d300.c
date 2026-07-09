
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Sprite_DrawRect(void *this,float param_1,float param_2)

{
  int *piVar1;
  bool bVar2;
  uint uVar3;
  float10 fVar4;
  float10 fVar5;
  float10 fVar6;
  float10 fVar7;
  float in_stack_0000001c;
  void *pvVar8;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cce38;
  pvStack_c = ExceptionList;
  local_4 = 0;
  ExceptionList = &pvStack_c;
  fVar4 = Gfx_TransformX(*(void **)((int)this + 4),param_1);
  fVar5 = Gfx_TransformY(*(void **)((int)this + 4),param_2);
  fVar6 = Gfx_TransformX(*(void **)((int)this + 4),(float)*(int *)((int)this + 200) + param_1);
  fVar7 = Gfx_TransformY(*(void **)((int)this + 4),(float)*(int *)((int)this + 0xcc) + param_2);
  uVar3 = Color_RandomRGBA();
  bVar2 = in_stack_0000001c != _DAT_004cf310;
  *(float *)((int)this + 0x58) = (float)fVar4;
  *(float *)((int)this + 0x74) = (float)fVar6;
  *(uint *)((int)this + 0x68) = uVar3;
  *(uint *)((int)this + 0x84) = uVar3;
  *(uint *)((int)this + 0xa0) = uVar3;
  *(uint *)((int)this + 0xbc) = uVar3;
  *(float *)((int)this + 0x5c) = (float)fVar5;
  *(float *)((int)this + 0x78) = (float)fVar5;
  *(float *)((int)this + 0x90) = (float)fVar4;
  *(float *)((int)this + 0x94) = (float)fVar7;
  *(float *)((int)this + 0xac) = (float)fVar6;
  *(float *)((int)this + 0xb0) = (float)fVar7;
  *(undefined1 *)((int)this + 0x54) = 0;
  if (bVar2) {
    *(undefined1 *)((int)this + 0x54) = 1;
  }
  Graphics_SetRenderMode(*(void **)((int)this + 4),*(char *)((int)this + 0xd1));
  Graphics_ApplyMaterialAndDraw(*(void **)((int)this + 4),(undefined4 *)((int)this + 8));
  piVar1 = *(int **)(*(int *)((int)this + 4) + 0x154);
  pvVar8 = (void *)0x1c;
  (**(code **)(*piVar1 + 0x120))(piVar1,5,2,(float *)((int)this + 0x58));
  Matrix_Identity(&puStack_8);
  ExceptionList = pvVar8;
  return;
}

