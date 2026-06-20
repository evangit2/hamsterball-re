
undefined4 __cdecl Mesh_ReorderFacesByAdjacency(int param_1,int param_2,uint param_3)

{
  undefined4 *_Memory;
  uint uVar1;
  uint uVar2;
  undefined4 *puVar3;
  undefined4 local_8;
  
  local_8 = 0;
  _Memory = operator_new(param_3);
  if (_Memory == (undefined4 *)0x0) {
    local_8 = 0x8007000e;
  }
  else {
    puVar3 = _Memory;
    for (uVar1 = param_3 >> 2; uVar1 != 0; uVar1 = uVar1 - 1) {
      *puVar3 = 0;
      puVar3 = puVar3 + 1;
    }
    for (uVar1 = param_3 & 3; uVar1 != 0; uVar1 = uVar1 - 1) {
      *(undefined1 *)puVar3 = 0;
      puVar3 = (undefined4 *)((int)puVar3 + 1);
    }
    uVar1 = 0;
    if (param_3 != 0) {
      do {
        if ((*(char *)(uVar1 + (int)_Memory) == '\0') &&
           (uVar2 = *(uint *)(param_1 + uVar1 * 4), uVar2 != 0xffffffff)) {
          while (uVar2 != uVar1) {
            Mesh_SwapFaceEntries(uVar1,uVar2,param_2);
            Mesh_RemapFaceIndices(uVar2,param_1,param_2);
            *(undefined1 *)(uVar2 + (int)_Memory) = 1;
            uVar2 = *(uint *)(param_1 + uVar2 * 4);
            if ((uVar2 == 0xffffffff) || (*(char *)(uVar2 + (int)_Memory) != '\0'))
            goto LAB_0047d934;
          }
          Mesh_RemapFaceIndices(uVar1,param_1,param_2);
        }
LAB_0047d934:
        uVar1 = uVar1 + 1;
      } while (uVar1 < param_3);
    }
  }
  _free(_Memory);
  return local_8;
}

