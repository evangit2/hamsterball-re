
void __thiscall Sound_LoadOgg(void *this,char *param_1)

{
  undefined4 uVar1;
  short *psVar2;
  FILE *_File;
  int iVar3;
  uint uVar4;
  undefined4 *puVar5;
  short sVar6;
  short *unaff_ESI;
  int iVar7;
  int unaff_EDI;
  int *piVar8;
  longlong lVar9;
  undefined4 uVar10;
  int *piVar11;
  undefined1 local_170 [12];
  undefined4 local_164;
  int local_160;
  int local_15c;
  undefined4 local_158;
  int local_14c [4];
  undefined4 *local_13c;
  int aiStack_138 [4];
  int local_128 [67];
  int iStack_1c;
  
  if (((*(int *)((int)this + 4) != 0) && (*(int *)(*(int *)((int)this + 4) + 0x84c) != 0)) &&
     (_File = (FILE *)CRT_fsopen(param_1,"rb"), _File != (FILE *)0x0)) {
    iVar3 = Sound_EnumerateDevices(_File,local_128,(undefined4 *)0x0,0);
    if (iVar3 < 0) {
      _fclose(_File);
      return;
    }
    iVar3 = MeshArchive_GetSubmeshPtr((int)local_128,-1);
    local_164 = CONCAT22(*(ushort *)(iVar3 + 4),1);
    local_160 = *(int *)(iVar3 + 8);
    uVar4 = (int)((uint)*(ushort *)(iVar3 + 4) * 0x10) >> 3;
    local_15c = (uVar4 & 0xffff) * local_160;
    local_158 = CONCAT22(0x10,(short)uVar4);
    lVar9 = BitStream_ReadValue((int)local_128,-1);
    iVar7 = (int)lVar9 * (local_158 & 0xffff);
    piVar8 = local_14c;
    for (iVar3 = 9; iVar3 != 0; iVar3 = iVar3 + -1) {
      *piVar8 = 0;
      piVar8 = piVar8 + 1;
    }
    local_13c = &local_164;
    piVar11 = (int *)0x0;
    local_14c[0] = 0x24;
    local_14c[1] = 0x82;
    piVar8 = *(int **)(*(int *)((int)this + 4) + 0x84c);
    local_14c[2] = iVar7;
    iVar3 = (**(code **)(*piVar8 + 0xc))(piVar8,local_14c,local_170);
    if ((iVar3 != 0) ||
       (iVar7 = (**(code **)(*piVar11 + 0x2c))
                          (piVar11,0,iVar7,&stack0xfffffe88,&stack0xfffffe84,0,0,0),
       iVar3 = unaff_EDI, psVar2 = unaff_ESI, iVar7 != 0)) {
      MeshArchive_dtor(aiStack_138);
      return;
    }
    for (; 0 < iVar3; iVar3 = iVar3 - uVar4) {
      uVar4 = MeshArchive_ReadVertices((int)aiStack_138,psVar2,iVar3,&local_160);
      if ((uVar4 == 0) || ((int)uVar4 < 0)) break;
      psVar2 = (short *)((int)psVar2 + uVar4);
    }
    uVar10 = 0;
    (**(code **)(*piVar11 + 0x4c))(piVar11,unaff_ESI,unaff_EDI,0,0);
    puVar5 = operator_new(0xc);
    if (puVar5 == (undefined4 *)0x0) {
      puVar5 = (undefined4 *)0x0;
    }
    else {
      uVar1 = *(undefined4 *)((int)this + 4);
      *puVar5 = &PTR_SoundEntry_ScalarDtor_004d8e78;
      puVar5[2] = uVar1;
      puVar5[1] = uVar10;
    }
    AthenaList_Append((void *)((int)this + 8),(int)puVar5);
    sVar6 = 0;
    if (0 < iStack_1c) {
      do {
        piVar8 = *(int **)(*(int *)((int)this + 4) + 0x84c);
        (**(code **)(*piVar8 + 0x14))(piVar8,uVar10,&stack0xfffffe88);
        puVar5 = operator_new(0xc);
        if (puVar5 == (undefined4 *)0x0) {
          puVar5 = (undefined4 *)0x0;
        }
        else {
          uVar1 = *(undefined4 *)((int)this + 4);
          *puVar5 = &PTR_SoundEntry_ScalarDtor_004d8e78;
          puVar5[2] = uVar1;
          puVar5[1] = unaff_ESI;
        }
        AthenaList_Append((void *)((int)this + 8),(int)puVar5);
        sVar6 = sVar6 + 1;
      } while (sVar6 < iStack_1c);
    }
    MeshArchive_dtor(local_14c);
  }
  return;
}

