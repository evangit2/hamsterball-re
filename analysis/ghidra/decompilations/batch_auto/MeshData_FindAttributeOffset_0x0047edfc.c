
int __fastcall MeshData_FindAttributeOffset(int param_1)

{
  int *piVar1;
  uint uVar2;
  
  uVar2 = 0;
  piVar1 = (int *)(param_1 + 8);
  do {
    if (*piVar1 != -1) {
      return *(int *)(param_1 + 8 + uVar2 * 4) + *(int *)(param_1 + 0x1c);
    }
    uVar2 = uVar2 + 1;
    piVar1 = piVar1 + 1;
  } while (uVar2 < 4);
  return -1;
}

