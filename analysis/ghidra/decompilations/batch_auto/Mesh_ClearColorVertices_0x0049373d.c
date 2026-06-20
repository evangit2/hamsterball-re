
void __thiscall Mesh_ClearColorVertices(void *this,uint param_1)

{
  uint uVar1;
  int iVar2;
  float *pfVar3;
  
  uVar1 = *(int *)((int)this + 0x1058) * 0x10 + param_1;
  if (param_1 < uVar1) {
    iVar2 = ((uVar1 - param_1) - 1 >> 4) + 1;
    pfVar3 = (float *)(param_1 + 8);
    do {
      if ((((pfVar3[-2] == *(float *)((int)this + 0x1c)) &&
           (pfVar3[-1] == *(float *)((int)this + 0x20))) &&
          (*pfVar3 == *(float *)((int)this + 0x24))) && (pfVar3[1] == *(float *)((int)this + 0x28)))
      {
        pfVar3[1] = 0.0;
        *pfVar3 = 0.0;
        pfVar3[-1] = 0.0;
        pfVar3[-2] = 0.0;
      }
      pfVar3 = pfVar3 + 4;
      iVar2 = iVar2 + -1;
    } while (iVar2 != 0);
  }
  return;
}

