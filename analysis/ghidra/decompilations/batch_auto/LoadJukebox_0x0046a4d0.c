
void LoadJukebox(char *param_1)

{
  char cVar1;
  char *pcVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 *puVar5;
  char *pcVar6;
  char *****pppppcVar7;
  undefined4 uVar8;
  undefined4 local_60;
  void *local_5c;
  undefined4 local_58 [6];
  undefined4 local_40 [6];
  undefined1 local_28 [4];
  char ****local_24 [4];
  undefined4 local_14;
  uint local_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  int local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd4b8;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  Sprite_DrawColoredRect(local_40);
  local_4 = 0;
  FileHandle_Open(local_40,param_1);
  puVar3 = MWParser_ReadTag((int)local_40);
  while (puVar3 != (undefined4 *)0x0) {
    iVar4 = __stricmp((char *)puVar3[1],"SONG");
    if (iVar4 == 0) {
      local_10 = 0xf;
      local_14 = 0;
      local_24[0] = (char ****)((uint)local_24[0] & 0xffffff00);
      pcVar2 = "";
      do {
        pcVar6 = pcVar2;
        pcVar2 = pcVar6 + 1;
      } while (*pcVar6 != '\0');
      StdString_Assign(local_28,(undefined4 *)&DAT_004d1354,(uint)(pcVar6 + -0x4d1354));
      local_4._0_1_ = 1;
      uVar8 = 0;
      Sprite_DrawColoredRect(local_58);
      local_4 = CONCAT31(local_4._1_3_,2);
      AthenaString_Set(local_58,(char *)puVar3[2]);
      puVar5 = MWParser_ReadTag((int)local_58);
      while (puVar5 != (undefined4 *)0x0) {
        iVar4 = __stricmp((char *)puVar5[1],"NAME");
        if (iVar4 == 0) {
          pcVar2 = (char *)puVar5[2];
          pcVar6 = pcVar2;
          do {
            cVar1 = *pcVar6;
            pcVar6 = pcVar6 + 1;
          } while (cVar1 != '\0');
          StdString_Assign(local_28,(undefined4 *)pcVar2,(int)pcVar6 - (int)(pcVar2 + 1));
        }
        iVar4 = __stricmp((char *)puVar5[1],"HEX");
        if ((iVar4 == 0) &&
           (iVar4 = _sscanf((char *)puVar5[2],"%x",&local_60), uVar8 = local_60, iVar4 != 1)) {
          local_60 = 0;
          uVar8 = local_60;
        }
        (**(code **)*puVar5)(1);
        puVar5 = MWParser_ReadTag((int)local_58);
      }
      pppppcVar7 = (char *****)local_24[0];
      if (local_10 < 0x10) {
        pppppcVar7 = local_24;
      }
      RegKeyList_AppendStr(local_5c,(char *)pppppcVar7,uVar8);
      local_4._0_1_ = 1;
      StreamReader_dtor(local_58);
      local_4 = (uint)local_4._1_3_ << 8;
      if (0xf < local_10) {
        _free(local_24[0]);
      }
      local_10 = 0xf;
      local_14 = 0;
      local_24[0] = (char ****)((uint)local_24[0] & 0xffffff00);
    }
    (**(code **)*puVar3)(1);
    puVar3 = MWParser_ReadTag((int)local_40);
  }
  local_4 = 0xffffffff;
  StreamReader_dtor(local_40);
  ExceptionList = pvStack_c;
  return;
}

