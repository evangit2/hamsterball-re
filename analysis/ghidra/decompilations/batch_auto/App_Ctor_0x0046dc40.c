
undefined4 * __fastcall App_Ctor(undefined4 *param_1)

{
  char *pcVar1;
  DWORD DVar2;
  char *pcVar3;
  undefined4 *puVar4;
  undefined4 uVar5;
  void *pvVar6;
  HCURSOR pHVar7;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cd741;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_App_ScalarDtor_004d9750;
  param_1[0xf] = 0xf;
  param_1[0xe] = 0;
  *(undefined1 *)(param_1 + 10) = 0;
  local_4 = 0;
  param_1[0x82] = "(none)";
  BugTracker_InitBrushes(0x534608);
  DVar2 = GetTickCount();
  param_1[0x81] = DVar2;
  param_1[0x83] = &DAT_004d97f8;
  param_1[0x84] = "Startup";
  param_1[8] = 0;
  pcVar1 = "";
  do {
    pcVar3 = pcVar1;
    pcVar1 = pcVar3 + 1;
  } while (*pcVar3 != '\0');
  StdString_Assign(param_1 + 9,(undefined4 *)&DAT_004d1354,(uint)(pcVar3 + -0x4d1354));
  puVar4 = operator_new(0xc);
  local_4._0_1_ = 1;
  if (puVar4 == (undefined4 *)0x0) {
    uVar5 = 0;
  }
  else {
    uVar5 = RegKey_Ctor(puVar4);
  }
  param_1[0x15] = uVar5;
  param_1[0x57] = 0x280;
  param_1[0x58] = 0x1e0;
  param_1[0x5d] = 0;
  *(undefined1 *)((int)param_1 + 0x15a) = 1;
  param_1[0x6d] = 0;
  *(undefined1 *)(param_1 + 0x80) = 0;
  local_4._0_1_ = 0;
  DAT_005341e0 = param_1;
  *(undefined1 *)(param_1 + 0x56) = 0;
  param_1[2] = 0;
  *(undefined1 *)((int)param_1 + 0x159) = 0;
  param_1[0x59] = 0;
  param_1[99] = 0;
  param_1[100] = 0;
  param_1[0x65] = 0;
  *(undefined1 *)(param_1 + 0x6b) = 0;
  *(undefined1 *)(param_1 + 0x75) = 0;
  *(undefined1 *)((int)param_1 + 0x1ad) = 0;
  param_1[0x62] = 0;
  param_1[3] = 0;
  param_1[0x5b] = 100;
  param_1[0x5c] = 0x4b;
  param_1[0x74] = 1;
  pvVar6 = operator_new(0x848);
  local_4 = CONCAT31(local_4._1_3_,2);
  if (pvVar6 == (void *)0x0) {
    pvVar6 = (void *)0x0;
  }
  else {
    pvVar6 = MeshWorld_ctor_v2(pvVar6,param_1);
  }
  param_1[0x61] = pvVar6;
  param_1[0x5d] = 0;
  param_1[0x5e] = 0;
  param_1[0x5f] = 0;
  param_1[0x60] = 0;
  param_1[0x7b] = 0;
  pHVar7 = LoadCursorA((HINSTANCE)0x0,&DAT_00007f00);
  param_1[0x77] = pHVar7;
  pHVar7 = LoadCursorA((HINSTANCE)0x0,&DAT_00007f02);
  param_1[0x78] = pHVar7;
  pHVar7 = LoadCursorA((HINSTANCE)0x0,&DAT_00007f89);
  param_1[0x79] = pHVar7;
  pHVar7 = LoadCursorA((HINSTANCE)0x0,&DAT_00007f01);
  param_1[0x7a] = pHVar7;
  Texture_SetDimensions(param_1,param_1[0x57],param_1[0x58]);
  param_1[0x6c] = 0;
  param_1[0x76] = param_1[0x77];
  CoInitialize((LPVOID)0x0);
  ExceptionList = local_c;
  return param_1;
}

