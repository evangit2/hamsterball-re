
undefined4 __thiscall DDSURFACEDESC_ParsePixelFormat(void *this,uint *param_1,uint param_2)

{
  byte bVar1;
  ushort uVar2;
  uint *puVar3;
  byte bVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  void *pvVar8;
  undefined1 *puVar9;
  uint *puVar10;
  uint uVar11;
  byte *pbVar12;
  byte *pbVar13;
  int iVar14;
  uint uVar15;
  uint *puVar16;
  byte *pbVar17;
  byte *pbVar18;
  uint *puVar19;
  byte *pbVar20;
  byte *pbVar21;
  byte *pbVar22;
  byte *pbVar23;
  int local_1c;
  byte *local_14;
  
  puVar16 = param_1;
  if (param_2 < 0x28) {
    return 0x80004005;
  }
  uVar11 = *param_1;
  if (uVar11 < 0x28) {
    return 0x80004005;
  }
  puVar3 = (uint *)param_1[1];
  uVar5 = param_1[2];
  if ((int)uVar5 < 1) {
    uVar5 = -uVar5;
  }
  uVar2 = *(ushort *)((int)param_1 + 0xe);
  uVar6 = param_1[8];
  if ((uVar2 < 9) && (uVar6 == 0)) {
    uVar6 = 1 << ((byte)uVar2 & 0x1f);
  }
  uVar6 = uVar11 + uVar6 * 4;
  if (param_2 < uVar6) {
    return 0x80004005;
  }
  if ((short)param_1[3] != 1) {
    return 0x80004005;
  }
  if ((((int)param_1[2] < 0) && (param_1[4] != 0)) && (param_1[4] != 3)) {
    return 0x80004005;
  }
  if (param_1[4] < 3) {
    param_1 = (uint *)(uint)uVar2;
    if (((param_1 == (uint *)0x1) || (param_1 == (uint *)0x4)) || (param_1 == (uint *)0x8)) {
      param_2 = 0x29;
      goto LAB_0048d1bb;
    }
    if (param_1 == (uint *)0x10) goto LAB_0048d1ab;
    if (param_1 == (uint *)0x18) goto LAB_0048d05b;
    if (param_1 != (uint *)0x20) {
      return 0x80004005;
    }
  }
  else {
    if (param_1[4] != 3) {
      return 0x80004001;
    }
    if (uVar11 < 0x6c) {
      param_2 = 0;
      puVar19 = (uint *)(uVar11 + (int)param_1);
      uVar11 = puVar19[2];
      uVar15 = puVar19[1];
      uVar7 = *puVar19;
    }
    else {
      param_2 = param_1[0xd];
      uVar11 = param_1[0xc];
      uVar15 = param_1[0xb];
      uVar7 = param_1[10];
    }
    param_1 = (uint *)(uint)*(ushort *)((int)param_1 + 0xe);
    if (param_1 == (uint *)0x10) {
      if (uVar11 == 0xff) {
        if (uVar15 != 0xff) {
          return 0x80004005;
        }
        if (uVar7 != 0xff) {
          return 0x80004005;
        }
        if (param_2 != 0xff00) {
          return 0x80004005;
        }
        param_2 = 0x33;
        goto LAB_0048d1bb;
      }
      if (uVar11 != 0x1f) {
        if (uVar11 == 0xf) {
          if (uVar15 != 0xf0) {
            return 0x80004005;
          }
          if (uVar7 != 0xf00) {
            return 0x80004005;
          }
          if (param_2 == 0xf000) {
            param_2 = 0x1a;
          }
          else {
            if (param_2 != 0) {
              return 0x80004005;
            }
            param_2 = 0x1e;
          }
        }
        else {
          if (uVar11 != 3) {
            return 0x80004005;
          }
          if (uVar15 != 0x1c) {
            return 0x80004005;
          }
          if (uVar7 != 0xe0) {
            return 0x80004005;
          }
          if (param_2 != 0xff00) {
            return 0x80004005;
          }
          param_2 = 0x1d;
        }
        goto LAB_0048d1bb;
      }
      if (uVar15 == 0x7e0) {
        if (uVar7 != 0xf800) {
          return 0x80004005;
        }
        if (param_2 != 0) {
          return 0x80004005;
        }
        param_2 = 0x17;
        goto LAB_0048d1bb;
      }
      if (uVar15 != 0x3e0) {
        return 0x80004005;
      }
      if (uVar7 != 0x7c00) {
        return 0x80004005;
      }
      if (param_2 != 0) {
        if (param_2 != 0x8000) {
          return 0x80004005;
        }
        param_2 = 0x19;
        goto LAB_0048d1bb;
      }
LAB_0048d1ab:
      param_2 = 0x18;
      goto LAB_0048d1bb;
    }
    if (param_1 == (uint *)0x18) {
      if (uVar11 != 0xff) {
        return 0x80004005;
      }
      if (uVar15 != 0xff00) {
        return 0x80004005;
      }
      if (uVar7 != 0xff0000) {
        return 0x80004005;
      }
      if (param_2 != 0) {
        return 0x80004005;
      }
LAB_0048d05b:
      param_2 = 0x14;
      goto LAB_0048d1bb;
    }
    if (param_1 != (uint *)0x20) {
      return 0x80004005;
    }
    if (uVar11 != 0xff) {
      if (uVar11 == 0x3ff00000) {
        if (uVar15 != 0xffc00) {
          return 0x80004005;
        }
        if (uVar7 != 0x3ff) {
          return 0x80004005;
        }
        if (param_2 != 0xc0000000) {
          return 0x80004005;
        }
        param_2 = 0x1f;
      }
      else {
        if (uVar11 != 0) {
          return 0x80004005;
        }
        if (uVar15 != 0xffff0000) {
          return 0x80004005;
        }
        if (uVar7 != 0xffff) {
          return 0x80004005;
        }
        if (param_2 != 0) {
          return 0x80004005;
        }
        param_2 = 0x22;
      }
      goto LAB_0048d1bb;
    }
    if (uVar15 != 0xff00) {
      return 0x80004005;
    }
    if (uVar7 != 0xff0000) {
      return 0x80004005;
    }
    if (param_2 != 0) {
      if (param_2 != 0xff000000) {
        return 0x80004005;
      }
      param_2 = 0x15;
      goto LAB_0048d1bb;
    }
  }
  param_2 = 0x16;
LAB_0048d1bb:
  if ((*(int *)((int)this + 0x40) != 0) && (param_2 == 0x29)) {
    puVar19 = (uint *)puVar16[8];
    if (puVar19 == (uint *)0x0) {
      puVar19 = (uint *)(1 << ((byte)param_1 & 0x1f));
    }
    *(undefined4 *)((int)this + 0x3c) = 1;
    pvVar8 = operator_new(0x400);
    *(void **)((int)this + 8) = pvVar8;
    if (pvVar8 == (void *)0x0) {
      return 0x8007000e;
    }
    param_1 = (uint *)0x0;
    puVar9 = (undefined1 *)(*puVar16 + (int)puVar16);
    if (puVar19 != (uint *)0x0) {
      do {
        iVar14 = (int)param_1 * 4;
        *(undefined1 *)(iVar14 + *(int *)((int)this + 8)) = puVar9[2];
        *(undefined1 *)(iVar14 + 1 + *(int *)((int)this + 8)) = puVar9[1];
        *(undefined1 *)(iVar14 + 2 + *(int *)((int)this + 8)) = *puVar9;
        *(undefined1 *)(iVar14 + 3 + *(int *)((int)this + 8)) = 0xff;
        param_1 = (uint *)((int)param_1 + 1);
        puVar9 = puVar9 + 4;
      } while (param_1 < puVar19);
    }
    for (; puVar19 < &DAT_00000100; puVar19 = (uint *)((int)puVar19 + 1)) {
      iVar14 = (int)puVar19 * 4;
      *(undefined1 *)(iVar14 + *(int *)((int)this + 8)) = 0xff;
      *(undefined1 *)(iVar14 + 1 + *(int *)((int)this + 8)) = 0xff;
      *(undefined1 *)(iVar14 + 2 + *(int *)((int)this + 8)) = 0xff;
      *(undefined1 *)(iVar14 + 3 + *(int *)((int)this + 8)) = 0xff;
    }
  }
  uVar2 = *(ushort *)((int)puVar16 + 0xe);
  puVar19 = puVar3;
  if (uVar2 == 1) {
    puVar10 = (uint *)((int)puVar3 + 7U >> 3);
  }
  else if (uVar2 == 4) {
    puVar10 = (uint *)((int)puVar3 + 1U >> 1);
  }
  else {
    puVar10 = (uint *)((uint)(uVar2 >> 3) * (int)puVar3);
    puVar19 = puVar10;
  }
  uVar15 = (int)puVar10 + 3U & 0xfffffffc;
  *(uint *)this = param_2;
  uVar11 = (int)puVar19 + 3U & 0xfffffffc;
  *(uint **)((int)this + 0xc) = puVar3;
  *(uint *)((int)this + 0x30) = uVar11;
  *(undefined4 *)((int)this + 0x34) = 0;
  *(uint *)((int)this + 0x10) = uVar5;
  *(undefined4 *)((int)this + 0x14) = 1;
  if (*(int *)((int)this + 0x40) != 0) {
    if (((int)puVar16[2] < 0) && (7 < *(ushort *)((int)puVar16 + 0xe))) {
      *(undefined4 *)((int)this + 0x38) = 0;
      *(uint *)((int)this + 4) = uVar6 + (int)puVar16;
    }
    else {
      *(undefined4 *)((int)this + 0x38) = 1;
      pbVar12 = operator_new(uVar11 * uVar5);
      *(byte **)((int)this + 4) = pbVar12;
      if (pbVar12 == (byte *)0x0) {
        return 0x8007000e;
      }
      pbVar17 = (byte *)(uVar6 + (int)puVar16);
      iVar14 = *(int *)((int)this + 0x30);
      pbVar20 = pbVar12;
      local_1c = iVar14;
      if (-1 < (int)puVar16[2]) {
        pbVar20 = pbVar12 + (uVar5 - 1) * iVar14;
        local_1c = -iVar14;
      }
      pbVar13 = pbVar12 + iVar14 * uVar5;
      local_14 = pbVar20;
      if (puVar16[4] == 2) {
        while (pbVar12 <= pbVar20) {
          if (*pbVar17 == 0) {
            bVar4 = pbVar17[1];
            if (bVar4 == 0) {
              pbVar20 = local_14 + -*(int *)((int)this + 0x30);
              local_14 = pbVar20;
            }
            else if (bVar4 == 1) {
              pbVar20 = pbVar12 + -*(int *)((int)this + 0x30);
            }
            else if (bVar4 == 2) {
              pbVar13 = pbVar17 + 3;
              pbVar17 = pbVar17 + 2;
              pbVar20 = pbVar20 + ((uint)*pbVar17 - (uint)*pbVar13 * *(int *)((int)this + 0x30));
            }
            else {
              uVar11 = 0;
              if (bVar4 != 0) {
                do {
                  if ((uVar11 & 1) == 0) {
                    bVar4 = pbVar17[((int)uVar11 >> 1) + 2] >> 4;
                  }
                  else {
                    bVar4 = pbVar17[((int)uVar11 >> 1) + 2] & 0xf;
                  }
                  pbVar20[uVar11] = bVar4;
                  uVar11 = uVar11 + 1;
                } while ((int)uVar11 < (int)(uint)pbVar17[1]);
              }
              pbVar20 = pbVar20 + pbVar17[1];
              pbVar17 = pbVar17 + ((pbVar17[1] >> 1) + 1 & 0xfffffffe);
            }
          }
          else {
            uVar11 = 0;
            if (*pbVar17 != 0) {
              do {
                if ((uVar11 & 1) == 0) {
                  bVar4 = pbVar17[1] >> 4;
                }
                else {
                  bVar4 = pbVar17[1] & 0xf;
                }
                pbVar20[uVar11] = bVar4;
                uVar11 = uVar11 + 1;
              } while ((int)uVar11 < (int)(uint)*pbVar17);
            }
            pbVar20 = pbVar20 + *pbVar17;
          }
          pbVar17 = pbVar17 + 2;
        }
      }
      else if (puVar16[4] == 1) {
        while (pbVar12 <= pbVar20) {
          bVar4 = *pbVar17;
          if (bVar4 == 0) {
            uVar11 = (uint)pbVar17[1];
            if (uVar11 == 0) {
              pbVar20 = local_14 + -*(int *)((int)this + 0x30);
              local_14 = pbVar20;
            }
            else if (uVar11 == 1) {
              pbVar20 = pbVar12 + -*(int *)((int)this + 0x30);
            }
            else {
              if (uVar11 != 2) {
                pbVar13 = pbVar17 + 2;
                pbVar18 = pbVar20;
                for (uVar5 = (uint)(pbVar17[1] >> 2); uVar5 != 0; uVar5 = uVar5 - 1) {
                  *(undefined4 *)pbVar18 = *(undefined4 *)pbVar13;
                  pbVar13 = pbVar13 + 4;
                  pbVar18 = pbVar18 + 4;
                }
                for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
                  *pbVar18 = *pbVar13;
                  pbVar13 = pbVar13 + 1;
                  pbVar18 = pbVar18 + 1;
                }
                uVar11 = (uint)pbVar17[1];
                pbVar17 = pbVar17 + (uVar11 + 1 & 0xfffffffe);
                goto LAB_0048d4bb;
              }
              pbVar13 = pbVar17 + 3;
              pbVar17 = pbVar17 + 2;
              pbVar20 = pbVar20 + ((uint)*pbVar17 - (uint)*pbVar13 * *(int *)((int)this + 0x30));
            }
          }
          else {
            bVar1 = pbVar17[1];
            pbVar13 = pbVar20;
            for (uVar11 = (uint)(bVar4 >> 2); uVar11 != 0; uVar11 = uVar11 - 1) {
              *(uint *)pbVar13 = CONCAT22(CONCAT11(bVar1,bVar1),CONCAT11(bVar1,bVar1));
              pbVar13 = pbVar13 + 4;
            }
            for (uVar11 = bVar4 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
              *pbVar13 = bVar1;
              pbVar13 = pbVar13 + 1;
            }
            uVar11 = (uint)*pbVar17;
LAB_0048d4bb:
            pbVar20 = pbVar20 + uVar11;
          }
          pbVar17 = pbVar17 + 2;
        }
      }
      else if (*(short *)((int)puVar16 + 0xe) == 1) {
        for (; (pbVar12 <= pbVar20 && (pbVar20 < pbVar13)); pbVar20 = pbVar20 + local_1c) {
          param_1 = (uint *)0x0;
          if (puVar3 != (uint *)0x0) {
            do {
              *(byte *)((int)param_1 + (int)pbVar20) =
                   pbVar17[(uint)param_1 >> 3] >> (7 - ((byte)param_1 & 7) & 0x1f) & 1;
              param_1 = (uint *)((int)param_1 + 1);
            } while (param_1 < puVar3);
          }
          pbVar17 = pbVar17 + uVar15;
        }
      }
      else if (*(short *)((int)puVar16 + 0xe) == 4) {
        for (; (pbVar12 <= pbVar20 && (pbVar20 < pbVar13)); pbVar20 = pbVar20 + local_1c) {
          puVar16 = (uint *)0x0;
          if (puVar3 != (uint *)0x0) {
            do {
              if (((uint)puVar16 & 1) == 0) {
                bVar4 = pbVar17[(uint)puVar16 >> 1] >> 4;
              }
              else {
                bVar4 = pbVar17[(uint)puVar16 >> 1] & 0xf;
              }
              *(byte *)((int)puVar16 + (int)pbVar20) = bVar4;
              puVar16 = (uint *)((int)puVar16 + 1);
            } while (puVar16 < puVar3);
          }
          pbVar17 = pbVar17 + uVar15;
        }
      }
      else {
        while ((pbVar12 <= pbVar20 && (pbVar20 < pbVar13))) {
          pbVar18 = pbVar17 + uVar15;
          pbVar21 = pbVar20 + local_1c;
          for (uVar11 = (uint)puVar19 >> 2; uVar11 != 0; uVar11 = uVar11 - 1) {
            *(undefined4 *)pbVar20 = *(undefined4 *)pbVar17;
            pbVar17 = pbVar17 + 4;
            pbVar20 = pbVar20 + 4;
          }
          pbVar22 = pbVar17;
          pbVar23 = pbVar20;
          for (uVar11 = (uint)puVar19 & 3; pbVar17 = pbVar18, pbVar20 = pbVar21, uVar11 != 0;
              uVar11 = uVar11 - 1) {
            *pbVar23 = *pbVar22;
            pbVar22 = pbVar22 + 1;
            pbVar23 = pbVar23 + 1;
          }
        }
      }
    }
  }
  return 0;
}

