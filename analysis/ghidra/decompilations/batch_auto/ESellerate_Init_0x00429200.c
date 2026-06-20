
void __fastcall ESellerate_Init(int param_1)

{
  int iVar1;
  
  if (*(char *)(param_1 + 0x218) == '\0') {
    *(undefined1 *)(param_1 + 0x218) = 1;
    iVar1 = eSellerate_ExtractDLLNull();
    if (iVar1 == 1) {
      MessageBoxA((HWND)0x0,"Count not install eSellerate Engine!","HEY",0);
    }
  }
  return;
}

