/* Ghidra Decompilation
 * Function: LoadRaceData
 * Address: 0x40A120
 * Decompiled: 2026-06-19
 */


void __thiscall LoadRaceData(void *this,char *param_1)

{
  undefined4 *puVar1;
  int iVar2;
  undefined4 *puVar3;
  ulonglong uVar4;
  double dVar5;
  undefined4 local_3c [6];
  undefined4 local_24 [6];
  void *pvStack_c;
  undefined1 *puStack_8;
  uint local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9560;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  Sprite_DrawColoredRect(local_24);
  local_4 = 0;
  FileHandle_Open(local_24,"racedata.xml");
  *(undefined4 *)((int)this + 0x2990) = 0;
  *(undefined4 *)((int)this + 0x2998) = 0;
  *(undefined4 *)((int)this + 0x2994) = 0;
  puVar1 = MWParser_ReadTag((int)local_24);
  while (puVar1 != (undefined4 *)0x0) {
    iVar2 = __stricmp((char *)puVar1[1],param_1);
    if (iVar2 == 0) {
      Sprite_DrawColoredRect(local_3c);
      local_4 = CONCAT31(local_4._1_3_,1);
      AthenaString_Set(local_3c,(char *)puVar1[2]);
      puVar3 = MWParser_ReadTag((int)local_3c);
      while (puVar3 != (undefined4 *)0x0) {
        iVar2 = __stricmp((char *)puVar3[1],"TIME");
        if (iVar2 == 0) {
          _atof((char *)puVar3[2]);
          uVar4 = __ftol2();
          *(int *)((int)this + 0x2998) = (int)uVar4;
        }
        iVar2 = __stricmp((char *)puVar3[1],(char *)&PTR_DAT_004cf5b8);
        if (iVar2 == 0) {
          _atof((char *)puVar3[2]);
          uVar4 = __ftol2();
          *(int *)((int)this + 0x2990) = 9 - (int)uVar4;
        }
        iVar2 = __stricmp((char *)puVar3[1],"WEASEL");
        if (iVar2 == 0) {
          _atof((char *)puVar3[2]);
          uVar4 = __ftol2();
          *(int *)((int)this + 0x2994) = 9 - (int)uVar4;
        }
        iVar2 = __stricmp((char *)puVar3[1],"GOLD");
        if (iVar2 == 0) {
          _atof((char *)puVar3[2]);
          uVar4 = __ftol2();
          *(int *)((int)this + 0x29a0) = 9 - (int)uVar4;
        }
        iVar2 = __stricmp((char *)puVar3[1],"SILVER");
        if (iVar2 == 0) {
          _atof((char *)puVar3[2]);
          uVar4 = __ftol2();
          *(int *)((int)this + 0x29a4) = 9 - (int)uVar4;
        }
        iVar2 = __stricmp((char *)puVar3[1],"BRONZE");
        if (iVar2 == 0) {
          _atof((char *)puVar3[2]);
          uVar4 = __ftol2();
          *(int *)((int)this + 0x29a8) = 9 - (int)uVar4;
        }
        iVar2 = __stricmp((char *)puVar3[1],(char *)&PTR_DAT_004cf588);
        if (iVar2 == 0) {
          dVar5 = _atof((char *)puVar3[2]);
          *(float *)((int)this + 0x299c) = (float)dVar5;
        }
        (**(code **)*puVar3)(1);
        puVar3 = MWParser_ReadTag((int)local_3c);
      }
      local_4 = local_4 & 0xffffff00;
      StreamReader_dtor(local_3c);
    }
    (**(code **)*puVar1)(1);
    puVar1 = MWParser_ReadTag((int)local_24);
  }
  iVar2 = *(int *)((int)this + 0x878);
  *(undefined4 *)((int)this + 0x29ac) = *(undefined4 *)((int)this + 0x29a8);
  if (*(char *)(iVar2 + 0x8bc + *(int *)(*(int *)(iVar2 + 0x220) + 8) * 4) != '\0') {
    *(undefined4 *)((int)this + 0x29ac) = *(undefined4 *)((int)this + 0x29a4);
  }
  if (*(char *)(iVar2 + 0x8bd + *(int *)(*(int *)(iVar2 + 0x220) + 8) * 4) != '\0') {
    *(undefined4 *)((int)this + 0x29ac) = *(undefined4 *)((int)this + 0x29a0);
  }
  if (*(char *)(iVar2 + 0x8be + *(int *)(*(int *)(iVar2 + 0x220) + 8) * 4) != '\0') {
    *(undefined4 *)((int)this + 0x29ac) = *(undefined4 *)((int)this + 0x2994);
  }
  local_4 = 0xffffffff;
  StreamReader_dtor(local_24);
  ExceptionList = pvStack_c;
  return;
}

