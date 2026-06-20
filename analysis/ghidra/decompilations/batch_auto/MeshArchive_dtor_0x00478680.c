
undefined4 __cdecl MeshArchive_dtor(int *param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  if (param_1 != (int *)0x0) {
    Vorbis_FreeDecodeState(param_1 + 0x2e);
    Vorbis_FreeDecoder(param_1 + 0x1a);
    StreamWriter_Destroy((undefined4 *)param_1[0x18]);
    if ((param_1[0xc] != 0) && (param_1[7] != 0)) {
      iVar2 = 0;
      if (0 < param_1[7]) {
        iVar1 = 0;
        iVar3 = 0;
        do {
          D3DResourcePool_Release((undefined4 *)(param_1[0xc] + iVar3));
          MeshGroup_dtor((int *)(param_1[0xd] + iVar1));
          iVar2 = iVar2 + 1;
          iVar3 = iVar3 + 0x20;
          iVar1 = iVar1 + 0x10;
        } while (iVar2 < param_1[7]);
      }
      _free((void *)param_1[0xc]);
      _free((void *)param_1[0xd]);
    }
    if ((void *)param_1[9] != (void *)0x0) {
      _free((void *)param_1[9]);
    }
    if ((void *)param_1[0xb] != (void *)0x0) {
      _free((void *)param_1[0xb]);
    }
    if ((void *)param_1[10] != (void *)0x0) {
      _free((void *)param_1[10]);
    }
    if ((void *)param_1[8] != (void *)0x0) {
      _free((void *)param_1[8]);
    }
    OggStream_Destroy((undefined4 *)param_1[6]);
    if (*param_1 != 0) {
      (*(code *)param_1[0x48])(*param_1);
    }
    for (iVar2 = 0x4a; iVar2 != 0; iVar2 = iVar2 + -1) {
      *param_1 = 0;
      param_1 = param_1 + 1;
    }
  }
  return 0;
}

