
void * __thiscall Texture_LoadWithMips(void *this,LPCSTR param_1,LPCSTR param_2)

{
  undefined4 *puVar1;
  int *piVar2;
  char cVar3;
  int *piVar4;
  int *piVar5;
  bool bVar6;
  LPCSTR pCVar7;
  char *pcVar8;
  int iVar9;
  BOOL BVar10;
  int iVar11;
  undefined4 *unaff_EBP;
  uint uVar12;
  int iVar13;
  int unaff_retaddr;
  int iStack_7f8;
  int local_7f4 [5];
  undefined4 uStack_7e0;
  int local_7dc [6];
  undefined4 uStack_7c4;
  undefined4 uStack_7c0;
  char local_7bc [256];
  undefined1 auStack_6bc [4];
  char local_6b8 [256];
  char acStack_5b8 [4];
  char acStack_5b4 [260];
  int aiStack_4b0 [20];
  int aiStack_460 [85];
  char local_30c [260];
  char local_208 [260];
  char local_104 [260];
  
  *(undefined ***)this = &PTR_Texture_DeletingDtor_004da648;
  Window_Notify(0x5341d0,(byte *)"---------------");
  Window_Notify(0x5341d0,(byte *)"Start Loading Texture: %s");
  iVar11 = 0;
  *(LPCSTR *)((int)this + 0xc) = param_1;
  *(undefined1 *)((int)this + 0x1c) = 0;
  *(undefined1 *)((int)this + 0x1d) = 0;
  *(undefined1 *)((int)this + 0x1e) = 0;
  *(undefined4 *)((int)this + 4) = 0;
  *(undefined4 *)((int)this + 8) = 0;
  AthenaString_SprintfToBuffer(local_30c,(byte *)"textures\\");
  AthenaString_SprintfToBuffer(local_208,&DAT_004d1354);
  AthenaString_SprintfToBuffer(local_104,&DAT_004d1354);
  if (*(int *)(*(int *)((int)this + 0xc) + 0x7d8) != 0) {
    AthenaString_SprintfToBuffer(local_30c,&DAT_004da61c);
    AthenaString_SprintfToBuffer(local_208,(byte *)"textures\\");
  }
  bVar6 = false;
  pCVar7 = param_2;
  do {
    cVar3 = *pCVar7;
    pCVar7[(int)(local_7bc + -(int)param_2)] = cVar3;
    pCVar7 = pCVar7 + 1;
  } while (cVar3 != '\0');
  pcVar8 = local_7bc;
  uVar12 = 0;
  do {
    cVar3 = *pcVar8;
    pcVar8 = pcVar8 + 1;
  } while (cVar3 != '\0');
  local_7f4[0] = (int)pcVar8 - (int)(local_7bc + 1);
  if (local_7f4[0] != 0) {
    iVar13 = 0x3ab;
    do {
      if (local_7bc[uVar12] == '.') {
        pcVar8 = local_7bc + uVar12 + 1;
        do {
          cVar3 = *pcVar8;
          pcVar8[iVar13] = cVar3;
          pcVar8 = pcVar8 + 1;
        } while (cVar3 != '\0');
        local_7bc[uVar12] = '\0';
      }
      pcVar8 = local_7bc;
      uVar12 = uVar12 + 1;
      iVar13 = iVar13 + -1;
      do {
        cVar3 = *pcVar8;
        pcVar8 = pcVar8 + 1;
      } while (cVar3 != '\0');
    } while (uVar12 < (uint)((int)pcVar8 - (int)(local_7bc + 1)));
  }
  iVar13 = 0;
  do {
    AthenaString_SprintfToBuffer(local_6b8,(byte *)"%s%s-mip1.%s");
    iVar9 = _check_file_access(local_6b8,0);
    if (iVar9 == 0) {
      bVar6 = true;
    }
    AthenaString_SprintfToBuffer(local_6b8,&DAT_004da604);
    iVar9 = _check_file_access(local_6b8,0);
    if (iVar9 == 0) break;
    iVar11 = iVar11 + 1;
    if (3 < iVar11) {
      iVar11 = 0;
    }
    iVar13 = iVar13 + 1;
  } while (iVar13 < 3);
  Window_Notify(0x5341d0,(byte *)"Initial Load...");
  BVar10 = SetFileSecurityW(*(LPCWSTR *)(*(int *)((int)this + 0xc) + 0x154),
                            (SECURITY_INFORMATION)local_6b8,(PSECURITY_DESCRIPTOR)0xffffffff);
  if ((BVar10 < 0) &&
     (BVar10 = SetFileSecurityW(*(LPCWSTR *)(*(int *)((int)this + 0xc) + 0x154),
                                (SECURITY_INFORMATION)local_6b8,(PSECURITY_DESCRIPTOR)0xffffffff),
     BVar10 < 0)) {
    if (BVar10 < -0x7789f795) {
      if (BVar10 == -0x7789f796) {
        Window_Notify(0x5341d0,(byte *)"D3DERR_NOTAVAILABLE");
LAB_004769f1:
        Window_Notify(0x5341d0,(byte *)"D3DERR_OUTOFVIDEOMEMORY");
LAB_00476a03:
        Window_Notify(0x5341d0,(byte *)"D3DERR_OUTOFVIDEOMEMORY");
LAB_00476a15:
        Window_Notify(0x5341d0,(byte *)"D3DXERR_INVALIDDATA");
      }
      else if (BVar10 != -0x7ff8fff2) {
        if (BVar10 != -0x7789fe84) goto LAB_00476a39;
        goto LAB_004769f1;
      }
      Window_Notify(0x5341d0,(byte *)"E_OUTOFMEMORY");
    }
    else {
      if (BVar10 == -0x7789f794) goto LAB_00476a03;
      if (BVar10 == -0x7789f4a7) goto LAB_00476a15;
    }
LAB_00476a39:
    MessageBoxA((HWND)0x0,param_2,"TEXTURE LOAD FAILED!",0);
    *(int *)((int)this + 4) = 0;
  }
  piVar4 = *(int **)((int)this + 4);
  if (piVar4 != (int *)0x0) {
    (**(code **)(*piVar4 + 0x38))(piVar4,0,local_7dc);
    local_7f4[0] = local_7dc[0];
    if (((local_7dc[0] == 0x15) || (local_7dc[0] == 0x1a)) || (local_7dc[0] == 0x1d)) {
      *(undefined1 *)((int)this + 0x1c) = 1;
    }
    *(undefined4 *)((int)this + 0x14) = uStack_7c4;
    *(undefined4 *)((int)this + 0x18) = uStack_7c0;
    pCVar7 = param_2;
    do {
      cVar3 = *pCVar7;
      pCVar7 = pCVar7 + 1;
    } while (cVar3 != '\0');
    pcVar8 = operator_new((uint)(pCVar7 + (1 - (int)(param_2 + 1))));
    *(char **)((int)this + 8) = pcVar8;
    pCVar7 = param_2;
    do {
      cVar3 = *pCVar7;
      pCVar7 = pCVar7 + 1;
      *pcVar8 = cVar3;
      pcVar8 = pcVar8 + 1;
    } while (cVar3 != '\0');
    *(undefined4 *)((int)this + 0x10) = 1;
  }
  *(undefined4 *)((int)this + 0x20) = 0;
  if (!bVar6) {
LAB_00476f51:
    Window_Notify(0x5341d0,(byte *)"Finish Loading Texture: %s");
    Window_Notify(0x5341d0,(byte *)"---------------");
    return this;
  }
  Window_Notify(0x5341d0,(byte *)"Loading MipMaps...");
  (**(code **)(**(int **)((int)this + 4) + 8))(*(int **)((int)this + 4));
  *(undefined4 *)((int)this + 4) = 0;
  Window_Notify(0x5341d0,(byte *)"     ...mipmap[0] filename: %s");
  SetFileSecurityW(*(LPCWSTR *)(*(int *)((int)this + 0xc) + 0x154),(SECURITY_INFORMATION)auStack_6bc
                   ,(PSECURITY_DESCRIPTOR)0xffffffff);
  iVar11 = 1;
  iVar13 = 1;
  do {
    AthenaString_SprintfToBuffer(acStack_5b8,(byte *)"%s%s-mip%d.%s");
    Window_Notify(0x5341d0,(byte *)"     ...mipmap[%d] filename: %s");
    iVar9 = _check_file_access(acStack_5b8,0);
    if (iVar9 != 0) break;
    BVar10 = SetFileSecurityW(*(LPCWSTR *)(*(int *)((int)this + 0xc) + 0x154),
                              (SECURITY_INFORMATION)acStack_5b8,(PSECURITY_DESCRIPTOR)0xffffffff);
    if ((BVar10 < 0) &&
       (BVar10 = SetFileSecurityW(*(LPCWSTR *)(*(int *)((int)this + 0xc) + 0x154),
                                  (SECURITY_INFORMATION)acStack_5b8,(PSECURITY_DESCRIPTOR)0xffffffff
                                 ), BVar10 < 0)) {
      if (BVar10 < -0x7789f795) {
        if (BVar10 == -0x7789f796) {
          Window_Notify(0x5341d0,(byte *)"D3DERR_NOTAVAILABLE");
LAB_00476d19:
          Window_Notify(0x5341d0,(byte *)"D3DERR_OUTOFVIDEOMEMORY");
LAB_00476d2b:
          Window_Notify(0x5341d0,(byte *)"D3DERR_OUTOFVIDEOMEMORY");
          goto LAB_00476d3d;
        }
        if (BVar10 != -0x7ff8fff2) {
          if (BVar10 != -0x7789fe84) goto LAB_00476d61;
          goto LAB_00476d19;
        }
      }
      else {
        if (BVar10 == -0x7789f794) goto LAB_00476d2b;
        if (BVar10 != -0x7789f4a7) goto LAB_00476d61;
LAB_00476d3d:
        Window_Notify(0x5341d0,(byte *)"D3DXERR_INVALIDDATA");
      }
      Window_Notify(0x5341d0,(byte *)"E_OUTOFMEMORY");
LAB_00476d61:
      MessageBoxA((HWND)0x0,param_1,"TEXTURE LOAD FAILED!",0);
      *(undefined4 *)((int)this + 4) = 0;
      return this;
    }
    iVar11 = iVar11 + 1;
    iVar13 = iVar13 + 1;
  } while (iVar13 < 0x14);
  Window_Notify(0x5341d0,(byte *)"Create Texture with %d Mip Levels...");
  puVar1 = (undefined4 *)((int)this + 4);
  iVar13 = (**(code **)(**(int **)(unaff_retaddr + 0x154) + 0x50))
                     (*(int **)(unaff_retaddr + 0x154),*(undefined4 *)((int)this + 0x14),
                      *(undefined4 *)((int)this + 0x18),iVar11,0,iStack_7f8,1,puVar1);
  if ((-1 < iVar13) ||
     (iVar13 = (**(code **)(**(int **)(param_1 + 0x154) + 0x50))
                         (*(int **)(param_1 + 0x154),*(undefined4 *)((int)this + 0x14),
                          *(undefined4 *)((int)this + 0x18),iVar11,0,iStack_7f8,1,puVar1),
     -1 < iVar13)) {
    iVar13 = 0;
    if (0 < iVar11) {
      do {
        piVar5 = *(int **)((int)this + 4);
        piVar4 = aiStack_4b0 + iVar13;
        piVar2 = aiStack_460 + iVar13;
        *piVar4 = 0;
        *piVar2 = 0;
        (**(code **)(*piVar5 + 0x3c))(piVar5,iVar13,piVar4);
        (**(code **)(*(int *)*unaff_EBP + 0x3c))((int *)*unaff_EBP,0,piVar2);
        (**(code **)(**(int **)((int)this + 4) + 0x38))(*(int **)((int)this + 4),iVar13,local_7f4);
        local_7f4[2] = 0;
        local_7f4[3] = 0;
        local_7f4[4] = uStack_7c4;
        uStack_7e0 = uStack_7c0;
        local_7f4[0] = 0;
        local_7f4[1] = 0;
        Window_Notify(0x5341d0,(byte *)"Plotting mip level...");
        if (((*piVar2 != 0) && (*piVar4 != 0)) &&
           (iVar11 = (**(code **)(**(int **)(param_1 + 0x154) + 0x70))
                               (*(int **)(param_1 + 0x154),*piVar2,local_7f4 + 2,1,*piVar4,local_7f4
                               ), iVar11 < 0)) {
          D3D_ErrorToString(iVar11,acStack_5b4,0xff);
          Window_Notify(0x5341d0,(byte *)
                                 "Error %s [%d,%d]*******************************************************"
                       );
        }
        iVar13 = iVar13 + 1;
      } while (iVar13 < iStack_7f8);
    }
    *(undefined1 *)((int)this + 0x1d) = 1;
    *(int *)((int)this + 0x20) = iStack_7f8;
    goto LAB_00476f51;
  }
  if (iVar13 < -0x7789f795) {
    if (iVar13 == -0x7789f796) {
      Window_Notify(0x5341d0,(byte *)"D3DERR_NOTAVAILABLE");
LAB_00476db0:
      Window_Notify(0x5341d0,(byte *)"D3DERR_OUTOFVIDEOMEMORY");
LAB_00476dc2:
      Window_Notify(0x5341d0,(byte *)"D3DERR_OUTOFVIDEOMEMORY");
      goto LAB_00476dd4;
    }
    if (iVar13 != -0x7ff8fff2) {
      if (iVar13 != -0x7789fe84) goto LAB_00476df8;
      goto LAB_00476db0;
    }
  }
  else {
    if (iVar13 == -0x7789f794) goto LAB_00476dc2;
    if (iVar13 != -0x7789f4a7) goto LAB_00476df8;
LAB_00476dd4:
    Window_Notify(0x5341d0,(byte *)"D3DXERR_INVALIDDATA");
  }
  Window_Notify(0x5341d0,(byte *)"E_OUTOFMEMORY");
LAB_00476df8:
  MessageBoxA((HWND)0x0,param_2,"TEXTURE LOAD FAILED!",0);
  *puVar1 = 0;
  return this;
}

