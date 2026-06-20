
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 D3DX_LoadSurfaceFromPNG(int param_1,uint param_2)

{
  undefined1 *puVar1;
  byte bVar2;
  int iVar3;
  uint uVar4;
  void *pvVar5;
  int iVar6;
  undefined4 unaff_ESI;
  void *unaff_EDI;
  int iVar7;
  longlong lVar8;
  undefined1 auStack_158 [256];
  double local_58;
  double local_50;
  int local_48;
  uint local_44;
  int local_40;
  undefined1 local_3c [4];
  int *local_38;
  uint local_34;
  uint local_30;
  uint local_2c;
  uint local_28;
  uint local_24;
  undefined4 *local_20;
  uint local_1c;
  int local_18;
  int local_14;
  uint local_10;
  uint *local_c;
  int *local_8;
  
  local_8 = (int *)0x0;
  local_c = (uint *)0x0;
  local_20 = (undefined4 *)0x0;
  iVar3 = PNG_CompareSignature(param_1,0,param_2);
  if (iVar3 != 0) {
    return unaff_ESI;
  }
  local_8 = PNG_InitContext("1.0.5",0,&LAB_0048dffa,CRT_Noop2);
  if (((local_8 == (int *)0x0) ||
      (local_c = Color_AllocVertexBlock((int)local_8), local_c == (uint *)0x0)) ||
     (iVar3 = __setjmp3(local_8,0,unaff_EDI,unaff_ESI), iVar3 != 0)) goto LAB_0048e56d;
  local_48 = param_1;
  local_44 = param_2;
  PNG_SetWriteFn((int)local_8,&local_48,&LAB_0048dfb7);
  PNG_ParseFrame(local_8,local_c);
  PNG_GetIHDR((int)local_8,local_c,&local_34,&local_1c,&local_28,&local_30,(uint *)0x0,(uint *)0x0,
              (uint *)0x0);
  if (local_30 == 0) {
    *local_38 = 0x32;
  }
  else if (local_30 == 2) {
    *local_38 = 0x14;
  }
  else if (local_30 == 3) {
    *local_38 = 0x29;
  }
  else if (local_30 == 4) {
    *local_38 = 0x33;
  }
  else if (local_30 == 6) {
    *local_38 = 0x15;
  }
  if (local_28 == 0x10) {
    PNG_SetTransformStrip16((int)local_8);
    iVar3 = *local_38;
    if (iVar3 == 0x14) {
      *local_38 = 0x36315220;
    }
    else if (iVar3 == 0x15) {
      *local_38 = 0x36315241;
    }
    else if (iVar3 == 0x32) {
      *local_38 = 0x36314c20;
    }
    else {
      if (iVar3 != 0x33) {
        return unaff_ESI;
      }
      *local_38 = 0x36314c41;
    }
  }
  if ((int)local_28 < 8) {
    PNG_SetTransformPack((int)local_8);
  }
  if ((local_30 == 0) && ((int)local_28 < 8)) {
    PNG_SetTransformShift((int)local_8);
  }
  iVar3 = PNG_GetSRGBIntent((int)local_8,(int)local_c,(uint *)local_3c);
  if (iVar3 == 0) {
    iVar3 = PNG_GetGamma((int)local_8,(int)local_c,&local_58);
    if (iVar3 != 0) {
      PNG_SetTransformScale((int)local_8,_DAT_004dc030,local_58);
    }
  }
  else {
    PNG_SetSRGB((int)local_8,(int)local_c,local_3c[0]);
  }
  if ((*local_38 != 0x29) &&
     (uVar4 = Mem_CheckAccessPath((int)local_8,(int)local_c,0x10), uVar4 != 0)) {
    PNG_SetTransformShift((int)local_8);
    iVar3 = *local_38;
    if (iVar3 == 0x14) {
      *local_38 = 0x15;
    }
    else if (iVar3 == 0x32) {
      *local_38 = 0x33;
    }
    else if (iVar3 == 0x36314c20) {
      *local_38 = 0x36314c41;
    }
    else if (iVar3 == 0x36315220) {
      *local_38 = 0x36315241;
    }
  }
  iVar3 = *local_38;
  if ((((iVar3 == 0x14) || (iVar3 == 0x36315220)) || (iVar3 == 0x15)) || (iVar3 == 0x36315241)) {
    Object_SetDirtyFlag((int)local_8);
  }
  if (*local_38 == 0x14) {
    PNG_SetTransformAddAlpha((int)local_8,0xff,1);
    *local_38 = 0x16;
  }
  PNG_DispatchRow(local_8,(int *)local_c);
  iVar3 = *local_38;
  uVar4 = 3;
  if (iVar3 < 0x34) {
    if (iVar3 == 0x33) {
LAB_0048e2d9:
      local_10 = 2;
      uVar4 = local_10;
    }
    else if (iVar3 != 0x14) {
      if ((iVar3 == 0x15) || (iVar3 == 0x16)) {
LAB_0048e2cf:
        uVar4 = 4;
      }
      else if ((iVar3 == 0x29) || (uVar4 = local_10, iVar3 == 0x32)) goto LAB_0048e295;
    }
  }
  else if (iVar3 == 0x36314c20) {
LAB_0048e295:
    local_10 = 1;
    uVar4 = local_10;
  }
  else {
    if (iVar3 == 0x36314c41) goto LAB_0048e2d9;
    if ((iVar3 != 0x36315220) && (uVar4 = local_10, iVar3 == 0x36315241)) goto LAB_0048e2cf;
  }
  local_10 = uVar4;
  bVar2 = PNG_GetCompressionType((int)local_8,(int)local_c);
  if (local_10 == bVar2) {
    local_38[3] = local_34;
    local_38[4] = local_1c;
    local_38[5] = 1;
    iVar3 = PNG_GetWidth((int)local_8,(int)local_c);
    local_38[0xc] = iVar3;
    local_38[0xd] = 0;
    if (local_38[0x10] != 0) {
      pvVar5 = operator_new(iVar3 * local_1c);
      local_38[1] = (int)pvVar5;
      if (pvVar5 != (void *)0x0) {
        local_38[0xe] = 1;
        local_20 = operator_new(local_1c << 2);
        if (local_20 != (undefined4 *)0x0) {
          uVar4 = 0;
          if (local_1c != 0) {
            do {
              local_20[uVar4] = local_38[0xc] * uVar4 + local_38[1];
              uVar4 = uVar4 + 1;
            } while (uVar4 < local_1c);
          }
          PNG_DecodeImage(local_8,local_20);
          if (*local_38 == 0x29) {
            local_2c = 0;
            local_24 = 0;
            uVar4 = Mem_CheckAccessPath((int)local_8,(int)local_c,8);
            if (uVar4 != 0) {
              PNG_GetPalette((int)local_8,(int)local_c,&local_14,&local_24);
            }
            uVar4 = Mem_CheckAccessPath((int)local_8,(int)local_c,0x10);
            if (uVar4 != 0) {
              PNG_GetSPLT((int)local_8,(int)local_c,&local_40,&local_2c,(int *)0x0);
            }
            if (((float)local_8[0x4c] <= (float)_DAT_004dc028) ||
               ((float)local_8[0x4d] <= (float)_DAT_004dc028)) {
              iVar3 = 0;
              do {
                auStack_158[iVar3] = (char)iVar3;
                iVar3 = iVar3 + 1;
              } while (iVar3 < 0x100);
            }
            else {
              local_18 = 0;
              local_50 = (double)((float)_DAT_004cf3c8 /
                                 ((float)local_8[0x4d] * (float)local_8[0x4c]));
              do {
                FPU_FdivThunk();
                lVar8 = __ftol();
                iVar3 = local_18 + 1;
                puVar1 = auStack_158 + local_18;
                local_18 = iVar3;
                *puVar1 = (char)lVar8;
              } while (iVar3 < 0x100);
            }
            pvVar5 = operator_new(0x400);
            local_38[2] = (int)pvVar5;
            if (pvVar5 != (void *)0x0) {
              iVar3 = 0;
              local_38[0xf] = 1;
              if (0 < (int)local_2c) {
                iVar7 = 0;
                do {
                  iVar6 = iVar3 * 4;
                  *(undefined1 *)(iVar6 + local_38[2]) = auStack_158[*(byte *)(iVar7 + local_14)];
                  *(undefined1 *)(local_38[2] + 1 + iVar6) =
                       auStack_158[*(byte *)(iVar7 + 1 + local_14)];
                  *(undefined1 *)(local_38[2] + 2 + iVar6) =
                       auStack_158[*(byte *)(iVar7 + 2 + local_14)];
                  puVar1 = (undefined1 *)(iVar3 + local_40);
                  iVar3 = iVar3 + 1;
                  *(undefined1 *)(local_38[2] + 3 + iVar6) = *puVar1;
                  iVar7 = iVar7 + 3;
                } while (iVar3 < (int)local_2c);
              }
              if (iVar3 < (int)local_24) {
                iVar7 = iVar3 * 3;
                do {
                  iVar6 = iVar3 * 4;
                  *(undefined1 *)(iVar6 + local_38[2]) = auStack_158[*(byte *)(iVar7 + local_14)];
                  *(undefined1 *)(local_38[2] + 1 + iVar6) =
                       auStack_158[*(byte *)(iVar7 + 1 + local_14)];
                  *(undefined1 *)(local_38[2] + 2 + iVar6) =
                       auStack_158[*(byte *)(iVar7 + 2 + local_14)];
                  *(undefined1 *)(local_38[2] + 3 + iVar6) = 0xff;
                  iVar3 = iVar3 + 1;
                  iVar7 = iVar7 + 3;
                } while (iVar3 < (int)local_24);
              }
              if (iVar3 < 0x100) {
                iVar3 = iVar3 << 2;
                do {
                  *(undefined1 *)(iVar3 + local_38[2]) = 0xff;
                  *(undefined1 *)(iVar3 + 1 + local_38[2]) = 0xff;
                  *(undefined1 *)(iVar3 + 2 + local_38[2]) = 0xff;
                  *(undefined1 *)(iVar3 + 3 + local_38[2]) = 0xff;
                  iVar3 = iVar3 + 4;
                } while (iVar3 < 0x400);
              }
            }
          }
        }
      }
    }
  }
LAB_0048e56d:
  if (local_8 != (int *)0x0) {
    PNG_Close(&local_8,&local_c,(undefined4 *)0x0);
  }
  if (local_20 != (undefined4 *)0x0) {
    _free(local_20);
  }
  return unaff_ESI;
}

