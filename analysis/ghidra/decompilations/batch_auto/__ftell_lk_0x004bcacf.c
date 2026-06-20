
/* Library Function - Single Match
    __ftell_lk
   
   Library: Visual Studio 2003 Release */

int __cdecl __ftell_lk(uint *param_1)

{
  uint _FileHandle;
  uint uVar1;
  byte bVar2;
  int *piVar3;
  uint *puVar4;
  long lVar5;
  char *pcVar6;
  uint *puVar7;
  char *pcVar8;
  char *pcVar9;
  int iVar10;
  int local_c;
  int local_8;
  
  puVar7 = param_1;
  _FileHandle = param_1[4];
  if ((int)param_1[1] < 0) {
    param_1[1] = 0;
  }
  local_8 = __lseek(_FileHandle,0,1);
  if (local_8 < 0) {
LAB_004bcb6c:
    local_c = -1;
  }
  else {
    uVar1 = param_1[3];
    if ((uVar1 & 0x108) == 0) {
      return local_8 - param_1[1];
    }
    pcVar6 = (char *)*param_1;
    pcVar9 = (char *)param_1[2];
    local_c = (int)pcVar6 - (int)pcVar9;
    if ((uVar1 & 3) == 0) {
      if (-1 < (char)uVar1) {
        piVar3 = __errno();
        *piVar3 = 0x16;
        goto LAB_004bcb6c;
      }
    }
    else if (((*(byte *)((&DAT_005369e0)[(int)_FileHandle >> 5] + 4 + (_FileHandle & 0x1f) * 0x24) &
              0x80) != 0) && (pcVar8 = pcVar9, pcVar9 < pcVar6)) {
      do {
        if (*pcVar8 == '\n') {
          local_c = local_c + 1;
        }
        pcVar8 = pcVar8 + 1;
      } while (pcVar8 < (char *)*param_1);
    }
    if (local_8 != 0) {
      if ((param_1[3] & 1) != 0) {
        if (param_1[1] == 0) {
          local_c = 0;
        }
        else {
          puVar4 = (uint *)(pcVar6 + (param_1[1] - (int)pcVar9));
          iVar10 = (_FileHandle & 0x1f) * 0x24;
          if ((*(byte *)(iVar10 + 4 + (&DAT_005369e0)[(int)_FileHandle >> 5]) & 0x80) != 0) {
            lVar5 = __lseek(_FileHandle,0,2);
            if (lVar5 == local_8) {
              pcVar6 = (char *)param_1[2];
              pcVar9 = (char *)((int)puVar4 + (int)pcVar6);
              param_1 = puVar4;
              for (; pcVar6 < pcVar9; pcVar6 = pcVar6 + 1) {
                if (*pcVar6 == '\n') {
                  param_1 = (uint *)((int)param_1 + 1);
                }
              }
              bVar2 = *(byte *)((int)puVar7 + 0xd) & 0x20;
            }
            else {
              __lseek(_FileHandle,local_8,0);
              puVar7 = (uint *)&DAT_00000200;
              if (((&DAT_00000200 < puVar4) || ((param_1[3] & 8) == 0)) ||
                 ((param_1[3] & 0x400) != 0)) {
                puVar7 = (uint *)param_1[6];
              }
              bVar2 = *(byte *)(iVar10 + 4 + (&DAT_005369e0)[(int)_FileHandle >> 5]) & 4;
              param_1 = puVar7;
            }
            puVar4 = param_1;
            if (bVar2 != 0) {
              puVar4 = (uint *)((int)param_1 + 1);
            }
          }
          param_1 = puVar4;
          local_8 = local_8 - (int)param_1;
        }
      }
      local_c = local_c + local_8;
    }
  }
  return local_c;
}

