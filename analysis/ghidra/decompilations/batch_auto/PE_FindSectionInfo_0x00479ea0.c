
undefined4 __cdecl
PE_FindSectionInfo(LPCVOID param_1,LPSTR param_2,DWORD param_3,int *param_4,int *param_5)

{
  int iVar1;
  uint uVar2;
  SIZE_T SVar3;
  DWORD DVar4;
  uint *puVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  _MEMORY_BASIC_INFORMATION local_1c;
  
  SVar3 = VirtualQuery(param_1,&local_1c,0x1c);
  if (SVar3 == 0) {
    return 0;
  }
  DVar4 = GetModuleFileNameA(local_1c.AllocationBase,param_2,param_3);
  puVar5 = (uint *)0x0;
  if (DVar4 != 0) {
    iVar1 = ((HMODULE)((int)local_1c.AllocationBase + 0x3c))->unused;
    uVar8 = (uint)*(ushort *)
                   ((int)&((HMODULE)((int)local_1c.AllocationBase + 4))->unused + iVar1 + 2);
    puVar5 = (uint *)((int)local_1c.AllocationBase + iVar1);
    uVar9 = (int)param_1 - (int)local_1c.AllocationBase;
    uVar7 = 0;
    if (uVar8 != 0) {
      puVar5 = (uint *)((int)puVar5 +
                       *(ushort *)
                        ((int)&((HMODULE)((int)local_1c.AllocationBase + 0x14))->unused + iVar1) +
                       0x28);
      do {
        uVar2 = puVar5[-1];
        uVar6 = *puVar5;
        if (*puVar5 <= puVar5[-2]) {
          uVar6 = puVar5[-2];
        }
        if ((uVar2 <= uVar9) && (uVar9 <= uVar6 + uVar2)) {
          *param_4 = uVar7 + 1;
          *param_5 = uVar9 - uVar2;
          return CONCAT31((int3)((uint)param_5 >> 8),1);
        }
        uVar7 = uVar7 + 1;
        puVar5 = puVar5 + 10;
      } while (uVar7 < uVar8);
    }
  }
  return (uint)puVar5 & 0xffffff00;
}

