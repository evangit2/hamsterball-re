
void __thiscall D3DXSkinMesh_RebuildList(void *this,void *param_1)

{
  int iVar1;
  int iVar2;
  
  AthenaList_Free((int)param_1);
  *(undefined4 *)((int)this + 0x3c) = 0;
  if (*(int *)((int)this + 0x38) < 1) {
    iVar2 = 0;
  }
  else {
    iVar2 = **(int **)((int)this + 0x440);
    *(undefined4 *)((int)this + 0x3c) = 1;
  }
  while( true ) {
    if (iVar2 == 0) {
      return;
    }
    AthenaList_Append(param_1,iVar2);
    iVar1 = *(int *)((int)this + 0x3c);
    if (*(int *)((int)this + 0x38) <= iVar1) break;
    iVar2 = *(int *)(*(int *)((int)this + 0x440) + iVar1 * 4);
    *(int *)((int)this + 0x3c) = iVar1 + 1;
  }
  return;
}

