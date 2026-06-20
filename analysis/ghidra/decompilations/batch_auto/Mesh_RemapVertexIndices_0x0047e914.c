
void __thiscall Mesh_RemapVertexIndices(void *this,int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  
  piVar1 = (int *)(*(int *)((int)this + 0x40) + param_1 * 0xc);
  piVar3 = (int *)(*(int *)((int)this + 0x44) + param_1 * 0xc);
  if (*piVar1 != -1) {
    iVar2 = (int)piVar1 - (int)piVar3;
    iVar4 = 3;
    do {
      *(undefined4 *)(iVar2 + (int)piVar3) =
           *(undefined4 *)(param_2 + *(int *)(iVar2 + (int)piVar3) * 4);
      if ((*(byte *)((int)this + 0xc) & 1) != 0) {
        if (*piVar3 == -1) {
          *piVar3 = -1;
        }
        else {
          *piVar3 = *(int *)(param_3 + *piVar3 * 4);
        }
      }
      piVar3 = piVar3 + 1;
      iVar4 = iVar4 + -1;
    } while (iVar4 != 0);
  }
  return;
}

