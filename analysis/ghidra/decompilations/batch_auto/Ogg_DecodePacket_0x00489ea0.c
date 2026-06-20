
void Ogg_DecodePacket(void)

{
  undefined4 *puVar1;
  byte bVar2;
  undefined4 *in_EAX;
  undefined4 uVar3;
  int iVar4;
  undefined3 extraout_var;
  undefined3 extraout_var_00;
  undefined3 extraout_var_01;
  int iVar5;
  int iVar6;
  undefined8 uVar7;
  undefined4 *local_4c [4];
  undefined4 *local_3c;
  undefined4 *local_38;
  int local_34;
  int local_30;
  int local_2c;
  undefined4 *local_28;
  undefined4 *local_24;
  int local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 *local_14;
  undefined4 *local_10;
  int local_c;
  undefined4 local_8;
  undefined4 local_4;
  
  if (-1 < (int)in_EAX[0xd]) {
    while (in_EAX[1] != 0) {
      if (-1 < (int)in_EAX[0xc]) {
        uVar3 = D3DX_SurfaceClipBlit(in_EAX[0xc] + 0x1b);
        in_EAX[1] = uVar3;
      }
      puVar1 = (undefined4 *)in_EAX[1];
      in_EAX[0xc] = 0;
      in_EAX[0x11] = 0;
      in_EAX[0x10] = 0;
      if (puVar1 == (undefined4 *)0x0) {
        *in_EAX = 0;
        return;
      }
      local_18 = puVar1[2];
      local_20 = *(int *)*puVar1 + puVar1[1];
      local_1c = 0;
      local_4c[0] = puVar1;
      local_28 = puVar1;
      local_24 = puVar1;
      iVar4 = AthenaList_ReadDword(0x12);
      local_2c = puVar1[2];
      local_34 = puVar1[1] + *(int *)*puVar1;
      iVar5 = 0;
      local_30 = 0;
      local_38 = puVar1;
      if (local_2c < 0x1b) {
        iVar6 = puVar1[2];
        do {
          local_38 = (undefined4 *)local_38[3];
          iVar5 = iVar5 + iVar6;
          iVar6 = local_38[2];
          local_2c = iVar6 + iVar5;
          local_34 = local_38[1] + *(int *)*local_38;
          local_30 = iVar5;
        } while (local_2c < 0x1b);
      }
      in_EAX[0xc] = (uint)*(byte *)((local_34 - local_30) + 0x1a);
      local_3c = puVar1;
      if (iVar4 != in_EAX[7]) {
        in_EAX[0xe] = (in_EAX[7] != -1) + 1;
        iVar5 = D3DX_SurfaceClipBlit(in_EAX[0xd]);
        in_EAX[3] = iVar5;
        if (iVar5 == 0) {
          in_EAX[2] = 0;
        }
        in_EAX[0xd] = 0;
      }
      bVar2 = StreamReader_ReadFlag((int *)local_4c);
      if (CONCAT31(extraout_var,bVar2) == 0) {
        if (in_EAX[0xd] != 0) {
          iVar5 = D3DX_SurfaceClipBlit(in_EAX[0xd]);
          in_EAX[3] = iVar5;
          if (iVar5 == 0) {
            in_EAX[2] = 0;
          }
          in_EAX[0xd] = 0;
          goto LAB_0048a042;
        }
      }
      else if (in_EAX[0xd] == 0) {
        Ogg_CalculateCRC();
        iVar5 = D3DX_SurfaceClipBlit(in_EAX[0x12] & 0x7fffffff);
        in_EAX[3] = iVar5;
        if (iVar5 == 0) {
          in_EAX[2] = 0;
        }
LAB_0048a042:
        if ((in_EAX[0xf] == 0) && (in_EAX[0xe] == 0)) {
          in_EAX[0xf] = 2;
        }
      }
      if ((int)in_EAX[0x11] < (int)in_EAX[0xc]) {
        local_4 = puVar1[2];
        local_c = *(int *)*puVar1 + puVar1[1];
        local_8 = 0;
        local_14 = puVar1;
        local_10 = puVar1;
        uVar7 = StreamReader_ReadByteSequence(6);
        *(undefined8 *)(in_EAX + 10) = uVar7;
        Ogg_CalculateCRC();
        in_EAX[0xd] = in_EAX[0xd] + in_EAX[0x12];
        Ogg_CalculateCRC();
      }
      in_EAX[7] = iVar4 + 1;
      bVar2 = StreamReader_ReadSyncFlag((int *)local_4c);
      in_EAX[4] = CONCAT31(extraout_var_00,bVar2);
      bVar2 = StreamReader_ReadTypeFlag((int *)local_4c);
      in_EAX[5] = CONCAT31(extraout_var_01,bVar2);
      if ((int)in_EAX[0xd] < 0) {
        return;
      }
    }
  }
  return;
}

