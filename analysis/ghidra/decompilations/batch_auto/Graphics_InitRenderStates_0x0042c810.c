
void __fastcall Graphics_InitRenderStates(void *param_1)

{
  ulonglong uVar1;
  ulonglong uVar2;
  ulonglong uVar3;
  
  uVar1 = __ftol2();
  uVar2 = __ftol2();
  uVar3 = __ftol2();
  Graphics_ClearViewport
            (param_1,(((uint)uVar1 | 0xffffff00) << 8 | (uint)uVar2 & 0xff) << 8 |
                     (uint)uVar3 & 0xff);
  return;
}

