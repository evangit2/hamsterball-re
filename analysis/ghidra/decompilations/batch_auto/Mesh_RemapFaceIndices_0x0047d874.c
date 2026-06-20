
void __cdecl Mesh_RemapFaceIndices(int param_1,int param_2,int param_3)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  
  iVar1 = param_3 + param_1 * 0xc;
  uVar3 = 0;
  do {
    iVar2 = *(int *)(iVar1 + uVar3 * 4);
    if (iVar2 == -1) {
      *(undefined4 *)(iVar1 + uVar3 * 4) = 0xffffffff;
    }
    else {
      *(undefined4 *)(iVar1 + uVar3 * 4) = *(undefined4 *)(param_2 + iVar2 * 4);
    }
    uVar3 = uVar3 + 1;
  } while (uVar3 < 3);
  return;
}

