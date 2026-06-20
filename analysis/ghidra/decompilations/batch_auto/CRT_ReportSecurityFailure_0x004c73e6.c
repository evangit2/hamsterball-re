
/* WARNING: Function: __SEH_prolog replaced with injection: SEH_prolog */
/* WARNING: Function: __chkstk replaced with injection: alloca_probe */

void CRT_ReportSecurityFailure(int param_1)

{
  DWORD DVar1;
  size_t sVar2;
  uint *_Dest;
  char *_Source;
  uint unaff_retaddr;
  uint local_12c [65];
  undefined1 local_28;
  uint *local_24;
  uint local_20;
  undefined1 *local_1c;
  undefined4 uStack_c;
  undefined *local_8;
  
  local_8 = &DAT_004ee5c8;
  uStack_c = 0x4c73f5;
  local_20 = DAT_004fce90 ^ unaff_retaddr;
  if (DAT_00535644 == (code *)0x0) {
    if (param_1 == 1) {
      _Source = "Buffer overrun detected!";
      local_24 = (uint *)0x4ee430;
    }
    else {
      _Source = "Unknown security failure detected!";
      local_24 = (uint *)0x4ee4f0;
    }
    local_28 = 0;
    DVar1 = GetModuleFileNameA((HMODULE)0x0,(LPSTR)local_12c,0x104);
    if (DVar1 == 0) {
      strcpy((char *)local_12c,"<program name unknown>");
    }
    _Dest = local_12c;
    sVar2 = _strlen((char *)local_12c);
    if (0x3c < sVar2 + 0xb) {
      sVar2 = _strlen((char *)local_12c);
      _Dest = (uint *)(&stack0xfffffea3 + sVar2);
      _strncpy((char *)_Dest,"...",3);
    }
    _strlen((char *)_Dest);
    local_1c = &stack0xfffffec8;
    strcpy(&stack0xfffffec8,_Source);
    StrCat_Fast((uint *)&stack0xfffffec8,(uint *)&DAT_004ea1d0);
    StrCat_Fast((uint *)&stack0xfffffec8,(uint *)"Program: ");
    StrCat_Fast((uint *)&stack0xfffffec8,_Dest);
    StrCat_Fast((uint *)&stack0xfffffec8,(uint *)&DAT_004ea1d0);
    StrCat_Fast((uint *)&stack0xfffffec8,local_24);
    ___crtMessageBoxA(&stack0xfffffec8,"Microsoft Visual C++ Runtime Library",0x12010);
  }
  else {
    local_8 = (undefined *)0x0;
    (*DAT_00535644)();
    local_8 = (undefined *)0xffffffff;
  }
                    /* WARNING: Subroutine does not return */
  __exit(3);
}

