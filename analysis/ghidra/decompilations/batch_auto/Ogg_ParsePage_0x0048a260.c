
/* WARNING: Removing unreachable block (ram,0x0048a2f7) */

int __cdecl Ogg_ParsePage(int param_1,undefined4 *param_2)

{
  int iVar1;
  undefined4 *puVar2;
  char cVar3;
  byte bVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  void *pvVar8;
  int iVar9;
  int *piVar10;
  undefined4 extraout_ECX;
  int local_18;
  undefined4 *local_10;
  char *local_c;
  
  if (param_2 != (undefined4 *)0x0) {
    Pool_FreeList((int *)*param_2);
    Pool_FreeList((int *)param_2[2]);
    *param_2 = 0;
    param_2[1] = 0;
    param_2[2] = 0;
    param_2[3] = 0;
  }
  puVar2 = *(undefined4 **)(param_1 + 8);
  local_10 = (undefined4 *)0x0;
  piVar10 = (int *)(param_1 + 8);
  local_c = (char *)0x0;
  if (puVar2 != (undefined4 *)0x0) {
    local_c = (char *)(puVar2[1] + *(int *)*puVar2);
    local_10 = puVar2;
  }
  if (*(int *)(param_1 + 0x14) == 0) {
    if (*(int *)(param_1 + 0xc) < 0x1b) {
      return 0;
    }
    AthenaList_IterateNext();
    if ((((*local_c != 'O') || (cVar3 = AthenaList_ReadByte(1), cVar3 != 'g')) ||
        (cVar3 = AthenaList_ReadByte(2), cVar3 != 'g')) ||
       (cVar3 = AthenaList_ReadByte(3), cVar3 != 'S')) goto LAB_0048a45e;
    bVar4 = AthenaList_ReadByte(0x1a);
    *(uint *)(param_1 + 0x14) = bVar4 + 0x1b;
  }
  iVar7 = *(int *)(param_1 + 0x14);
  if (*(int *)(param_1 + 0xc) < iVar7) {
    return 0;
  }
  if ((*(int *)(param_1 + 0x18) == 0) && (iVar7 != 0x1b && -1 < iVar7 + -0x1b)) {
    iVar9 = 0x1b;
    do {
      if (iVar9 < 0) {
        local_c = (char *)(local_10[1] + *(int *)*local_10);
      }
      AthenaList_IterateNext();
      *(int *)(param_1 + 0x18) = *(int *)(param_1 + 0x18) + (uint)(byte)local_c[iVar9];
      iVar1 = iVar9 + -0x1a;
      iVar9 = iVar9 + 1;
    } while (iVar1 < *(int *)(param_1 + 0x14) + -0x1b);
  }
  if (*(int *)(param_1 + 0xc) < *(int *)(param_1 + 0x18) + iVar7) {
    return 0;
  }
  uVar5 = AthenaList_ReadDword(0x16);
  AthenaList_WriteDword(0x16,0);
  uVar6 = StreamWriter_FlushToCallback
                    (extraout_ECX,*piVar10,*(int *)(param_1 + 0x18) + *(int *)(param_1 + 0x14));
  if (uVar5 == uVar6) {
    AthenaList_WriteDword(0x16,uVar5);
    if (param_2 == (undefined4 *)0x0) {
      iVar7 = D3DX_SurfaceClipBlit(*(int *)(param_1 + 0x18) + *(int *)(param_1 + 0x14));
      *piVar10 = iVar7;
      if (iVar7 == 0) {
        *(undefined4 *)(param_1 + 4) = 0;
      }
    }
    else {
      piVar10 = AthenaList_SplitChunk((int *)(param_1 + 4),*(int *)(param_1 + 0x14));
      *param_2 = piVar10;
      param_2[1] = *(undefined4 *)(param_1 + 0x14);
      piVar10 = AthenaList_SplitChunk((int *)(param_1 + 4),*(int *)(param_1 + 0x18));
      param_2[2] = piVar10;
      param_2[3] = *(undefined4 *)(param_1 + 0x18);
    }
    iVar7 = *(int *)(param_1 + 0x18);
    iVar9 = *(int *)(param_1 + 0x14);
    *(undefined4 *)(param_1 + 0x10) = 0;
    *(undefined4 *)(param_1 + 0x14) = 0;
    *(undefined4 *)(param_1 + 0x18) = 0;
    iVar7 = iVar7 + iVar9;
    *(int *)(param_1 + 0xc) = *(int *)(param_1 + 0xc) - iVar7;
    return iVar7;
  }
  AthenaList_WriteDword(0x16,uVar5);
LAB_0048a45e:
  *(undefined4 *)(param_1 + 0x14) = 0;
  *(undefined4 *)(param_1 + 0x18) = 0;
  iVar7 = D3DX_SurfaceClipBlit(1);
  local_18 = -1;
  *piVar10 = iVar7;
  do {
    if (iVar7 == 0) {
LAB_0048a4c7:
      if (*piVar10 == 0) {
        *(undefined4 *)(param_1 + 4) = 0;
      }
      *(int *)(param_1 + 0xc) = *(int *)(param_1 + 0xc) + local_18;
      return local_18;
    }
    puVar2 = (undefined4 *)*piVar10;
    iVar7 = *(int *)*puVar2;
    iVar9 = puVar2[1];
    pvVar8 = _memchr((void *)(iVar9 + iVar7),0x4f,puVar2[2]);
    if (pvVar8 != (void *)0x0) {
      iVar7 = (int)pvVar8 - (iVar9 + iVar7);
      iVar9 = D3DX_SurfaceClipBlit(iVar7);
      *piVar10 = iVar9;
      local_18 = local_18 - iVar7;
      goto LAB_0048a4c7;
    }
    local_18 = local_18 - *(int *)(*piVar10 + 8);
    iVar7 = D3DX_SurfaceClipBlit(*(int *)(*piVar10 + 8));
    *piVar10 = iVar7;
  } while( true );
}

