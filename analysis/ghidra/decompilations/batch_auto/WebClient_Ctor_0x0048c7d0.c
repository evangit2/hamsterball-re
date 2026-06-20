
undefined4 * __fastcall WebClient_Ctor(undefined4 *param_1)

{
  undefined4 *this;
  undefined4 *this_00;
  int iVar1;
  WNDCLASSEXA WStack_3c;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ce071;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_WebClient_DeletingDtor_004dbe18;
  this = param_1 + 2;
  *this = &PTR_AthenaCString_DeletingDtor_004d290c;
  param_1[3] = 0;
  param_1[5] = 0;
  *(undefined1 *)(param_1 + 8) = 1;
  param_1[4] = 0;
  param_1[7] = 0;
  *(undefined1 *)(param_1 + 6) = 0;
  AthenaString_AssignCStr(this,(char *)0x0);
  this_00 = param_1 + 9;
  local_4 = 0;
  *this_00 = &PTR_AthenaCString_DeletingDtor_004d290c;
  param_1[10] = 0;
  param_1[0xc] = 0;
  *(undefined1 *)(param_1 + 0xf) = 1;
  param_1[0xb] = 0;
  param_1[0xe] = 0;
  *(undefined1 *)(param_1 + 0xd) = 0;
  AthenaString_AssignCStr(this_00,(char *)0x0);
  local_4._0_1_ = 1;
  param_1[0x10] = &PTR_AthenaCString_DeletingDtor_004d290c;
  param_1[0x11] = 0;
  param_1[0x13] = 0;
  *(undefined1 *)(param_1 + 0x16) = 1;
  param_1[0x12] = 0;
  param_1[0x15] = 0;
  *(undefined1 *)(param_1 + 0x14) = 0;
  AthenaString_AssignCStr(param_1 + 0x10,(char *)0x0);
  local_4 = CONCAT31(local_4._1_3_,2);
  if (DAT_00535240 == 0) {
    iVar1 = Ordinal_115(0x101,param_1 + 0x120);
    if (iVar1 != 0) {
      ExceptionList = pvStack_c;
      return param_1;
    }
    WStack_3c.cbSize = 0x30;
    WStack_3c.style = 0;
    WStack_3c.lpfnWndProc = WebClient_WndProc;
    WStack_3c.cbClsExtra = 0;
    WStack_3c.cbWndExtra = 0;
    WStack_3c.hInstance = (HINSTANCE)0x0;
    WStack_3c.hCursor = (HCURSOR)0x0;
    WStack_3c.hIcon = (HICON)0x0;
    WStack_3c.hbrBackground = (HBRUSH)0x0;
    WStack_3c.lpszMenuName = (LPCSTR)0x0;
    WStack_3c.lpszClassName = "WebWindow";
    WStack_3c.hIconSm = (HICON)0x0;
    RegisterClassExA(&WStack_3c);
  }
  DAT_00535240 = DAT_00535240 + 1;
  param_1[1] = 0;
  AthenaString_AssignCStr(this,"");
  AthenaString_AssignCStr(this_00,"");
  param_1[0x117] = 0;
  param_1[0x11d] = 0;
  param_1[0x11e] = 0;
  param_1[0x118] = 0xffffffff;
  AthenaString_AssignCStr(param_1 + 0x10,"Raptisoftware");
  ExceptionList = pvStack_c;
  return param_1;
}

