
void __cdecl Window_Notify(int param_1,byte *param_2)

{
  char cVar1;
  char *hMem;
  undefined1 local_c [4];
  char *local_8;
  char *local_4;
  
  if (*(char *)(param_1 + 0xc) != '\0') {
    if (*(char *)(param_1 + 0xe) != '\0') {
      *(undefined1 *)(param_1 + 0xe) = 0;
      Window_Notify(param_1,(byte *)"*** BEGIN RAPTISOFT SESSION ***");
    }
    if ((*(char *)(param_1 + 0xd) != '\0') && (*(int *)(param_1 + 4) != 0)) {
      hMem = GlobalAlloc(0,0x800);
      AthenaString_Sprintf(hMem,param_2,(wchar_t *)&stack0x0000000c);
      local_8 = hMem;
      do {
        cVar1 = *local_8;
        local_8 = local_8 + 1;
      } while (cVar1 != '\0');
      local_8 = local_8 + (1 - (int)(hMem + 1));
      local_4 = hMem;
      SendMessageA(*(HWND *)(param_1 + 4),0x4a,0,(LPARAM)local_c);
      GlobalFree(hMem);
    }
  }
  return;
}

