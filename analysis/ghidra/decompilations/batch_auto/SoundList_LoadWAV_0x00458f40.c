
void __thiscall SoundList_LoadWAV(void *this,char *param_1)

{
  undefined4 uVar1;
  int iVar2;
  size_t _Size;
  undefined4 *puVar3;
  int iVar4;
  uint uVar5;
  char *pcVar6;
  short sVar7;
  int *piVar8;
  undefined4 *puVar9;
  char *pcVar10;
  int *piVar11;
  undefined4 *puVar12;
  bool bVar13;
  undefined1 *puVar14;
  void *_Memory;
  void *pvVar15;
  int *piVar16;
  undefined4 *local_58 [5];
  size_t local_44;
  undefined4 local_40;
  uint local_3c;
  undefined2 local_34;
  short local_32;
  int local_30;
  int *local_2c;
  ushort local_28;
  short local_26;
  undefined4 local_24 [4];
  undefined2 *local_14;
  int *piStack_c;
  
  if ((*(int *)((int)this + 4) != 0) && (*(int *)(*(int *)((int)this + 4) + 0x84c) != 0)) {
    iVar2 = FID_conflict___open(param_1,0x8000);
    if ((short)iVar2 == -1) {
      MessageBoxA((HWND)0x0,"Could not load sound",param_1,0);
      return;
    }
    iVar2 = (int)(short)iVar2;
    _Size = __filelength(iVar2);
    local_44 = _Size;
    puVar3 = _malloc(_Size);
    local_58[0] = puVar3;
    __read(iVar2,puVar3,_Size);
    __close(iVar2);
    iVar2 = 5;
    bVar13 = true;
    local_40 = (int *)*puVar3;
    uVar5 = local_3c >> 8;
    local_3c = uVar5 << 8;
    pcVar6 = (char *)&local_40;
    pcVar10 = "RIFF";
    do {
      if (iVar2 == 0) break;
      iVar2 = iVar2 + -1;
      bVar13 = *pcVar6 == *pcVar10;
      pcVar6 = pcVar6 + 1;
      pcVar10 = pcVar10 + 1;
    } while (bVar13);
    if (!bVar13) {
      _free(local_58[0]);
      MessageBoxA((HWND)0x0,"Could not load sound (1)","HEY",0);
      return;
    }
    local_40 = (int *)puVar3[2];
    iVar2 = 5;
    bVar13 = true;
    local_3c = uVar5 << 8;
    pcVar6 = (char *)&local_40;
    pcVar10 = "WAVE";
    do {
      if (iVar2 == 0) break;
      iVar2 = iVar2 + -1;
      bVar13 = *pcVar6 == *pcVar10;
      pcVar6 = pcVar6 + 1;
      pcVar10 = pcVar10 + 1;
    } while (bVar13);
    if (!bVar13) {
      _free(local_58[0]);
      MessageBoxA((HWND)0x0,"Could not load sound (2)","HEY",0);
      return;
    }
    local_34 = *(undefined2 *)(puVar3 + 5);
    local_30 = puVar3[6];
    local_40 = (int *)CONCAT22((short)((uint)puVar3[2] >> 0x10),local_34);
    local_32 = *(short *)((int)puVar3 + 0x16);
    pcVar6 = (char *)((int)puVar3 + 0x22);
    do {
      if ((((*pcVar6 == 'd') && (pcVar6[1] == 'a')) && (pcVar6[2] == 't')) && (pcVar6[3] == 'a')) {
        uVar1 = *(undefined4 *)(pcVar6 + 4);
        local_26 = *(short *)(puVar3 + 8) * 8;
        local_28 = *(short *)(puVar3 + 8) * local_32;
        local_2c = (int *)((uint)local_28 * local_30);
        iVar2 = *(int *)((int)this + 4);
        puVar3 = local_24;
        for (iVar4 = 9; iVar4 != 0; iVar4 = iVar4 + -1) {
          *puVar3 = 0;
          puVar3 = puVar3 + 1;
        }
        local_14 = &local_34;
        local_24[0] = 0x24;
        local_24[1] = 0x82;
        piVar16 = *(int **)(iVar2 + 0x84c);
        local_24[2] = uVar1;
        iVar2 = (**(code **)(*piVar16 + 0xc))();
        if (iVar2 != 0) {
          MessageBoxA((HWND)0x0,"Could not load sound (4)","HEY",0);
          return;
        }
        _Memory = (void *)0x2;
        puVar3 = (undefined4 *)&stack0xffffffa4;
        puVar14 = &stack0xffffffa0;
        iVar2 = (**(code **)(*piStack_c + 0x2c))(piStack_c,0,uVar1,local_58,&stack0xffffff9c);
        if (iVar2 != 0) {
          (**(code **)(*local_2c + 8))(local_2c);
          local_30 = 0;
          MessageBoxA((HWND)0x0,"Could not load sound (5)","HEY",0);
          return;
        }
        piVar8 = (int *)(pcVar6 + 8);
        piVar11 = piVar16;
        for (uVar5 = (uint)puVar14 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
          *piVar11 = *piVar8;
          piVar8 = piVar8 + 1;
          piVar11 = piVar11 + 1;
        }
        for (uVar5 = (uint)puVar14 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
          *(char *)piVar11 = (char)*piVar8;
          piVar8 = (int *)((int)piVar8 + 1);
          piVar11 = (int *)((int)piVar11 + 1);
        }
        if (puVar3 != (undefined4 *)0x0) {
          puVar9 = (undefined4 *)(puVar14 + (int)(pcVar6 + 8));
          puVar12 = puVar3;
          for (uVar5 = (uint)_Memory >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
            *puVar12 = *puVar9;
            puVar9 = puVar9 + 1;
            puVar12 = puVar12 + 1;
          }
          for (uVar5 = (uint)_Memory & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
            *(undefined1 *)puVar12 = *(undefined1 *)puVar9;
            puVar9 = (undefined4 *)((int)puVar9 + 1);
            puVar12 = (undefined4 *)((int)puVar12 + 1);
          }
        }
        pvVar15 = _Memory;
        iVar2 = (**(code **)(*local_2c + 0x4c))(local_2c,piVar16,puVar14,puVar3);
        if (iVar2 == 0) {
          puVar3 = operator_new(0xc);
          if (puVar3 == (undefined4 *)0x0) {
            puVar3 = (undefined4 *)0x0;
          }
          else {
            uVar1 = *(undefined4 *)((int)this + 4);
            *puVar3 = &PTR_SoundEntry_ScalarDtor_004d8e78;
            puVar3[2] = uVar1;
            puVar3[1] = local_40;
          }
          AthenaList_Append((void *)((int)this + 8),(int)puVar3);
          uVar5 = local_3c;
          sVar7 = 0;
          if (0 < (int)local_3c) {
            do {
              piVar16 = *(int **)(*(int *)((int)this + 4) + 0x84c);
              (**(code **)(*piVar16 + 0x14))(piVar16,local_40,&stack0xffffff84);
              puVar3 = operator_new(0xc);
              if (puVar3 == (undefined4 *)0x0) {
                puVar3 = (undefined4 *)0x0;
              }
              else {
                uVar1 = *(undefined4 *)((int)this + 4);
                *puVar3 = &PTR_SoundEntry_ScalarDtor_004d8e78;
                puVar3[2] = uVar1;
                puVar3[1] = pvVar15;
              }
              AthenaList_Append((void *)((int)this + 8),(int)puVar3);
              sVar7 = sVar7 + 1;
            } while ((int)sVar7 < (int)uVar5);
          }
          _free(_Memory);
          return;
        }
        (**(code **)(*local_40 + 8))(local_40);
        local_44 = 0;
        MessageBoxA((HWND)0x0,"Could not load sound (6)","HEY",0);
        return;
      }
      pcVar6 = pcVar6 + 1;
    } while ((int)pcVar6 - (int)local_58[0] < (int)local_44);
    _free(local_58[0]);
    MessageBoxA((HWND)0x0,"Could not load sound (3)","HEY",0);
  }
  return;
}

