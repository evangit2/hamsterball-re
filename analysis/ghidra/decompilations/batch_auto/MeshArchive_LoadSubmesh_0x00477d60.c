
int __thiscall MeshArchive_LoadSubmesh(void *this,undefined4 *param_1,int *param_2,int *param_3)

{
  undefined4 *in_EAX;
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  longlong lVar4;
  int local_30 [12];
  
  local_30[0] = 0;
  local_30[1] = 0;
  local_30[2] = 0;
  local_30[3] = 0;
  local_30[4] = 0;
  local_30[5] = 0;
  local_30[6] = 0;
  local_30[7] = 0;
  local_30[8] = 0;
  local_30[9] = 0;
  local_30[10] = 0;
  local_30[0xb] = 0;
  if (param_3 == (int *)0x0) {
    lVar4 = FileStream_SeekRead(local_30,0x400,0);
    if (lVar4 == -0x80) {
      return -0x80;
    }
    if ((lVar4 < 0x100000000) && (lVar4 < 0)) {
      return -0x84;
    }
    param_3 = local_30;
  }
  uVar1 = AthenaList_Sort_14();
  BitStream_InitWithBytes(*(undefined4 **)((int)this + 0x60),uVar1);
  if (in_EAX != (undefined4 *)0x0) {
    *in_EAX = *(undefined4 *)(*(int *)((int)this + 0x60) + 0x18);
  }
  *(undefined4 *)((int)this + 0x40) = 3;
  Vorbis_AllocDecoder(param_1);
  OggPacket_Init(param_2);
  iVar3 = 0;
LAB_00477e28:
  AthenaList_SortMerge(*(int **)((int)this + 0x60),param_3);
  if (iVar3 < 3) {
    while (iVar2 = D3DX_Uninit(), iVar2 != 0) {
      if (iVar2 == -1) goto LAB_00477ec8;
      iVar2 = Vorbis_ProcessPacket(param_1,(int)param_2,local_30 + 4);
      if (iVar2 != 0) goto LAB_00477ecd;
      iVar3 = iVar3 + 1;
      if (2 < iVar3) goto LAB_00477e7d;
    }
    if (iVar3 < 3) goto code_r0x00477ea3;
  }
LAB_00477e7d:
  BitStream_dtor(local_30 + 4);
  AthenaList_FreeAll(local_30);
  return 0;
code_r0x00477ea3:
  lVar4 = FileStream_SeekRead(param_3,0x400,0);
  if (lVar4 < 0) {
LAB_00477ec8:
    iVar2 = -0x85;
LAB_00477ecd:
    BitStream_dtor(local_30 + 4);
    AthenaList_FreeAll(local_30);
    D3DResourcePool_Release(param_1);
    MeshGroup_dtor(param_2);
    *(undefined4 *)((int)this + 0x40) = 2;
    return iVar2;
  }
  goto LAB_00477e28;
}

