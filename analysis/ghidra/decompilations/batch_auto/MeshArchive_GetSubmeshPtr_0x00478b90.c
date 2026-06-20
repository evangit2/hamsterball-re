
int __cdecl MeshArchive_GetSubmeshPtr(int param_1,int param_2)

{
  if (*(int *)(param_1 + 4) != 0) {
    if (param_2 < 0) {
      if (*(int *)(param_1 + 0x40) < 3) goto LAB_00478bbd;
      param_2 = *(int *)(param_1 + 0x48);
    }
    else if (*(int *)(param_1 + 0x1c) <= param_2) {
      return 0;
    }
    return param_2 * 0x20 + *(int *)(param_1 + 0x30);
  }
LAB_00478bbd:
  return *(int *)(param_1 + 0x30);
}

