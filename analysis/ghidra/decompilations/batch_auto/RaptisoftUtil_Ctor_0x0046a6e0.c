
undefined4 * __fastcall RaptisoftUtil_Ctor(undefined4 *param_1)

{
  HWND hWnd;
  undefined1 local_c [4];
  undefined4 local_8;
  undefined4 local_4;
  
  *param_1 = &PTR_RaptisoftUtil_DeletingDtor_004d92ec;
  *(undefined1 *)((int)param_1 + 0xd) = 1;
  param_1[1] = 0;
  hWnd = FindWindowA((LPCSTR)0x0,"Raptisoft Utility");
  param_1[1] = hWnd;
  if (hWnd == (HWND)0x0) {
    *(undefined1 *)((int)param_1 + 0xd) = 0;
    *(undefined1 *)((int)param_1 + 0xe) = 1;
    *(undefined1 *)(param_1 + 3) = 1;
    return param_1;
  }
  local_8 = 0;
  local_4 = 0;
  SendMessageA(hWnd,0x4a,0,(LPARAM)local_c);
  *(undefined1 *)((int)param_1 + 0xe) = 1;
  *(undefined1 *)(param_1 + 3) = 1;
  return param_1;
}

