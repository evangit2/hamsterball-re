
void __thiscall Level_LoadMeshes(void *this,int param_1)

{
  char cVar1;
  int iVar2;
  void *pvVar3;
  int iVar4;
  int iVar5;
  char *pcVar6;
  undefined4 *puVar7;
  char *pcVar8;
  undefined4 *puVar9;
  undefined4 *puVar10;
  int iVar11;
  undefined4 *puVar12;
  undefined4 *puVar13;
  int iVar14;
  undefined4 *puVar15;
  bool bVar16;
  undefined4 *local_7c;
  int local_74;
  undefined4 local_60;
  float fStack_5c;
  undefined4 uStack_58;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd211;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  pvVar3 = operator_new(0x488);
  iVar14 = 0;
  local_4 = 0;
  if (pvVar3 == (void *)0x0) {
    pvVar3 = (void *)0x0;
  }
  else {
    pvVar3 = MeshWorld_ctor(pvVar3,*(undefined4 *)((int)this + 4));
  }
  *(void **)((int)this + 8) = pvVar3;
  *(undefined1 *)((int)pvVar3 + 0x459) = 0;
  *(undefined1 *)((int)this + 0xd) = 1;
  local_4 = 0xffffffff;
  Vec6_Copy((void *)(*(int *)((int)this + 8) + 0x45c),(undefined4 *)(*(int *)(param_1 + 8) + 0x45c))
  ;
  *(undefined1 *)(*(int *)((int)this + 8) + 0x459) = 0;
  *(int *)((int)this + 0x47c) = param_1;
  *(undefined4 *)((int)this + 0x434) = *(undefined4 *)(param_1 + 0x434);
  if (*(char *)(param_1 + 0x430) == '\0') {
    iVar5 = AthenaList_NextIndex(*(int *)(param_1 + 8) + 0x2c);
    iVar4 = *(int *)(param_1 + 8);
    *(undefined4 *)(iVar4 + 0x34 + iVar5 * 4) = 0;
    if (0 < *(int *)(iVar4 + 0x30)) {
      iVar14 = **(int **)(iVar4 + 0x438);
      *(undefined4 *)(iVar4 + 0x34 + iVar5 * 4) = 1;
    }
    while (iVar14 != 0) {
      if ((*(char **)(iVar14 + 0x864) == (char *)0x0) ||
         (pcVar6 = strstr(*(char **)(iVar14 + 0x864),"(NOCOLLIDE)"), pcVar6 == (char *)0x0)) {
        puVar7 = operator_new(0x874);
        local_4 = 2;
        if (puVar7 == (undefined4 *)0x0) {
          local_7c = (undefined4 *)0x0;
        }
        else {
          local_7c = CreateMeshBuffer(puVar7);
        }
        *(undefined1 *)(local_7c + 0x217) = 0;
        local_4 = 0xffffffff;
        AthenaList_Append((void *)(*(int *)((int)this + 8) + 0x2c),(int)local_7c);
        pcVar6 = *(char **)(iVar14 + 0x864);
        if (pcVar6 != (char *)0x0) {
          pcVar8 = pcVar6 + 1;
          do {
            cVar1 = *pcVar6;
            pcVar6 = pcVar6 + 1;
          } while (cVar1 != '\0');
          pcVar8 = _malloc((size_t)(pcVar6 + (1 - (int)pcVar8)));
          local_7c[0x219] = pcVar8;
          pcVar6 = *(char **)(iVar14 + 0x864);
          do {
            cVar1 = *pcVar6;
            pcVar6 = pcVar6 + 1;
            *pcVar8 = cVar1;
            pcVar8 = pcVar8 + 1;
          } while (cVar1 != '\0');
          iVar4 = __strnicmp(*(char **)(iVar14 + 0x864),"N:",2);
          if (iVar4 == 0) {
            *(undefined1 *)((int)local_7c + 0x85d) = 1;
          }
          iVar4 = __strnicmp(*(char **)(iVar14 + 0x864),"E:",2);
          if (iVar4 == 0) {
            *(undefined1 *)((int)local_7c + 0x85d) = 1;
            *(undefined1 *)((int)local_7c + 0x863) = 1;
          }
        }
        iVar4 = AthenaList_NextIndex(iVar14 + 0x424);
        *(undefined4 *)(iVar14 + 0x42c + iVar4 * 4) = 0;
        if (*(int *)(iVar14 + 0x428) < 1) {
          iVar11 = 0;
        }
        else {
          iVar11 = **(int **)(iVar14 + 0x830);
          *(undefined4 *)(iVar14 + 0x42c + iVar4 * 4) = 1;
        }
        while (iVar11 != 0) {
          puVar13 = (undefined4 *)
                    (*(int *)(iVar11 + 0xc) * 0x20 +
                    *(int *)(*(int *)(*(int *)(param_1 + 0x47c) + 0x480) + 0x440));
          bVar16 = false;
          local_74 = 0;
          puVar7 = puVar13;
          if (0 < *(int *)(iVar11 + 4)) {
            do {
              puVar15 = puVar7 + 8;
              puVar9 = operator_new(0x60);
              if (bVar16) {
                puVar10 = puVar7 + 0x10;
                puVar12 = puVar15;
              }
              else {
                puVar10 = puVar15;
                puVar12 = puVar7 + 0x10;
              }
              *puVar9 = *puVar13;
              puVar9[1] = puVar7[1];
              puVar9[2] = puVar7[2];
              puVar9[8] = *puVar10;
              puVar9[9] = puVar10[1];
              puVar9[10] = puVar10[2];
              puVar9[0x10] = *puVar12;
              puVar9[0x11] = puVar12[1];
              puVar9[0x12] = puVar12[2];
              puStack_8 = (undefined1 *)puVar9[10];
              bVar16 = bVar16 == false;
              Graphics_InitShaderDispatch();
              puVar9[3] = local_60;
              puVar9[4] = -fStack_5c;
              puVar9[5] = uStack_58;
              puVar9[0xb] = local_60;
              puVar9[0xc] = -fStack_5c;
              puVar9[0xd] = uStack_58;
              puVar9[0x13] = local_60;
              puVar9[0x14] = -fStack_5c;
              puVar9[0x15] = uStack_58;
              AthenaList_Append(local_7c + 3,(int)puVar9);
              puVar13 = puVar13 + 8;
              local_74 = local_74 + 1;
              puVar7 = puVar15;
            } while (local_74 < *(int *)(iVar11 + 4));
          }
          iVar2 = *(int *)(iVar14 + 0x42c + iVar4 * 4);
          if (*(int *)(iVar14 + 0x428) <= iVar2) break;
          iVar11 = *(int *)(*(int *)(iVar14 + 0x830) + iVar2 * 4);
          *(int *)(iVar14 + 0x42c + iVar4 * 4) = iVar2 + 1;
        }
      }
      iVar4 = *(int *)(param_1 + 8);
      iVar11 = *(int *)(iVar4 + 0x34 + iVar5 * 4);
      if (*(int *)(iVar4 + 0x30) <= iVar11) {
        ExceptionList = local_c;
        return;
      }
      iVar14 = *(int *)(*(int *)(iVar4 + 0x438) + iVar11 * 4);
      *(int *)(iVar4 + 0x34 + iVar5 * 4) = iVar11 + 1;
    }
  }
  else {
    *(undefined1 *)((int)this + 0x430) = 1;
    iVar4 = AthenaList_NextIndex(param_1 + 0x18);
    iVar14 = 0;
    *(undefined4 *)(param_1 + 0x20 + iVar4 * 4) = 0;
    if (0 < *(int *)(param_1 + 0x1c)) {
      iVar14 = **(int **)(param_1 + 0x424);
      *(undefined4 *)(param_1 + 0x20 + iVar4 * 4) = 1;
    }
    if (iVar14 != 0) {
      do {
        pvVar3 = operator_new(0x10d0);
        local_4 = 1;
        if (pvVar3 == (void *)0x0) {
          pvVar3 = (void *)0x0;
        }
        else {
          pvVar3 = CollisionLevel_ctorWithLevel(pvVar3,iVar14);
        }
        *(int *)((int)pvVar3 + 0x47c) = param_1;
        local_4 = 0xffffffff;
        AthenaList_Append((void *)((int)this + 0x18),(int)pvVar3);
        iVar5 = *(int *)(param_1 + 0x20 + iVar4 * 4);
        if (*(int *)(param_1 + 0x1c) <= iVar5) {
          ExceptionList = local_c;
          return;
        }
        iVar14 = *(int *)(*(int *)(param_1 + 0x424) + iVar5 * 4);
        *(int *)(param_1 + 0x20 + iVar4 * 4) = iVar5 + 1;
      } while (iVar14 != 0);
    }
  }
  ExceptionList = local_c;
  return;
}

