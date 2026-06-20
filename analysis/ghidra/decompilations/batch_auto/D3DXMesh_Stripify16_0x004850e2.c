
int __thiscall D3DXMesh_Stripify16(void *this,void *param_1,int param_2,int param_3)

{
  ushort uVar1;
  int iVar2;
  void *this_00;
  int iVar3;
  uint uVar4;
  int iVar5;
  int local_18;
  uint local_10;
  int local_c;
  short local_8;
  
  this_00 = param_1;
  local_18 = 0;
  local_10 = 0;
  if (*(int *)((int)this + 0x54) != 0) {
    local_c = 0;
    do {
      iVar5 = 0;
      iVar3 = *(int *)((int)this + 0x50) + local_c;
      iVar2 = *(int *)(iVar3 + 4);
      local_18 = MeshData_InitShortVertexAdj
                           (this_00,(ushort)iVar2,(ushort)*(undefined4 *)(iVar3 + 8),'\0');
      if (local_18 < 0) {
        return local_18;
      }
      while (param_1 = (void *)MeshEdge_GetFirstVertex((int)this_00), (short)param_1 != -1) {
        while( true ) {
          MeshData_RemoveShortFace(this_00,(ushort)param_1);
          uVar4 = MeshData_FindBestNextShortEdge(this_00,(ushort)param_1);
          iVar3 = iVar5 + iVar2;
          iVar5 = iVar5 + 1;
          local_8 = (short)uVar4;
          *(int *)(param_2 + ((uint)param_1 & 0xffff) * 4) = iVar3;
          if (local_8 == 3) break;
          param_1 = (void *)(uint)*(ushort *)
                                   (*(int *)((int)this_00 + 4) +
                                   (((uint)param_1 & 0xffff) * 3 + (uVar4 & 0xffff)) * 2);
        }
      }
      local_10 = local_10 + 1;
      local_c = local_c + 0x14;
    } while (local_10 < *(uint *)((int)this + 0x54));
  }
  uVar4 = 0;
  if (*(int *)((int)this + 0x58) != 0) {
    do {
      *(undefined4 *)(param_3 + uVar4 * 4) = 0xffff;
      uVar4 = uVar4 + 1;
    } while (uVar4 < *(uint *)((int)this + 0x58));
  }
  uVar4 = 0;
  if (*(int *)((int)this + 0x58) != 0) {
    do {
      uVar1 = *(ushort *)(param_2 + uVar4 * 4);
      if (uVar1 != 0xffff) {
        *(uint *)(param_3 + (uint)uVar1 * 4) = uVar4;
      }
      uVar4 = uVar4 + 1;
    } while (uVar4 < *(uint *)((int)this + 0x58));
  }
  return local_18;
}

