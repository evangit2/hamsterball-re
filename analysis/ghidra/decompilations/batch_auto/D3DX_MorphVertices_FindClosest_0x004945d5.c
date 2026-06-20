
void __thiscall D3DX_MorphVertices_FindClosest(void *this,uint param_1,float param_2,uint param_3)

{
  float fVar1;
  void *extraout_ECX;
  undefined1 *puVar2;
  uint uVar3;
  float *pfVar4;
  float *pfVar5;
  
  if (*(int *)((int)this + 0x1048) != 0) {
    param_3 = Vertex_Transform(this,param_3);
    this = extraout_ECX;
  }
  puVar2 = (undefined1 *)
           (*(int *)((int)this + 0x1050) * param_1 + *(int *)((int)this + 0x1054) * (int)param_2 +
           *(int *)((int)this + 0x18));
  param_1 = 0;
  if (*(int *)((int)this + 0x1058) != 0) {
    pfVar4 = (float *)(param_3 + 8);
    do {
      uVar3 = 0;
      param_2 = 3.4028235e+38;
      param_3 = 0;
      pfVar5 = (float *)((int)this + 0x38);
      do {
        fVar1 = (pfVar4[-2] - pfVar5[-2]) * (pfVar4[-2] - pfVar5[-2]) +
                (pfVar4[-1] - pfVar5[-1]) * (pfVar4[-1] - pfVar5[-1]) +
                (*pfVar4 - *pfVar5) * (*pfVar4 - *pfVar5) +
                (pfVar4[1] - pfVar5[1]) * (pfVar4[1] - pfVar5[1]);
        if (fVar1 < param_2) {
          uVar3 = param_3;
          param_2 = fVar1;
        }
        param_3 = param_3 + 1;
        pfVar5 = pfVar5 + 4;
      } while (param_3 < 0x100);
      *puVar2 = (char)uVar3;
      puVar2 = puVar2 + 1;
      param_1 = param_1 + 1;
      pfVar4 = pfVar4 + 4;
    } while (param_1 < *(uint *)((int)this + 0x1058));
  }
  return;
}

