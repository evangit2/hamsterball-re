
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Ball_Update(int param_1)

{
  int iVar1;
  float fVar2;
  float fVar3;
  bool bVar4;
  int iVar5;
  char *pcVar6;
  uint *puVar7;
  void *_Memory;
  int iVar8;
  undefined4 *puVar9;
  int iVar10;
  float *pfVar11;
  float local_b0;
  float local_ac;
  float local_a8;
  undefined4 local_a4;
  char cVar12;
  undefined4 *local_84;
  float local_80;
  float local_7c;
  float local_78;
  char local_71;
  int local_70;
  int local_6c;
  float local_68;
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  float local_40;
  float local_3c;
  float local_38;
  int local_34;
  float local_30;
  float local_28;
  undefined4 local_24 [2];
  float local_1c;
  float local_10;
  undefined1 local_c [12];
  
  if (*(char *)(param_1 + 0x324) != '\0') {
    return;
  }
  if (*(char *)(*(int *)(*(int *)(param_1 + 0x14) + 0x878) + 0x237) != '\0') {
    Ball_ResetCollisionMesh(param_1);
  }
  *(undefined1 *)(param_1 + 0x2f8) = 1;
  *(undefined4 *)(param_1 + 0x2ec) = 0;
  *(undefined4 *)(param_1 + 0x150) = 0;
  if (*(int *)(param_1 + 0x18) != -1) {
    iVar8 = *(int *)(param_1 + 0x18) * 0xa0;
    *(int *)(iVar8 + 0x5f4 + *(int *)(param_1 + 0x10)) =
         *(int *)(iVar8 + 0x5f4 + *(int *)(param_1 + 0x10)) + 1;
  }
  if (*(void **)(param_1 + 0xc28) != (void *)0x0) {
    _free(*(void **)(param_1 + 0xc28));
    *(undefined4 *)(param_1 + 0xc28) = 0;
  }
  iVar8 = *(int *)(param_1 + 0x1a4);
  *(undefined1 *)(param_1 + 0x310) = 1;
  *(undefined4 *)(param_1 + 0x29c) = 0x3f800000;
  if ((undefined4 *)(iVar8 + 0xca4) != local_24) {
    *(undefined4 *)(iVar8 + 0xca4) = 0;
    *(undefined4 *)(iVar8 + 0xca8) = 0;
    *(undefined4 *)(iVar8 + 0xcac) = 0;
  }
  *(undefined4 *)(*(int *)(param_1 + 0x1a4) + 0xc74) = 0;
  *(undefined1 *)(param_1 + 0x768) = 1;
  *(undefined1 *)(param_1 + 0x2e8) = 0;
  *(undefined1 *)(param_1 + 0x2e9) = 0;
  *(undefined1 *)(param_1 + 700) = 0;
  *(undefined1 *)(param_1 + 0x2cc) = 0;
  iVar8 = AthenaList_GetIndex(param_1 + 0x810);
  *(undefined4 *)(param_1 + 0x818 + iVar8 * 4) = 0;
  if (*(int *)(param_1 + 0x814) < 1) {
    _Memory = (void *)0x0;
  }
  else {
    _Memory = (void *)**(undefined4 **)(param_1 + 0xc1c);
    *(undefined4 *)(param_1 + 0x818 + iVar8 * 4) = 1;
  }
  while (_Memory != (void *)0x0) {
    _free(_Memory);
    iVar5 = *(int *)(param_1 + 0x818 + iVar8 * 4);
    if (*(int *)(param_1 + 0x814) <= iVar5) break;
    _Memory = *(void **)(*(int *)(param_1 + 0xc1c) + iVar5 * 4);
    *(int *)(param_1 + 0x818 + iVar8 * 4) = iVar5 + 1;
  }
  AthenaList_Clear(param_1 + 0x810);
  iVar8 = *(int *)(param_1 + 0x748);
  local_84 = (undefined4 *)0x0;
  local_68 = 99999.0;
  local_70 = 0;
  if (iVar8 == 0) {
    do {
      local_68 = 99999.0;
      if (*(char *)(*(int *)(param_1 + 0x10) + 0x237) == '\0') {
        iVar5 = AthenaList_GetIndex(*(int *)(param_1 + 0x14) + 0x1518);
        iVar8 = *(int *)(param_1 + 0x14);
        *(undefined4 *)(iVar8 + 0x1520 + iVar5 * 4) = 0;
        if (*(int *)(iVar8 + 0x151c) < 1) {
          puVar9 = (undefined4 *)0x0;
        }
        else {
          puVar9 = (undefined4 *)**(undefined4 **)(iVar8 + 0x1924);
          *(undefined4 *)(iVar8 + 0x1520 + iVar5 * 4) = 1;
        }
        local_6c = iVar5;
        if (puVar9 != (undefined4 *)0x0) {
          do {
            pcVar6 = (char *)(param_1 + 0xc2c);
            bVar4 = false;
            do {
              cVar12 = *pcVar6;
              pcVar6 = pcVar6 + 1;
            } while (cVar12 != '\0');
            local_34 = (int)pcVar6 - (param_1 + 0xc2d);
            if (local_34 == 0) {
LAB_00405896:
              bVar4 = true;
              puVar7 = FUN_004bac20((uint *)*puVar9,"[Z]");
              if ((puVar7 != (uint *)0x0) ||
                 (puVar7 = FUN_004bac20((uint *)*puVar9,"[X]"), puVar7 != (uint *)0x0)) {
                bVar4 = false;
              }
            }
            else {
              puVar7 = FUN_004bacc0((uint *)*puVar9,'(');
              if (puVar7 != (uint *)0x0) {
                local_a4 = 0x40588f;
                iVar8 = __strnicmp((char *)puVar7,(char *)(param_1 + 0xc2c),2);
                if (iVar8 == 0) goto LAB_00405896;
              }
            }
            iVar8 = *(int *)(param_1 + 0x10);
            if ((*(char *)(iVar8 + 0x234) == '\0') || (*(int *)(param_1 + 0x18) == -1)) {
LAB_0040599c:
              if ((bVar4) &&
                 ((fVar2 = *(float *)(param_1 + 0x2e0) -
                           (*(float *)(param_1 + 0x284) + _DAT_004cf48c),
                  fVar2 < (float)puVar9[2] != (fVar2 == (float)puVar9[2]) || (1 < local_70)))) {
                local_1c = *(float *)(param_1 + 0x2e4);
                local_3c = (float)puVar9[2];
                local_38 = (float)puVar9[3];
                local_30 = *(float *)(param_1 + 0x2dc) - (float)puVar9[1];
                fVar2 = *(float *)(param_1 + 0x2e0) - local_3c;
                fVar2 = SQRT(local_30 * local_30 +
                             fVar2 * fVar2 + (local_1c - local_38) * (local_1c - local_38));
                if ((fVar2 < local_68) &&
                   (local_84 = puVar9, local_68 = fVar2, *(char *)(iVar8 + 0x237) != '\0')) break;
              }
            }
            else {
              if (&local_80 != (float *)(puVar9 + 1)) {
                local_7c = (float)puVar9[2];
                local_80 = (float)puVar9[1];
                local_78 = (float)puVar9[3];
              }
              iVar5 = *(int *)((1 - *(int *)(param_1 + 0x18)) * 0xa0 + 0x5dc + iVar8);
              local_10 = *(float *)(iVar5 + 0x16c);
              local_80 = local_80 - *(float *)(iVar5 + 0x164);
              local_7c = local_7c - *(float *)(iVar5 + 0x168);
              local_78 = local_78 - local_10;
              fVar2 = local_80 * local_80 + local_7c * local_7c + local_78 * local_78;
              fVar3 = _DAT_004cf368;
              if (fVar2 < _DAT_004cf368 == (fVar2 == _DAT_004cf368)) {
                fVar3 = SQRT(fVar2);
              }
              iVar5 = local_6c;
              if (fVar3 < *(float *)(param_1 + 0x284) == (fVar3 == *(float *)(param_1 + 0x284)))
              goto LAB_0040599c;
            }
            iVar8 = *(int *)(param_1 + 0x14);
            iVar10 = *(int *)(iVar8 + 0x1520 + iVar5 * 4);
            if (*(int *)(iVar8 + 0x151c) <= iVar10) break;
            puVar9 = *(undefined4 **)(*(int *)(iVar8 + 0x1924) + iVar10 * 4);
            *(int *)(iVar8 + 0x1520 + iVar5 * 4) = iVar10 + 1;
          } while (puVar9 != (undefined4 *)0x0);
          goto LAB_00405ccc;
        }
      }
      else {
        do {
          cVar12 = '\0';
          iVar8 = FUN_004536a0(*(int *)(param_1 + 0x14) + 0x1518);
          iVar5 = FUN_0045dd60(&PTR_PTR_004f7360,iVar8,cVar12);
          iVar8 = *(int *)(param_1 + 0x14);
          if (iVar5 < 0) {
            local_84 = (undefined4 *)0x0;
          }
          else if (iVar5 < *(int *)(iVar8 + 0x151c)) {
            local_84 = *(undefined4 **)(*(int *)(iVar8 + 0x1924) + iVar5 * 4);
          }
          else {
            local_84 = (undefined4 *)0x0;
          }
          local_71 = '\x01';
          if ((*(char *)(*(int *)(param_1 + 0x10) + 0x234) != '\0') &&
             (*(int *)(param_1 + 0x18) != -1)) {
            iVar5 = AthenaList_GetIndex(iVar8 + 0x29d4);
            iVar8 = *(int *)(param_1 + 0x14);
            *(undefined4 *)(iVar8 + 0x29dc + iVar5 * 4) = 0;
            if (*(int *)(iVar8 + 0x29d8) < 1) {
              iVar10 = 0;
            }
            else {
              iVar10 = **(int **)(iVar8 + 0x2de0);
              *(undefined4 *)(iVar8 + 0x29dc + iVar5 * 4) = 1;
            }
            if (iVar10 != 0) {
              do {
                if (&local_58 != (float *)(local_84 + 1)) {
                  local_58 = (float)local_84[1];
                  local_54 = (float)local_84[2];
                  local_50 = (float)local_84[3];
                }
                local_58 = local_58 - *(float *)(iVar10 + 0x164);
                local_54 = local_54 - *(float *)(iVar10 + 0x168);
                local_50 = local_50 - *(float *)(iVar10 + 0x16c);
                fVar2 = local_58 * local_58 + local_54 * local_54 + local_50 * local_50;
                fVar3 = _DAT_004cf368;
                if (fVar2 < _DAT_004cf368 == (fVar2 == _DAT_004cf368)) {
                  fVar3 = SQRT(fVar2);
                }
                if (fVar3 < *(float *)(param_1 + 0x284) != (fVar3 == *(float *)(param_1 + 0x284))) {
                  local_71 = '\0';
                }
                iVar8 = *(int *)(param_1 + 0x14);
                iVar1 = *(int *)(iVar8 + 0x29dc + iVar5 * 4);
                if (*(int *)(iVar8 + 0x29d8) <= iVar1) break;
                iVar10 = *(int *)(*(int *)(iVar8 + 0x2de0) + iVar1 * 4);
                *(int *)(iVar8 + 0x29dc + iVar5 * 4) = iVar1 + 1;
              } while (iVar10 != 0);
            }
          }
          pfVar11 = (float *)(local_84 + 1);
          if (&local_64 != pfVar11) {
            local_64 = *pfVar11;
            local_60 = (float)local_84[2];
            local_5c = (float)local_84[3];
          }
          if (&local_a4 != (undefined4 *)local_c) {
            local_a4 = 0;
          }
          if (&local_b0 != &local_64) {
            local_b0 = local_64;
            local_ac = local_60;
            local_a8 = local_5c;
          }
          Mesh_FindClosestCollision(*(void **)(*(int *)(param_1 + 0x14) + 0x8b0),&local_4c);
          local_4c = local_4c - *pfVar11;
          local_48 = local_48 - (float)local_84[2];
          local_44 = local_44 - (float)local_84[3];
          fVar2 = local_4c * local_4c + local_48 * local_48 + local_44 * local_44;
        } while ((((fVar2 < _DAT_004cf368 == (fVar2 == _DAT_004cf368)) &&
                  (_DAT_004cf484 < SQRT(fVar2))) || (local_71 == '\0')) ||
                (*(char *)(*(int *)(param_1 + 0x10) + 0x237) == '\0'));
LAB_00405ccc:
        if (local_84 != (undefined4 *)0x0) goto LAB_00405cfa;
      }
      if (1 < local_70) {
        *(undefined1 *)(param_1 + 0xc2c) = 0;
      }
      local_70 = local_70 + 1;
    } while( true );
  }
  if (iVar8 == 1) {
    do {
      iVar5 = AthenaList_GetIndex(*(int *)(param_1 + 0x14) + 0x1518);
      iVar8 = *(int *)(param_1 + 0x14);
      *(undefined4 *)(iVar8 + 0x1520 + iVar5 * 4) = 0;
      if (*(int *)(iVar8 + 0x151c) < 1) {
        puVar9 = (undefined4 *)0x0;
      }
      else {
        puVar9 = (undefined4 *)**(undefined4 **)(iVar8 + 0x1924);
        *(undefined4 *)(iVar8 + 0x1520 + iVar5 * 4) = 1;
      }
      local_6c = iVar5;
      if (puVar9 != (undefined4 *)0x0) {
        do {
          pcVar6 = (char *)(param_1 + 0xc2c);
          bVar4 = false;
          do {
            cVar12 = *pcVar6;
            pcVar6 = pcVar6 + 1;
          } while (cVar12 != '\0');
          local_34 = (int)pcVar6 - (param_1 + 0xc2d);
          if ((local_34 == 0) ||
             ((puVar7 = FUN_004bacc0((uint *)*puVar9,'('), puVar7 != (uint *)0x0 &&
              (iVar8 = __stricmp((char *)puVar7,(char *)(param_1 + 0xc2c)), iVar8 == 0)))) {
            bVar4 = true;
            puVar7 = FUN_004bac20((uint *)*puVar9,"[X]");
            if (puVar7 == (uint *)0x0) {
              bVar4 = false;
            }
          }
          if ((*(char *)(*(int *)(param_1 + 0x10) + 0x234) == '\0') ||
             (*(int *)(param_1 + 0x18) == -1)) {
LAB_004056f0:
            if ((bVar4) &&
               ((fVar2 = *(float *)(param_1 + 0x2dc) - (*(float *)(param_1 + 0x284) + _DAT_004cf48c)
                , fVar2 < (float)puVar9[1] != (fVar2 == (float)puVar9[1]) || (3 < local_70)))) {
              local_50 = *(float *)(param_1 + 0x2e4);
              local_60 = (float)puVar9[2];
              local_5c = (float)puVar9[3];
              local_40 = *(float *)(param_1 + 0x2dc) - (float)puVar9[1];
              fVar2 = *(float *)(param_1 + 0x2e0) - local_60;
              fVar2 = SQRT(local_40 * local_40 +
                           fVar2 * fVar2 + (local_50 - local_5c) * (local_50 - local_5c));
              if (fVar2 < local_68) {
                local_84 = puVar9;
                local_68 = fVar2;
              }
            }
          }
          else {
            if (&local_80 != (float *)(puVar9 + 1)) {
              local_7c = (float)puVar9[2];
              local_80 = (float)puVar9[1];
              local_78 = (float)puVar9[3];
            }
            iVar8 = *(int *)((1 - *(int *)(param_1 + 0x18)) * 0xa0 + 0x5dc +
                            *(int *)(param_1 + 0x10));
            local_28 = *(float *)(iVar8 + 0x16c);
            local_80 = local_80 - *(float *)(iVar8 + 0x164);
            local_7c = local_7c - *(float *)(iVar8 + 0x168);
            local_78 = local_78 - local_28;
            fVar2 = local_80 * local_80 + local_7c * local_7c + local_78 * local_78;
            fVar3 = _DAT_004cf368;
            if (fVar2 < _DAT_004cf368 == (fVar2 == _DAT_004cf368)) {
              fVar3 = SQRT(fVar2);
            }
            iVar5 = local_6c;
            if (fVar3 < *(float *)(param_1 + 0x284) == (fVar3 == *(float *)(param_1 + 0x284)))
            goto LAB_004056f0;
          }
          iVar8 = *(int *)(param_1 + 0x14);
          iVar10 = *(int *)(iVar8 + 0x1520 + iVar5 * 4);
          if (*(int *)(iVar8 + 0x151c) <= iVar10) break;
          puVar9 = *(undefined4 **)(*(int *)(iVar8 + 0x1924) + iVar10 * 4);
          *(int *)(iVar8 + 0x1520 + iVar5 * 4) = iVar10 + 1;
        } while (puVar9 != (undefined4 *)0x0);
        if (local_84 != (undefined4 *)0x0) goto LAB_004057db;
      }
      local_70 = local_70 + 1;
      *(undefined1 *)(param_1 + 0xc2c) = 0;
    } while( true );
  }
  if (iVar8 == 2) {
    do {
      iVar5 = AthenaList_GetIndex(*(int *)(param_1 + 0x14) + 0x1518);
      iVar8 = *(int *)(param_1 + 0x14);
      *(undefined4 *)(iVar8 + 0x1520 + iVar5 * 4) = 0;
      if (*(int *)(iVar8 + 0x151c) < 1) {
        puVar9 = (undefined4 *)0x0;
      }
      else {
        puVar9 = (undefined4 *)**(undefined4 **)(iVar8 + 0x1924);
        *(undefined4 *)(iVar8 + 0x1520 + iVar5 * 4) = 1;
      }
      local_6c = iVar5;
      if (puVar9 != (undefined4 *)0x0) {
        do {
          pcVar6 = (char *)(param_1 + 0xc2c);
          bVar4 = false;
          do {
            cVar12 = *pcVar6;
            pcVar6 = pcVar6 + 1;
          } while (cVar12 != '\0');
          local_34 = (int)pcVar6 - (param_1 + 0xc2d);
          if ((local_34 == 0) ||
             ((puVar7 = FUN_004bacc0((uint *)*puVar9,'('), puVar7 != (uint *)0x0 &&
              (iVar8 = __stricmp((char *)puVar7,(char *)(param_1 + 0xc2c)), iVar8 == 0)))) {
            bVar4 = true;
            puVar7 = FUN_004bac20((uint *)*puVar9,"[Z]");
            if (puVar7 == (uint *)0x0) {
              bVar4 = false;
            }
          }
          if ((*(char *)(*(int *)(param_1 + 0x10) + 0x234) == '\0') ||
             (*(int *)(param_1 + 0x18) == -1)) {
LAB_00405470:
            if ((bVar4) &&
               (((float)puVar9[3] <=
                 *(float *)(param_1 + 0x284) + _DAT_004cf48c + *(float *)(param_1 + 0x2e4) ||
                (3 < local_70)))) {
              local_50 = *(float *)(param_1 + 0x2e4);
              local_60 = (float)puVar9[2];
              local_5c = (float)puVar9[3];
              local_40 = *(float *)(param_1 + 0x2dc) - (float)puVar9[1];
              fVar2 = *(float *)(param_1 + 0x2e0) - local_60;
              fVar2 = SQRT(local_40 * local_40 +
                           fVar2 * fVar2 + (local_50 - local_5c) * (local_50 - local_5c));
              if (fVar2 < local_68) {
                local_84 = puVar9;
                local_68 = fVar2;
              }
            }
          }
          else {
            if (&local_80 != (float *)(puVar9 + 1)) {
              local_7c = (float)puVar9[2];
              local_80 = (float)puVar9[1];
              local_78 = (float)puVar9[3];
            }
            iVar8 = *(int *)((1 - *(int *)(param_1 + 0x18)) * 0xa0 + 0x5dc +
                            *(int *)(param_1 + 0x10));
            local_28 = *(float *)(iVar8 + 0x16c);
            local_80 = local_80 - *(float *)(iVar8 + 0x164);
            local_7c = local_7c - *(float *)(iVar8 + 0x168);
            local_78 = local_78 - local_28;
            fVar2 = local_80 * local_80 + local_7c * local_7c + local_78 * local_78;
            fVar3 = _DAT_004cf368;
            if (fVar2 < _DAT_004cf368 == (fVar2 == _DAT_004cf368)) {
              fVar3 = SQRT(fVar2);
            }
            iVar5 = local_6c;
            if (fVar3 < *(float *)(param_1 + 0x284) == (fVar3 == *(float *)(param_1 + 0x284)))
            goto LAB_00405470;
          }
          iVar8 = *(int *)(param_1 + 0x14);
          iVar10 = *(int *)(iVar8 + 0x1520 + iVar5 * 4);
          if (*(int *)(iVar8 + 0x151c) <= iVar10) break;
          puVar9 = *(undefined4 **)(*(int *)(iVar8 + 0x1924) + iVar10 * 4);
          *(int *)(iVar8 + 0x1520 + iVar5 * 4) = iVar10 + 1;
        } while (puVar9 != (undefined4 *)0x0);
        if (local_84 != (undefined4 *)0x0) goto LAB_00405557;
      }
      local_70 = local_70 + 1;
      *(undefined1 *)(param_1 + 0xc2c) = 0;
    } while( true );
  }
  goto LAB_00405d1d;
LAB_00405cfa:
  *(undefined4 *)(param_1 + 0x164) = local_84[1];
  *(float *)(param_1 + 0x168) = *(float *)(param_1 + 0x284) + (float)local_84[2];
  goto LAB_00405d12;
LAB_00405557:
  *(undefined4 *)(param_1 + 0x164) = local_84[1];
  *(undefined4 *)(param_1 + 0x168) = local_84[2];
  *(float *)(param_1 + 0x16c) = (float)local_84[3] - *(float *)(param_1 + 0x284);
  goto LAB_00405d1d;
LAB_004057db:
  *(float *)(param_1 + 0x164) = *(float *)(param_1 + 0x284) + (float)local_84[1];
  *(undefined4 *)(param_1 + 0x168) = local_84[2];
LAB_00405d12:
  *(undefined4 *)(param_1 + 0x16c) = local_84[3];
LAB_00405d1d:
  *(undefined4 *)(param_1 + 0x178) = 0;
  *(undefined4 *)(param_1 + 0x174) = 0;
  *(undefined4 *)(param_1 + 0x170) = 0;
  *(undefined4 *)(param_1 + 0x184) = 0;
  *(undefined4 *)(param_1 + 0x180) = 0;
  *(undefined4 *)(param_1 + 0x17c) = 0;
  *(undefined4 *)(param_1 + 0x314) = 0;
  *(undefined4 *)(param_1 + 0x318) = 0;
  *(undefined1 *)(param_1 + 0x31c) = 0;
  *(undefined1 *)(param_1 + 0x31d) = 0;
  *(undefined1 *)(param_1 + 0x2f9) = 1;
  *(undefined4 *)(param_1 + 0x2fc) = 0;
  *(undefined1 *)(param_1 + 0x76a) = 0;
  *(undefined4 *)(param_1 + 0x300) = 0x96;
  return;
}

