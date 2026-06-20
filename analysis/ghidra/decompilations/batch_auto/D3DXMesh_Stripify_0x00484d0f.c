
int __thiscall D3DXMesh_Stripify(void *this,void *param_1,int param_2,int param_3)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  int local_14;
  uint local_10;
  int local_c;
  int local_8;
  
  local_14 = 0;
  local_10 = 0;
  if (*(int *)((int)this + 0x54) != 0) {
    local_c = 0;
    do {
      iVar1 = *(int *)((int)this + 0x50) + local_c;
      uVar4 = *(uint *)(iVar1 + 4);
      local_8 = 0;
      local_14 = MeshData_InitVertexAdj(param_1,uVar4,*(int *)(iVar1 + 8),'\0');
      if (local_14 < 0) {
        return local_14;
      }
      while (uVar3 = MeshData_FindAttributeOffset((int)param_1), uVar3 != 0xffffffff) {
        while( true ) {
          MeshData_RemoveFace(param_1,uVar3);
          uVar2 = MeshData_FindBestNextEdge(param_1,uVar3);
          iVar1 = local_8 + uVar4;
          local_8 = local_8 + 1;
          *(int *)(param_2 + uVar3 * 4) = iVar1;
          if (uVar2 == 3) break;
          uVar3 = *(uint *)(*(int *)((int)param_1 + 4) + (uVar2 + uVar3 * 3) * 4);
        }
      }
      local_10 = local_10 + 1;
      local_c = local_c + 0x14;
    } while (local_10 < *(uint *)((int)this + 0x54));
  }
  uVar4 = 0;
  if (*(int *)((int)this + 0x58) != 0) {
    do {
      *(undefined4 *)(param_3 + uVar4 * 4) = 0xffffffff;
      uVar4 = uVar4 + 1;
    } while (uVar4 < *(uint *)((int)this + 0x58));
  }
  uVar4 = 0;
  if (*(int *)((int)this + 0x58) != 0) {
    do {
      iVar1 = *(int *)(param_2 + uVar4 * 4);
      if (iVar1 != -1) {
        *(uint *)(param_3 + iVar1 * 4) = uVar4;
      }
      uVar4 = uVar4 + 1;
    } while (uVar4 < *(uint *)((int)this + 0x58));
  }
  return local_14;
}

