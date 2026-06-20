
/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __cdecl CRT_SetCodePage(UINT param_1)

{
  BYTE *pBVar1;
  byte bVar2;
  byte bVar3;
  uint uVar4;
  BOOL BVar5;
  BYTE *pBVar6;
  int iVar7;
  int extraout_ECX;
  undefined4 extraout_ECX_00;
  int iVar8;
  byte *pbVar9;
  byte *pbVar10;
  undefined4 *puVar11;
  uint unaff_retaddr;
  _cpinfo local_20;
  uint local_c;
  uint local_8;
  
  local_8 = DAT_004fce90 ^ unaff_retaddr;
  if (param_1 != 0) {
    iVar8 = 0;
    uVar4 = 0;
    do {
      if (*(UINT *)((int)&DAT_004fceb0 + uVar4) == param_1) {
        puVar11 = (undefined4 *)&DAT_00535760;
        for (iVar7 = 0x40; iVar7 != 0; iVar7 = iVar7 + -1) {
          *puVar11 = 0;
          puVar11 = puVar11 + 1;
        }
        local_c = 0;
        *(undefined1 *)puVar11 = 0;
        pbVar9 = (byte *)(iVar8 * 0x30 + 0x4fcec0);
        do {
          bVar3 = *pbVar9;
          pbVar10 = pbVar9;
          while ((bVar3 != 0 && (bVar2 = pbVar10[1], bVar2 != 0))) {
            uVar4 = (uint)bVar3;
            if (uVar4 <= bVar2) {
              bVar3 = (&DAT_004fcea8)[local_c];
              do {
                (&DAT_00535761)[uVar4] = (&DAT_00535761)[uVar4] | bVar3;
                uVar4 = uVar4 + 1;
              } while (uVar4 <= bVar2);
            }
            pbVar10 = pbVar10 + 2;
            bVar3 = *pbVar10;
          }
          local_c = local_c + 1;
          pbVar9 = pbVar9 + 8;
        } while (local_c < 4);
        DAT_00535864 = param_1;
        DAT_00535758 = 1;
        DAT_00535750 = CRT_MapCodePageToLocale();
        _DAT_00535870 = *(undefined4 *)(&DAT_004fceb4 + extraout_ECX);
        DAT_00535874 = *(undefined4 *)(&DAT_004fceb8 + extraout_ECX);
        DAT_00535878 = *(undefined4 *)(&DAT_004fcebc + extraout_ECX);
        goto LAB_004c3e5e;
      }
      uVar4 = uVar4 + 0x30;
      iVar8 = iVar8 + 1;
    } while (uVar4 < 0xf0);
    BVar5 = GetCPInfo(param_1,&local_20);
    if (BVar5 == 1) {
      puVar11 = (undefined4 *)&DAT_00535760;
      for (iVar8 = 0x40; iVar8 != 0; iVar8 = iVar8 + -1) {
        *puVar11 = 0;
        puVar11 = puVar11 + 1;
      }
      *(undefined1 *)puVar11 = 0;
      DAT_00535864 = param_1;
      DAT_00535750 = 0;
      if (local_20.MaxCharSize < 2) {
        DAT_00535758 = 0;
      }
      else {
        if (local_20.LeadByte[0] != '\0') {
          pBVar6 = local_20.LeadByte + 1;
          do {
            bVar3 = *pBVar6;
            if (bVar3 == 0) break;
            for (uVar4 = (uint)pBVar6[-1]; uVar4 <= bVar3; uVar4 = uVar4 + 1) {
              (&DAT_00535761)[uVar4] = (&DAT_00535761)[uVar4] | 4;
            }
            pBVar1 = pBVar6 + 1;
            pBVar6 = pBVar6 + 2;
          } while (*pBVar1 != 0);
        }
        uVar4 = 1;
        do {
          (&DAT_00535761)[uVar4] = (&DAT_00535761)[uVar4] | 8;
          uVar4 = uVar4 + 1;
        } while (uVar4 < 0xff);
        DAT_00535750 = CRT_MapCodePageToLocale();
        DAT_00535758 = extraout_ECX_00;
      }
      _DAT_00535870 = 0;
      DAT_00535874 = 0;
      DAT_00535878 = 0;
      goto LAB_004c3e5e;
    }
    if (DAT_005355ac == 0) {
      return 0xffffffff;
    }
  }
  setSBCS();
LAB_004c3e5e:
  CRT_BuildCaseMapTables();
  return 0;
}

