
void __cdecl WebClient_InitResponse(undefined4 param_1)

{
  void *pvVar1;
  
  pvVar1 = _calloc(1,0x50);
  *(undefined4 *)((int)pvVar1 + 0x18) = param_1;
  *(undefined4 *)((int)pvVar1 + 0x1c) = 0xffffffff;
  return;
}

