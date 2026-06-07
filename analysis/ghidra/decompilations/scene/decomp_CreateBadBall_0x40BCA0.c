// CreateBadBall (0x40BCA0) — Raw Ghidra decompilation
// Spawns 8-ball AI opponents from MESHWORLD objects named "BADBALL<tags>"

void __fastcall CreateBadBall(int param_1)
{
  float fVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  void *this;
  int *piVar5;
  long lVar6;
  undefined4 *puVar7;
  undefined4 auStack_24 [6];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9623;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  iVar3 = AthenaList_NextIndex(*(int *)(*(int *)(param_1 + 0x8ac) + 0x480) + 0x894);
  iVar4 = *(int *)(*(int *)(param_1 + 0x8ac) + 0x480);
  *(undefined4 *)(iVar4 + 0x89c + iVar3 * 4) = 0;
  if (*(int *)(iVar4 + 0x898) < 1) {
    puVar7 = (undefined4 *)0x0;
  }
  else {
    puVar7 = (undefined4 *)**(undefined4 **)(iVar4 + 0xca0);
    *(undefined4 *)(iVar4 + 0x89c + iVar3 * 4) = 1;
  }
  while( true ) {
    if (puVar7 == (undefined4 *)0x0) {
      ExceptionList = local_c;
      return;
    }
    iVar4 = __strnicmp((char *)*puVar7,"BADBALL",7);
    if (iVar4 == 0) {
      this = operator_new(0xc98);
      local_4 = 0;
      if (this == (void *)0x0) {
        piVar5 = (int *)0x0;
      }
      else {
        piVar5 = Ball_ctor(this,param_1);
      }
      local_4 = 0xffffffff;
      (**(code **)(*piVar5 + 4))();
      piVar5[0x59] = (int)((float)puVar7[1] + _DAT_004cf55c);
      piVar5[0x5a] = (int)((float)puVar7[2] + (float)piVar5[0xa1]);
      fVar1 = (float)puVar7[3];
      *(undefined1 *)((int)piVar5 + 0x281) = 0;
      piVar5[0x5b] = (int)(fVar1 + _DAT_004cf55c);
      if (piVar5 + 0x318 != puVar7 + 1) {
        piVar5[0x318] = puVar7[1];
        piVar5[0x319] = puVar7[2];
        piVar5[0x31a] = puVar7[3];
      }
      Sprite_DrawColoredRect(auStack_24);
      local_4 = 1;
      AthenaString_Set(auStack_24,(char *)*puVar7);
      puVar7 = MWParser_ReadTag((int)auStack_24);
      while (puVar7 != (undefined4 *)0x0) {
        iVar4 = __stricmp((char *)puVar7[1],"CHASE");
        if (iVar4 == 0) {
          lVar6 = _atol((char *)puVar7[2]);
          piVar5[0x31b] = (int)(float)lVar6;
        }
        iVar4 = __stricmp((char *)puVar7[1],"HOME");
        if (iVar4 == 0) {
          lVar6 = _atol((char *)puVar7[2]);
          piVar5[0x31c] = (int)(float)lVar6;
        }
        iVar4 = __stricmp((char *)puVar7[1],"SIZE");
        if (iVar4 == 0) {
          lVar6 = _atol((char *)puVar7[2]);
          piVar5[0x62] = 0x40400000;
          piVar5[0xa1] = (int)(float)lVar6;
          piVar5[0x9f] = 0;
          *(undefined1 *)(piVar5 + 0x313) = 1;
        }
        iVar4 = __stricmp((char *)puVar7[1],"SPINDISTANCE");
        if (iVar4 == 0) {
          lVar6 = _atol((char *)puVar7[2]);
          piVar5[799] = (int)(float)lVar6;
        }
        (**(code **)*puVar7)(1);
        puVar7 = MWParser_ReadTag((int)auStack_24);
      }
      AthenaList_Append((void *)(param_1 + 0x29d4),(int)piVar5);
      AthenaList_Append((void *)(param_1 + 0x2dec),(int)piVar5);
      local_4 = 0xffffffff;
      StreamReader_dtor(auStack_24);
    }
    iVar4 = *(int *)(*(int *)(param_1 + 0x8ac) + 0x480);
    iVar2 = *(int *)(iVar4 + 0x89c + iVar3 * 4);
    if (*(int *)(iVar4 + 0x898) <= iVar2) break;
    puVar7 = *(undefined4 **)(*(int *)(iVar4 + 0xca0) + iVar2 * 4);
    *(int *)(iVar4 + 0x89c + iVar3 * 4) = iVar2 + 1;
  }
  ExceptionList = local_c;
  return;
}