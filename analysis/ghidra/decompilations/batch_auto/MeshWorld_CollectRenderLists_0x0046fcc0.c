
void __thiscall MeshWorld_CollectRenderLists(void *this,void *param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  RenderList_FreeAndClear((int)param_1);
  iVar3 = AthenaList_NextIndex(*(int *)((int)this + 8) + 0x2c);
  iVar1 = *(int *)((int)this + 8);
  *(undefined4 *)(iVar1 + 0x34 + iVar3 * 4) = 0;
  if (*(int *)(iVar1 + 0x30) < 1) {
    iVar4 = 0;
  }
  else {
    iVar4 = **(int **)(iVar1 + 0x438);
    *(undefined4 *)(iVar1 + 0x34 + iVar3 * 4) = 1;
  }
  while( true ) {
    if (iVar4 == 0) {
      return;
    }
    RenderList_AppendCopy
              (param_1,(undefined4 *)
                       (*(int *)(iVar4 + 4) * 0x50 + *(int *)(*(int *)((int)this + 8) + 0x28)));
    iVar1 = *(int *)((int)this + 8);
    iVar2 = *(int *)(iVar1 + 0x34 + iVar3 * 4);
    if (*(int *)(iVar1 + 0x30) <= iVar2) break;
    iVar4 = *(int *)(*(int *)(iVar1 + 0x438) + iVar2 * 4);
    *(int *)(iVar1 + 0x34 + iVar3 * 4) = iVar2 + 1;
  }
  return;
}

