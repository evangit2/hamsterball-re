
void __fastcall MeshWorld_OptimizeAll(int param_1)

{
  int iVar1;
  int iVar2;
  float *pfVar3;
  char local_974 [256];
  undefined4 local_874 [538];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_c = ExceptionList;
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd80b;
  ExceptionList = &local_c;
  *(undefined4 *)(param_1 + 0x34) = 0;
  if (*(int *)(param_1 + 0x30) < 1) {
    iVar1 = 0;
  }
  else {
    iVar1 = **(int **)(param_1 + 0x438);
    *(undefined4 *)(param_1 + 0x34) = 1;
  }
  while (iVar1 != 0) {
    iVar2 = AthenaList_GetSize(iVar1 + 0xc);
    D3DXSkinMesh_Ctor(local_874,iVar2);
    local_4 = 0;
    *(undefined4 *)(iVar1 + 0x14) = 0;
    if (*(int *)(iVar1 + 0x10) < 1) {
      pfVar3 = (float *)0x0;
    }
    else {
      pfVar3 = (float *)**(undefined4 **)(iVar1 + 0x418);
      *(undefined4 *)(iVar1 + 0x14) = 1;
    }
    while (pfVar3 != (float *)0x0) {
      Mesh_AddElement(local_874,pfVar3);
      Mesh_AddElement(local_874,pfVar3 + 8);
      Mesh_AddElement(local_874,pfVar3 + 0x10);
      iVar2 = *(int *)(iVar1 + 0x14);
      if (*(int *)(iVar1 + 0x10) <= iVar2) break;
      pfVar3 = *(float **)(*(int *)(iVar1 + 0x418) + iVar2 * 4);
      *(int *)(iVar1 + 0x14) = iVar2 + 1;
    }
    *(undefined4 *)(iVar1 + 0x14) = 0;
    if (*(int *)(iVar1 + 0x10) < 1) {
      pfVar3 = (float *)0x0;
    }
    else {
      pfVar3 = (float *)**(undefined4 **)(iVar1 + 0x418);
      *(undefined4 *)(iVar1 + 0x14) = 1;
    }
    while (pfVar3 != (float *)0x0) {
      Mesh_AddFace(local_874,pfVar3);
      iVar2 = *(int *)(iVar1 + 0x14);
      if (*(int *)(iVar1 + 0x10) <= iVar2) break;
      pfVar3 = *(float **)(*(int *)(iVar1 + 0x418) + iVar2 * 4);
      *(int *)(iVar1 + 0x14) = iVar2 + 1;
    }
    iVar2 = D3DXSkinMesh_GenerateStrips(local_874,1);
    if (iVar2 == -1) {
      AthenaList_GetSize(iVar1 + 0xc);
      AthenaString_SprintfToBuffer(local_974,(byte *)"Optimize requested, but failed!\n\n%d faces");
      MessageBoxA((HWND)0x0,local_974,"Mesh",0);
      *(undefined1 *)(iVar1 + 0x85c) = 0;
      D3DXSkinMesh_DestroyAllObjects((int)local_874);
    }
    else {
      *(undefined1 *)(iVar1 + 0x85c) = 1;
      D3DXSkinMesh_RebuildList(local_874,(void *)(iVar1 + 0x424));
    }
    local_4 = 0xffffffff;
    Mesh_Dtor(local_874);
    iVar2 = *(int *)(param_1 + 0x34);
    if (*(int *)(param_1 + 0x30) <= iVar2) break;
    iVar1 = *(int *)(*(int *)(param_1 + 0x438) + iVar2 * 4);
    *(int *)(param_1 + 0x34) = iVar2 + 1;
  }
  MeshWorld_BuildVertexBuffer(param_1);
  ExceptionList = local_c;
  return;
}

