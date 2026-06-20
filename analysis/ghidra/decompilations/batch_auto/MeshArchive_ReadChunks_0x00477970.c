
longlong __cdecl MeshArchive_ReadChunks(undefined4 *param_1)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  int *in_EAX;
  int iVar4;
  uint uVar5;
  bool bVar6;
  longlong lVar7;
  undefined8 local_10;
  
  uVar1 = in_EAX[2];
  iVar2 = in_EAX[3];
  local_10 = -1;
  iVar4 = iVar2;
  uVar5 = uVar1;
  do {
    bVar6 = 0x3ff < uVar5;
    uVar5 = uVar5 - 0x400;
    iVar4 = iVar4 + -1 + (uint)bVar6;
    if ((iVar4 < 1) && (iVar4 < 0)) {
      uVar5 = 0;
      iVar4 = 0;
    }
    if (*in_EAX != 0) {
      (*(code *)in_EAX[0x47])(*in_EAX,uVar5,iVar4,0);
      in_EAX[2] = uVar5;
      in_EAX[3] = iVar4;
      Pool_Reset(in_EAX[6]);
    }
    if (in_EAX[3] <= iVar2) {
      if (in_EAX[3] < iVar2) goto LAB_004779e0;
      uVar3 = in_EAX[2];
      while (uVar3 < uVar1) {
LAB_004779e0:
        do {
          lVar7 = FileStream_SeekRead(param_1,uVar1 - in_EAX[2],
                                      (iVar2 - in_EAX[3]) - (uint)(uVar1 < (uint)in_EAX[2]));
          if (lVar7 == -0x80) {
            return -0x80;
          }
          if (lVar7 < 0) goto LAB_00477a31;
          local_10 = lVar7;
        } while (in_EAX[3] < iVar2);
        if (iVar2 < in_EAX[3]) break;
        uVar3 = in_EAX[2];
      }
    }
LAB_00477a31:
    if (((uint)local_10 & local_10._4_4_) != 0xffffffff) {
      if (*in_EAX != 0) {
        (*(code *)in_EAX[0x47])(*in_EAX,(uint)local_10,local_10._4_4_,0);
        in_EAX[2] = (uint)local_10;
        in_EAX[3] = local_10._4_4_;
        Pool_Reset(in_EAX[6]);
      }
      lVar7 = FileStream_SeekRead(param_1,0x400,0);
      if (((int)((ulonglong)lVar7 >> 0x20) == 0 || lVar7 < 0) && (lVar7 < 0)) {
        return -0x81;
      }
      return local_10;
    }
  } while( true );
}

