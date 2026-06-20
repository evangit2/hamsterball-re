
void __cdecl Mesh_SwapFaceEntries(int param_1,int param_2,int param_3)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  int iVar3;
  int iVar4;
  
  puVar2 = (undefined4 *)(param_3 + param_1 * 0xc);
  iVar3 = (param_3 + param_2 * 0xc) - (int)puVar2;
  iVar4 = 3;
  do {
    uVar1 = *(undefined4 *)(iVar3 + (int)puVar2);
    *(undefined4 *)(iVar3 + (int)puVar2) = *puVar2;
    *puVar2 = uVar1;
    puVar2 = puVar2 + 1;
    iVar4 = iVar4 + -1;
  } while (iVar4 != 0);
  return;
}

