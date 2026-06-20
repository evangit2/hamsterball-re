
void __thiscall
Sprite_DrawRotatedQuad(void *this,float param_1,float param_2,float param_3,float param_4)

{
  float fVar1;
  float fVar2;
  int *piVar3;
  uint uVar4;
  float *pfVar5;
  float10 fVar6;
  float10 fVar7;
  float10 fVar8;
  float10 fVar9;
  float10 fVar10;
  undefined1 auVar11 [10];
  void *local_20;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cce88;
  pvStack_c = ExceptionList;
  local_4 = 0;
  ExceptionList = &pvStack_c;
  fVar6 = Gfx_TransformY(*(void **)((int)this + 4),param_1 - (float)(*(int *)((int)this + 200) / 2))
  ;
  fVar7 = Gfx_TransformZ(*(void **)((int)this + 4),param_2 - (float)(*(int *)((int)this + 0xcc) / 2)
                        );
  fVar8 = Gfx_TransformY(*(void **)((int)this + 4),(float)(*(int *)((int)this + 200) / 2) + param_1)
  ;
  fVar9 = Gfx_TransformZ(*(void **)((int)this + 4),(float)(*(int *)((int)this + 0xcc) / 2) + param_2
                        );
  fVar10 = Gfx_TransformY(*(void **)((int)this + 4),param_1);
  fVar1 = (float)fVar10;
  fVar10 = Gfx_TransformZ(*(void **)((int)this + 4),param_2);
  fVar2 = (float)fVar10;
  uVar4 = Color_RandomRGBA();
  *(float *)((int)this + 0x58) = (float)fVar8;
  *(float *)((int)this + 0x5c) = (float)fVar7;
  *(float *)((int)this + 0x74) = (float)fVar6;
  *(float *)((int)this + 0x78) = (float)fVar7;
  *(float *)((int)this + 0x90) = (float)fVar8;
  *(float *)((int)this + 0x94) = (float)fVar9;
  *(float *)((int)this + 0xac) = (float)fVar6;
  *(float *)((int)this + 0xb0) = (float)fVar9;
  local_20 = (void *)0x4;
  pfVar5 = (float *)((int)this + 0x5c);
  do {
    fVar6 = Math_Atan2Angle(&PTR_PTR_004f7188,pfVar5[-1],*pfVar5,fVar1,fVar2);
    fVar7 = Vec2_Distance(pfVar5[-1],*pfVar5,fVar1,fVar2);
    auVar11 = Wave_Sin(&PTR_PTR_004f7188,(float)fVar6 - param_3);
    pfVar5[-1] = (float)((float10)auVar11 * (float10)(float)(fVar7 * (float10)param_4) +
                        (float10)fVar1);
    auVar11 = Wave_Cos(&PTR_PTR_004f7188,(float)fVar6 - param_3);
    local_20 = (void *)((int)local_20 + -1);
    *pfVar5 = (float)((float10)auVar11 * (float10)(float)(fVar7 * (float10)param_4) + (float10)fVar2
                     );
    pfVar5 = pfVar5 + 7;
  } while (local_20 != (void *)0x0);
  *(uint *)((int)this + 0x68) = uVar4;
  *(uint *)((int)this + 0x84) = uVar4;
  *(uint *)((int)this + 0xa0) = uVar4;
  *(uint *)((int)this + 0xbc) = uVar4;
  Graphics_SetRenderMode(*(void **)((int)this + 4),*(char *)((int)this + 0xd1));
  Graphics_ApplyMaterialAndDraw(*(void **)((int)this + 4),(undefined4 *)((int)this + 8));
  piVar3 = *(int **)(*(int *)((int)this + 4) + 0x154);
  (**(code **)(*piVar3 + 0x120))(piVar3,5,2,(float *)((int)this + 0x58),0x1c);
  Matrix_Identity((undefined4 *)&stack0x00000000);
  ExceptionList = local_20;
  return;
}

