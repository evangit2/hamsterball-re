
void LicenseKey_EncryptBlock(undefined4 *param_1,undefined4 *param_2,ushort param_3)

{
  byte bVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 *puVar5;
  int iVar6;
  undefined4 *puVar7;
  undefined4 *puVar8;
  byte *pbVar9;
  char acStack_1c8 [116];
  undefined4 local_148 [25];
  undefined4 local_e4 [25];
  undefined4 local_80 [25];
  undefined4 local_1c;
  undefined4 uStack_18;
  undefined4 uStack_14;
  undefined1 uStack_10;
  int local_c;
  undefined4 *local_8;
  
  puVar2 = param_1;
  local_8 = local_80;
  iVar6 = 0;
  puVar8 = local_e4;
  for (iVar4 = 0x19; iVar4 != 0; iVar4 = iVar4 + -1) {
    *puVar8 = *param_2;
    param_2 = param_2 + 1;
    puVar8 = puVar8 + 1;
  }
  do {
    *(char *)((int)local_80 + iVar6) = (char)iVar6;
    iVar6 = iVar6 + 1;
  } while (iVar6 < 100);
  local_c = 0x10;
  puVar8 = local_148;
  puVar5 = local_e4;
  do {
    puVar3 = puVar8;
    if ((param_3 & 1) != 0) {
      param_2 = (undefined4 *)&DAT_00000064;
      puVar7 = puVar8;
      do {
        puVar3 = local_8;
        *(undefined1 *)puVar7 =
             *(undefined1 *)
              ((int)*(char *)(((int)puVar5 - (int)puVar8) + (int)puVar7) + (int)local_8);
        puVar7 = (undefined4 *)((int)puVar7 + 1);
        param_2 = (undefined4 *)((int)param_2 + -1);
      } while (param_2 != (undefined4 *)0x0);
      local_8 = puVar8;
    }
    iVar4 = 100;
    puVar8 = puVar3;
    do {
      *(undefined1 *)puVar8 =
           *(undefined1 *)((int)*(char *)((int)puVar8 + ((int)puVar5 - (int)puVar3)) + (int)puVar5);
      puVar8 = (undefined4 *)((int)puVar8 + 1);
      iVar4 = iVar4 + -1;
    } while (iVar4 != 0);
    param_3 = param_3 >> 1;
    local_c = local_c + -1;
    puVar8 = puVar5;
    puVar5 = puVar3;
  } while (local_c != 0);
  pbVar9 = (byte *)&local_1c;
  bVar1 = *(byte *)(param_1 + 3);
  local_1c = 0;
  uStack_18 = 0;
  uStack_14 = 0;
  param_1 = (undefined4 *)0x0;
  uStack_10 = 0;
  do {
    *pbVar9 = 0;
    param_2 = (undefined4 *)0x7;
    do {
      if (99 < (int)param_1) break;
      *pbVar9 = *pbVar9 | (*(byte *)((int)*(char *)((int)param_1 + (int)local_8) / 8 + (int)puVar2)
                           >> (7U - *(char *)((int)param_1 + (int)local_8) % '\b' & 0x1f) & 1) <<
                          ((byte)param_2 & 0x1f);
      param_2 = (undefined4 *)((int)param_2 + -1);
      param_1 = (undefined4 *)((int)param_1 + 1);
    } while (-1 < (int)param_2);
    pbVar9 = pbVar9 + 1;
    if (99 < (int)param_1) {
      *puVar2 = local_1c;
      puVar2[1] = uStack_18;
      puVar2[2] = uStack_14;
      *(undefined1 *)(puVar2 + 3) = uStack_10;
      *(byte *)(puVar2 + 3) = *(byte *)(puVar2 + 3) | bVar1 & 0xf;
      return;
    }
  } while( true );
}

