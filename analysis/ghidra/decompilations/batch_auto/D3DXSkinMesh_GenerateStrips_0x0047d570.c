
int __thiscall D3DXSkinMesh_GenerateStrips(void *this,int param_1)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  int local_8;
  int local_4;
  
  piVar1 = operator_new(*(int *)((int)this + 0x18) * 0xc);
  local_8 = 0;
  do {
    iVar3 = *(int *)((int)this + 0x24);
    iVar2 = 0;
    if (0 < iVar3) {
      piVar4 = (int *)(*(int *)((int)this + 0x28) + 0x1c);
      do {
        if (*piVar4 == 0) break;
        iVar2 = iVar2 + 1;
        piVar4 = piVar4 + 9;
      } while (iVar2 < iVar3);
    }
    if (iVar2 == iVar3) {
      return local_8;
    }
    iVar3 = MeshStrip_GenerateNextStrip(this,iVar2,&local_4,piVar1,param_1);
    if (iVar3 == -1) {
      return -1;
    }
    local_8 = local_8 + 1;
    D3DXSkinMesh_CreateStrip
              (this,*piVar1 * 0x424 + *(int *)((int)this + 0x20),
               piVar1[1] * 0x424 + *(int *)((int)this + 0x20));
    iVar3 = local_4;
    iVar2 = 2;
    if (2 < local_4) {
      do {
        AthenaList_Append((void *)((int)this + 0x450),
                          piVar1[iVar2] * 0x424 + *(int *)((int)this + 0x20));
        iVar2 = iVar2 + 1;
        *(int *)(*(int *)((int)this + 0x44c) + 4) = *(int *)(*(int *)((int)this + 0x44c) + 4) + 1;
      } while (iVar2 < iVar3);
    }
    D3DXSkinMesh_CopyStripData((int)this);
  } while( true );
}

