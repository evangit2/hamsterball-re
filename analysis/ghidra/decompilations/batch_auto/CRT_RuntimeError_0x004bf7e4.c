
/* WARNING: Function: __chkstk replaced with injection: alloca_probe */
/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */
/* WARNING: Unable to track spacebase fully for stack */

void __cdecl CRT_RuntimeError(DWORD param_1)

{
  int iVar1;
  uint uVar2;
  DWORD DVar3;
  size_t sVar4;
  size_t sVar5;
  HANDLE hFile;
  CHAR *pCVar6;
  CHAR *pCVar7;
  int iVar8;
  CHAR *_Dest;
  uint unaff_retaddr;
  undefined4 uStackY_14c;
  UINT aUStackY_148 [3];
  undefined4 auStackY_13c [2];
  undefined4 uStackY_134;
  LPCVOID lpBuffer;
  DWORD *lpNumberOfBytesWritten;
  LPOVERLAPPED lpOverlapped;
  CHAR local_110 [260];
  undefined1 local_c;
  uint local_8;
  
  local_8 = DAT_004fce90 ^ unaff_retaddr;
  uVar2 = 0;
  do {
    if (param_1 == (&DAT_004fc848)[uVar2 * 2]) break;
    uVar2 = uVar2 + 1;
  } while (uVar2 < 0x12);
  iVar8 = uVar2 * 8;
  if (param_1 == (&DAT_004fc848)[uVar2 * 2]) {
    if ((DAT_005352c8 == 1) || ((DAT_005352c8 == 0 && (DAT_004fc488 == 1)))) {
      lpOverlapped = (LPOVERLAPPED)0x0;
      lpNumberOfBytesWritten = &param_1;
      sVar4 = _strlen(*(char **)(iVar8 + 0x4fc84c));
      lpBuffer = *(LPCVOID *)(iVar8 + 0x4fc84c);
      uStackY_134 = 0x4bf93e;
      hFile = GetStdHandle(0xfffffff4);
      uStackY_134 = 0x4bf945;
      WriteFile(hFile,lpBuffer,sVar4,lpNumberOfBytesWritten,lpOverlapped);
    }
    else if (param_1 != 0xfc) {
      local_c = 0;
      DVar3 = GetModuleFileNameA((HMODULE)0x0,local_110,0x104);
      if (DVar3 == 0) {
        strcpy(local_110,"<program name unknown>");
      }
      _Dest = local_110;
      sVar4 = _strlen(local_110);
      if (0x3c < sVar4 + 1) {
        sVar4 = _strlen(local_110);
        _Dest = (CHAR *)((int)&uStackY_14c + sVar4 + 1);
        _strncpy(_Dest,"...",3);
      }
      sVar4 = _strlen(_Dest);
      sVar5 = _strlen(*(char **)(iVar8 + 0x4fc84c));
      iVar1 = -(sVar4 + sVar5 + 0x1f & 0xfffffffc);
      *(char **)(local_110 + iVar1 + -0x10) = "Runtime Error!\n\nProgram: ";
      *(CHAR **)(local_110 + iVar1 + -0x14) = local_110 + iVar1 + -0xc;
      pCVar6 = local_110 + iVar1 + -0x18;
      pCVar6[0] = -0x13;
      pCVar6[1] = -8;
      pCVar6[2] = 'K';
      pCVar6[3] = '\0';
      strcpy(*(char **)(local_110 + iVar1 + -0x14),*(char **)(local_110 + iVar1 + -0x10));
      *(CHAR **)(local_110 + iVar1 + -0x18) = _Dest;
      *(CHAR **)(local_110 + iVar1 + -0x1c) = local_110 + iVar1 + -0xc;
      pCVar7 = local_110 + iVar1 + -0x20;
      pCVar7[0] = -0xc;
      pCVar7[1] = -8;
      pCVar7[2] = 'K';
      pCVar7[3] = '\0';
      StrCat_Fast(*(uint **)(local_110 + iVar1 + -0x1c),*(uint **)(local_110 + iVar1 + -0x18));
      *(undefined **)(local_110 + iVar1 + -0x20) = &DAT_004ea1d0;
      *(CHAR **)((int)&uStackY_134 + iVar1) = local_110 + iVar1 + -0xc;
      *(undefined4 *)((int)auStackY_13c + iVar1 + 4) = 0x4bf8ff;
      StrCat_Fast(*(uint **)((int)&uStackY_134 + iVar1),*(uint **)(local_110 + iVar1 + -0x20));
      *(undefined4 *)((int)auStackY_13c + iVar1 + 4) = *(undefined4 *)(iVar8 + 0x4fc84c);
      *(CHAR **)((int)auStackY_13c + iVar1) = local_110 + iVar1 + -0xc;
      *(undefined4 *)((int)aUStackY_148 + iVar1 + 8) = 0x4bf90b;
      StrCat_Fast(*(uint **)((int)auStackY_13c + iVar1),*(uint **)((int)auStackY_13c + iVar1 + 4));
      *(undefined4 *)((int)aUStackY_148 + iVar1 + 8) = 0x12010;
      *(char **)((int)aUStackY_148 + iVar1 + 4) = "Microsoft Visual C++ Runtime Library";
      *(CHAR **)((int)aUStackY_148 + iVar1) = local_110 + iVar1 + -0xc;
      *(undefined4 *)((int)&uStackY_14c + iVar1) = 0x4bf91b;
      ___crtMessageBoxA(*(LPCSTR *)((int)aUStackY_148 + iVar1),
                        *(LPCSTR *)((int)aUStackY_148 + iVar1 + 4),
                        *(UINT *)((int)aUStackY_148 + iVar1 + 8));
    }
  }
  return;
}

