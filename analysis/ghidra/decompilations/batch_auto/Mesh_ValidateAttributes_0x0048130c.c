
undefined4 __fastcall Mesh_ValidateAttributes(int param_1)

{
  int *in_EAX;
  uint uVar1;
  uint uVar2;
  uint local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x54) != 0) {
    in_EAX = *(int **)(param_1 + 0x50);
    do {
      uVar2 = in_EAX[1];
      uVar1 = in_EAX[2] + uVar2;
      if (uVar2 < uVar1) {
        do {
          if (((*(uint *)(param_1 + 0xc) & 4) != 0) &&
             (*(int *)(*(int *)(param_1 + 0x48) + uVar2 * 4) != *in_EAX)) {
            return (uint)in_EAX & 0xffffff00;
          }
          uVar2 = uVar2 + 1;
        } while (uVar2 < uVar1);
      }
      local_8 = local_8 + 1;
      in_EAX = in_EAX + 5;
    } while (local_8 < *(uint *)(param_1 + 0x54));
  }
  return CONCAT31((int3)((uint)in_EAX >> 8),1);
}

