
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004629e0(int *param_1,char *param_2)

{
  char cVar1;
  float fVar2;
  char *pcVar3;
  uint *puVar4;
  int _FileHandle;
  undefined4 *puVar5;
  void *pvVar6;
  undefined4 *puVar7;
  void *_DstBuf;
  int *piVar8;
  undefined4 unaff_EBX;
  int iVar9;
  char *pcVar10;
  undefined4 uStack_5a8;
  float fStack_5a4;
  int iStack_5a0;
  int iStack_59c;
  uint uStack_598;
  undefined4 uStack_594;
  undefined4 uStack_590;
  undefined4 uStack_58c;
  undefined4 *puStack_588;
  undefined4 *puStack_584;
  undefined4 uStack_580;
  int iStack_57c;
  int iStack_578;
  int iStack_574;
  undefined4 uStack_570;
  undefined4 uStack_56c;
  undefined4 uStack_568;
  undefined4 uStack_564;
  undefined4 uStack_560;
  int iStack_55c;
  undefined4 uStack_558;
  int iStack_554;
  int iStack_550;
  undefined4 uStack_54c;
  float afStack_548 [5];
  undefined4 auStack_534 [5];
  undefined4 auStack_520 [5];
  char acStack_50c [255];
  char cStack_40d;
  undefined4 uStack_40c;
  void *pvStack_14;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004cd128;
  pvStack_c = ExceptionList;
  pcVar3 = param_2;
  do {
    cVar1 = *pcVar3;
    pcVar3[(int)&uStack_40c - (int)param_2] = cVar1;
    pcVar3 = pcVar3 + 1;
  } while (cVar1 != '\0');
  ExceptionList = &pvStack_c;
  puVar4 = FUN_004bacc0(&uStack_40c,'.');
  *(undefined1 *)puVar4 = 0;
  pcVar3 = &cStack_40d;
  do {
    pcVar10 = pcVar3;
    pcVar3 = pcVar10 + 1;
  } while (pcVar10[1] != '\0');
  *(undefined4 *)(pcVar10 + 1) = s__cached_004d9030._0_4_;
  pvVar6 = (void *)param_1[0x120];
  pcVar3 = (char *)&uStack_40c;
  *(undefined4 *)(pcVar10 + 5) = s__cached_004d9030._4_4_;
  do {
    cVar1 = *pcVar3;
    pcVar3 = pcVar3 + 1;
  } while (cVar1 != '\0');
  FUN_00426e90(pvVar6,&uStack_40c,(int)pcVar3 - ((int)&uStack_40c + 1));
  _FileHandle = FID_conflict___open(param_2,0x8000);
  __read(_FileHandle,&iStack_578,4);
  iStack_5a0 = 0;
  if (0 < iStack_578) {
    do {
      __read(_FileHandle,&uStack_598,4);
      puVar5 = operator_new(0x7c);
      uStack_4 = 0;
      puStack_588 = puVar5;
      if (puVar5 == (undefined4 *)0x0) {
        puVar5 = (undefined4 *)0x0;
      }
      else {
        FUN_00457fa0(puVar5 + 0xb);
        *puVar5 = 0;
        if ((float *)(puVar5 + 7) != afStack_548) {
          puVar5[7] = 1.0;
          puVar5[8] = 0x3f800000;
          puVar5[9] = 0x3f800000;
        }
      }
      uStack_4 = 0xffffffff;
      pvVar6 = operator_new(uStack_598);
      *puVar5 = pvVar6;
      __read(_FileHandle,pvVar6,uStack_598);
      __read(_FileHandle,&uStack_5a8,4);
      puVar5[1] = uStack_5a8;
      __read(_FileHandle,&uStack_5a8,4);
      puVar5[2] = uStack_5a8;
      __read(_FileHandle,&uStack_5a8,4);
      puVar5[3] = uStack_5a8;
      __read(_FileHandle,&uStack_5a8,4);
      puVar5[4] = uStack_5a8;
      __read(_FileHandle,&uStack_5a8,4);
      puVar5[5] = uStack_5a8;
      __read(_FileHandle,&uStack_5a8,4);
      puVar5[6] = uStack_5a8;
      __read(_FileHandle,puVar5 + 10,4);
      if (*(char *)(puVar5 + 10) != '\0') {
        __read(_FileHandle,&uStack_594,4);
        __read(_FileHandle,&uStack_590,4);
        __read(_FileHandle,&iStack_59c,4);
        __read(_FileHandle,&fStack_5a4,4);
        fVar2 = (float)_DAT_004cf3c8;
        puVar5[0x10] = uStack_594;
        puVar5[0x13] = fStack_5a4;
        puVar5[0x11] = uStack_590;
        puVar5[0x12] = iStack_59c;
        *(bool *)(puVar5 + 0x1e) = (float)puVar5[0xf] != fVar2;
        __read(_FileHandle,&uStack_594,4);
        __read(_FileHandle,&uStack_590,4);
        __read(_FileHandle,&iStack_59c,4);
        __read(_FileHandle,&fStack_5a4,4);
        puVar5[0xf] = fStack_5a4;
        fVar2 = (float)_DAT_004cf3c8;
        puVar5[0xd] = uStack_590;
        puVar5[0xc] = uStack_594;
        puVar5[0xe] = iStack_59c;
        *(bool *)(puVar5 + 0x1e) = fStack_5a4 != fVar2;
        __read(_FileHandle,&uStack_560,4);
        __read(_FileHandle,&uStack_54c,4);
        __read(_FileHandle,&uStack_570,4);
        __read(_FileHandle,&uStack_558,4);
        fVar2 = (float)_DAT_004cf3c8;
        puVar5[0x14] = uStack_560;
        puVar5[0x17] = uStack_558;
        puVar5[0x15] = uStack_54c;
        puVar5[0x16] = uStack_570;
        *(bool *)(puVar5 + 0x1e) = (float)puVar5[0xf] != fVar2;
        __read(_FileHandle,&uStack_594,4);
        __read(_FileHandle,&uStack_590,4);
        __read(_FileHandle,&iStack_59c,4);
        __read(_FileHandle,&fStack_5a4,4);
        fVar2 = (float)_DAT_004cf3c8;
        puVar5[0x19] = uStack_590;
        puVar5[0x18] = uStack_594;
        puVar5[0x1a] = iStack_59c;
        puVar5[0x1b] = fStack_5a4;
        *(bool *)(puVar5 + 0x1e) = (float)puVar5[0xf] != fVar2;
        __read(_FileHandle,&uStack_564,4);
        puVar5[0x1c] = uStack_564;
        __read(_FileHandle,&iStack_554,4);
        *(bool *)((int)puVar5 + 0x79) = iStack_554 != 0;
        __read(_FileHandle,&iStack_55c,4);
        if (iStack_55c == 1) {
          __read(_FileHandle,&puStack_584,4);
          __read(_FileHandle,acStack_50c,(uint)puStack_584);
          puVar7 = Graphics_LoadTexture((void *)param_1[1],acStack_50c,'\x01');
          puVar5[0x1d] = puVar7;
        }
      }
      thunk_FUN_00453780((void *)(param_1[0x120] + 0x894),(int)puVar5);
      iStack_5a0 = iStack_5a0 + 1;
    } while (iStack_5a0 < iStack_578);
  }
  __read(_FileHandle,&iStack_57c,4);
  iStack_5a0 = 0;
  if (0 < iStack_57c) {
    do {
      __read(_FileHandle,&uStack_598,4);
      puStack_588 = operator_new(0x54);
      uStack_4 = 1;
      if (puStack_588 == (undefined4 *)0x0) {
        pvVar6 = (void *)0x0;
      }
      else {
        pvVar6 = (void *)FUN_00468510((int)puStack_588);
      }
      uStack_4 = 0xffffffff;
      _DstBuf = operator_new(uStack_598);
      *(void **)((int)pvVar6 + 0x50) = _DstBuf;
      __read(_FileHandle,_DstBuf,uStack_598);
      __read(_FileHandle,&iStack_59c,4);
      iVar9 = 0;
      if (0 < iStack_59c) {
        do {
          __read(_FileHandle,&fStack_5a4,4);
          afStack_548[0] = fStack_5a4;
          __read(_FileHandle,&fStack_5a4,4);
          __read(_FileHandle,&fStack_5a4,4);
          FUN_004685e0(pvVar6);
          iVar9 = iVar9 + 1;
        } while (iVar9 < iStack_59c);
      }
      FUN_00469090(pvVar6,'\0');
      thunk_FUN_00453780((void *)(param_1[0x120] + 0xcac),(int)pvVar6);
      iStack_5a0 = iStack_5a0 + 1;
    } while (iStack_5a0 < iStack_57c);
  }
  __read(_FileHandle,&iStack_574,4);
  uStack_598 = 0;
  if (0 < iStack_574) {
    do {
      __read(_FileHandle,&iStack_550,4);
      puStack_584 = operator_new(0xd4);
      uStack_4 = 2;
      if (puStack_584 == (undefined4 *)0x0) {
        piVar8 = (int *)0x0;
      }
      else {
        piVar8 = FUN_0046b4f0(puStack_584,param_1[1]);
      }
      uStack_4 = 0xffffffff;
      if (iStack_550 == 0) {
        piVar8[0x34] = 3;
        __read(_FileHandle,&iStack_5a0,4);
        __read(_FileHandle,&uStack_58c,4);
        __read(_FileHandle,&uStack_580,4);
        (**(code **)(*piVar8 + 4))(iStack_5a0,uStack_58c,uStack_580);
        __read(_FileHandle,&stack0xfffffa54,4);
        __read(_FileHandle,&uStack_598,4);
        __read(_FileHandle,&uStack_58c,4);
        (**(code **)(*piVar8 + 8))(unaff_EBX,uStack_598,uStack_58c);
        __read(_FileHandle,&puStack_588,4);
        __read(_FileHandle,&uStack_568,4);
        __read(_FileHandle,&uStack_56c,4);
        iVar9 = FUN_00453180(afStack_548,puStack_588,uStack_568,uStack_56c);
        piVar8[0x25] = *(int *)(iVar9 + 4);
        piVar8[0x26] = *(int *)(iVar9 + 8);
        piVar8[0x27] = *(int *)(iVar9 + 0xc);
        piVar8[0x28] = *(int *)(iVar9 + 0x10);
        FUN_00453200(afStack_548);
        iVar9 = FUN_00453180(auStack_520,0x3f400000,0x3f400000,0x3f400000);
        piVar8[0x2f] = *(int *)(iVar9 + 4);
        piVar8[0x30] = *(int *)(iVar9 + 8);
        piVar8[0x31] = *(int *)(iVar9 + 0xc);
        piVar8[0x32] = *(int *)(iVar9 + 0x10);
        FUN_00453200(auStack_520);
        iVar9 = FUN_00453180(auStack_534,0,0,0);
        piVar8[0x2a] = *(int *)(iVar9 + 4);
        piVar8[0x2b] = *(int *)(iVar9 + 8);
        piVar8[0x2c] = *(int *)(iVar9 + 0xc);
        piVar8[0x2d] = *(int *)(iVar9 + 0x10);
        FUN_00453200(auStack_534);
        thunk_FUN_00453780((void *)(param_1[0x120] + 0x478),(int)piVar8);
      }
      uStack_598 = uStack_598 + 1;
    } while ((int)uStack_598 < iStack_574);
  }
  __read(_FileHandle,(void *)(param_1[0x120] + 0x454),4);
  __read(_FileHandle,(void *)(param_1[0x120] + 0x458),4);
  __read(_FileHandle,(void *)(param_1[0x120] + 0x45c),4);
  __read(_FileHandle,(void *)(param_1[0x120] + 0x468),4);
  __read(_FileHandle,(void *)(param_1[0x120] + 0x46c),4);
  __read(_FileHandle,(void *)(param_1[0x120] + 0x470),4);
  __read(_FileHandle,(void *)(param_1[0x120] + 0x438),4);
  *(undefined4 *)(param_1[0x120] + 0x43c) = 0;
  pvVar6 = operator_new(*(int *)(param_1[0x120] + 0x438) << 5);
  *(void **)(param_1[0x120] + 0x440) = pvVar6;
  __read(_FileHandle,*(void **)(param_1[0x120] + 0x440),*(int *)(param_1[0x120] + 0x438) << 5);
  param_1[0x11f] = (int)param_1;
  *(undefined1 *)((int)param_1 + 0x431) = 1;
  puStack_584 = operator_new(0x44);
  uStack_4 = 3;
  if (puStack_584 == (undefined4 *)0x0) {
    iVar9 = 0;
  }
  else {
    iVar9 = Timer_Init(puStack_584);
  }
  uStack_4 = 0xffffffff;
  param_1[0x10d] = iVar9;
  (**(code **)(*param_1 + 0x34))(_FileHandle,param_1);
  __close(_FileHandle);
  FUN_004601a0((int)param_1);
  ExceptionList = pvStack_14;
  return;
}

