# MESHWORLD Parser Decompilation

Function at 0x00470930, called from LoadMeshWorld (0x0045de30)

## LoadMeshWorld (0x0045de30)

```c
undefined4 __thiscall LoadMeshWorld(void *this, char *param_1)
{
  // Formats filename with %s.meshworld
  // If file exists: creates MeshWorld obj (0x488 bytes), calls constructor at 0x004706e0
  // Then calls Parse at 0x00470930
  // If load fails: displays "COULD NOT LOAD" message box and exits
}
```

## MeshWorld Constructor (0x004706e0)

- vtable at PTR_FUN_004d9cdc
- Object is 0x488 bytes
- Initializes material array, fog colors, default params

## MeshWorld::Parse (0x00470930) - Full Decompilation

```c
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint __thiscall FUN_00470930(void *this,char *param_1,char param_2)

{
  char cVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  bool bVar5;
  bool bVar6;
  bool bVar7;
  undefined4 *puVar8;
  uint uVar9;
  byte *pbVar10;
  uint *_Str;
  long lVar11;
  long *plVar12;
  char *pcVar13;
  uint *puVar14;
  float *pfVar15;
  long lVar16;
  undefined4 *puVar17;
  undefined4 *puVar18;
  undefined4 uVar19;
  int iVar20;
  int iVar21;
  int unaff_EBP;
  short sVar22;
  short sVar23;
  long *plVar24;
  bool bVar25;
  double dVar26;
  double dVar27;
  int local_5b8;
  undefined4 *local_5b0;
  undefined4 *local_5a8;
  void *local_5a4;
  void *local_5a0;
  int local_59c;
  undefined4 local_594;
  undefined4 uStack_590;
  undefined4 uStack_58c;
  undefined4 uStack_588;
  undefined4 uStack_584;
  undefined4 uStack_580;
  undefined4 uStack_57c;
  undefined4 local_578;
  undefined4 uStack_574;
  undefined4 uStack_570;
  float fStack_554;
  float fStack_550;
  float fStack_54c;
  float local_548;
  float local_544;
  float local_540;
  float local_53c;
  float local_538;
  float local_534;
  float local_530;
  float local_52c;
  undefined1 local_528 [4];
  char local_524 [4];
  undefined1 local_520 [252];
  undefined4 auStack_424 [262];
  void *local_c;
  undefined1 *puStack_8;
  uint local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd8b0;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  FUN_0047d670(&local_594);
  local_4 = 0;
  puVar8 = operator_new(0x874);
  local_4._0_1_ = 1;
  if (puVar8 == (undefined4 *)0x0) {
    local_5a8 = (undefined4 *)0x0;
  }
  else {
    local_5a8 = FUN_00458970(puVar8);
  }
  local_4 = (uint)local_4._1_3_ << 8;
  iVar20 = -(int)param_1;
  do {
    cVar1 = *param_1;
    param_1[(int)(local_524 + iVar20)] = cVar1;
    param_1 = param_1 + 1;
  } while (cVar1 != '\0');
  puVar8 = (undefined4 *)(local_528 + 3);
  do {
    puVar17 = puVar8;
    puVar8 = (undefined4 *)((int)puVar17 + 1);
  } while (*(char *)((int)puVar17 + 1) != '\0');
  *(undefined4 *)((int)puVar17 + 1) = DAT_004d9e58;
  *(undefined1 *)((int)puVar17 + 5) = DAT_004d9e5c;
  bVar7 = FUN_0047d7c0(&local_594,local_524);
  if (bVar7) {
    bVar7 = false;
    bVar5 = false;
    bVar6 = false;
    pbVar10 = FUN_0047d6b0(&local_594,'\x01');
    while (pbVar10 != (byte *)0x0) {
      _Str = (uint *)FUN_004bc0d1(pbVar10,&DAT_004d9e54);
      iVar20 = 0x10;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*MATERIAL_COUNT";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if (bVar25) {
        _Str = (uint *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        lVar11 = _atol((char *)_Str);
        *(long *)((int)this + 0x24) = lVar11;
        lVar11 = _atol((char *)_Str);
        plVar12 = operator_new(lVar11 * 0x50 + 4);
        local_4._0_1_ = 2;
        if (plVar12 == (long *)0x0) {
          plVar24 = (long *)0x0;
        }
        else {
          plVar24 = plVar12 + 1;
          *plVar12 = lVar11;
          _eh_vector_constructor_iterator_(plVar24,0x50,lVar11,FUN_00457fa0,FUN_00457fd0);
        }
        local_4 = (uint)local_4._1_3_ << 8;
        *(long **)((int)this + 0x28) = plVar24;
      }
      iVar20 = 10;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*MATERIAL";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if (bVar25) {
        _Str = (uint *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        local_5b8 = _atol((char *)_Str);
        bVar7 = true;
        bVar5 = true;
        bVar6 = true;
      }
      iVar20 = 0x12;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*MATERIAL_AMBIENT";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if ((bVar25) && (bVar7)) {
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        iVar20 = (short)local_5b8 * 0x50;
        dVar26 = _atof(pcVar13);
        *(float *)(iVar20 + 0x14 + *(int *)((int)this + 0x28)) = (float)dVar26;
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar26 = _atof(pcVar13);
        *(float *)(iVar20 + 0x18 + *(int *)((int)this + 0x28)) = (float)dVar26;
        _Str = (uint *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar26 = _atof((char *)_Str);
        *(float *)(iVar20 + 0x1c + *(int *)((int)this + 0x28)) = (float)dVar26;
        *(undefined4 *)(iVar20 + 0x20 + *(int *)((int)this + 0x28)) = 0x3f800000;
        bVar7 = false;
      }
      iVar20 = 0x12;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*MATERIAL_DIFFUSE";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if ((bVar25) && (bVar5)) {
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        iVar20 = (short)local_5b8 * 0x50;
        dVar26 = _atof(pcVar13);
        *(float *)(iVar20 + 4 + *(int *)((int)this + 0x28)) = (float)dVar26;
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar26 = _atof(pcVar13);
        *(float *)(iVar20 + 8 + *(int *)((int)this + 0x28)) = (float)dVar26;
        _Str = (uint *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar26 = _atof((char *)_Str);
        *(float *)(iVar20 + 0xc + *(int *)((int)this + 0x28)) = (float)dVar26;
        *(undefined4 *)(iVar20 + 0x10 + *(int *)((int)this + 0x28)) = 0x3f800000;
        bVar5 = false;
      }
      iVar20 = 0xd;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*MAP_REFLECT";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if (bVar25) {
        *(undefined1 *)((short)local_5b8 * 0x50 + 0x4d + *(int *)((int)this + 0x28)) = 1;
      }
      iVar20 = 0x13;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*MATERIAL_SPECULAR";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if ((bVar25) && (bVar6)) {
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        iVar20 = (short)local_5b8 * 0x50;
        dVar26 = _atof(pcVar13);
        *(float *)(iVar20 + 0x24 + *(int *)((int)this + 0x28)) = (float)dVar26;
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar26 = _atof(pcVar13);
        *(float *)(iVar20 + 0x28 + *(int *)((int)this + 0x28)) = (float)dVar26;
        _Str = (uint *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar26 = _atof((char *)_Str);
        *(float *)(iVar20 + 0x2c + *(int *)((int)this + 0x28)) = (float)dVar26;
        *(undefined4 *)(iVar20 + 0x30 + *(int *)((int)this + 0x28)) = 0x3f800000;
        bVar6 = false;
      }
      iVar20 = 0xe;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*MATERIAL_REF";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if (bVar25) {
        _Str = (uint *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        lVar11 = _atol((char *)_Str);
        local_5a8[1] = lVar11;
      }
      iVar20 = 0x18;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*MATERIAL_SHINESTRENGTH";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if (bVar25) {
        _Str = (uint *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        iVar20 = (short)local_5b8 * 0x50;
        pfVar15 = (float *)(iVar20 + 0x24 + *(int *)((int)this + 0x28));
        dVar26 = _atof((char *)_Str);
        *pfVar15 = (float)dVar26 * *pfVar15;
        pfVar15 = (float *)(iVar20 + 0x28 + *(int *)((int)this + 0x28));
        dVar26 = _atof((char *)_Str);
        *pfVar15 = (float)dVar26 * *pfVar15;
        pfVar15 = (float *)(iVar20 + 0x2c + *(int *)((int)this + 0x28));
        dVar26 = _atof((char *)_Str);
        *pfVar15 = (float)dVar26 * *pfVar15;
        pfVar15 = (float *)(iVar20 + 0x30 + *(int *)((int)this + 0x28));
        dVar26 = _atof((char *)_Str);
        *pfVar15 = (float)dVar26 * *pfVar15;
      }
      iVar20 = 0x10;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*MATERIAL_SHINE";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if (bVar25) {
        _Str = (uint *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar26 = _atof((char *)_Str);
        *(float *)((short)local_5b8 * 0x50 + 0x44 + *(int *)((int)this + 0x28)) =
             (float)dVar26 * _DAT_004cf454;
      }
      iVar20 = 0x17;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*MATERIAL_TRANSPARENCY";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if (bVar25) {
        _Str = (uint *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        iVar20 = (short)local_5b8 * 0x50;
        dVar26 = _atof((char *)_Str);
        *(float *)(iVar20 + 0x10 + *(int *)((int)this + 0x28)) = _DAT_004cf310 - (float)dVar26;
        dVar26 = _atof((char *)_Str);
        *(float *)(iVar20 + 0x20 + *(int *)((int)this + 0x28)) = _DAT_004cf310 - (float)dVar26;
        dVar26 = _atof((char *)_Str);
        *(float *)(iVar20 + 0x40 + *(int *)((int)this + 0x28)) = _DAT_004cf310 - (float)dVar26;
        dVar26 = _atof((char *)_Str);
        *(float *)(iVar20 + 0x30 + *(int *)((int)this + 0x28)) = _DAT_004cf310 - (float)dVar26;
        dVar26 = _atof((char *)_Str);
        if ((double)_DAT_004cf310 - dVar26 != _DAT_004cf3c8) {
          *(undefined1 *)(iVar20 + 0x4c + *(int *)((int)this + 0x28)) = 1;
        }
      }
      iVar20 = 8;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*BITMAP";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if (bVar25) {
        uVar9 = FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        _Str = (uint *)(uVar9 + 1);
        puVar14 = FUN_004bacc0(_Str,'\"');
        if (puVar14 != (uint *)0x0) {
          *(undefined1 *)puVar14 = 0;
        }
        puVar8 = FUN_00455c50(*(void **)((int)this + 4),(char *)_Str,'\x01');
        *(undefined4 **)((short)local_5b8 * 0x50 + 0x48 + *(int *)((int)this + 0x28)) = puVar8;
      }
      iVar20 = 0xc;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*GEOMOBJECT";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if (bVar25) {
        puVar8 = operator_new(0x874);
        local_4._0_1_ = 3;
        if (puVar8 == (undefined4 *)0x0) {
          local_5a8 = (undefined4 *)0x0;
        }
        else {
          local_5a8 = FUN_00458970(puVar8);
        }
        local_4 = (uint)local_4._1_3_ << 8;
        FUN_00453280((int)(local_5a8 + 3));
        *(undefined1 *)(local_5a8 + 0x217) = 0;
        FUN_00453780((void *)((int)this + 0x2c),(int)local_5a8);
      }
      iVar20 = 0xb;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*NODE_NAME";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if (bVar25) {
        uVar9 = FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        _Str = (uint *)(uVar9 + 1);
        puVar14 = FUN_004bacc0(_Str,'\"');
        *(undefined1 *)puVar14 = 0;
      }
      iVar20 = 8;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*TM_POS";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if (bVar25) {
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar26 = _atof(pcVar13);
        local_5a8[0x21a] = (float)dVar26;
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar26 = _atof(pcVar13);
        local_5a8[0x21c] = (float)dVar26;
        _Str = (uint *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar26 = _atof((char *)_Str);
        local_5a8[0x21b] = (float)dVar26;
      }
      iVar20 = 0x10;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*MESH_NUMVERTEX";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if (bVar25) {
        _Str = (uint *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        lVar11 = _atol((char *)_Str);
        local_5a0 = operator_new(lVar11 << 5);
        if (0 < lVar11) {
          puVar8 = (undefined4 *)((int)local_5a0 + 0x10);
          do {
            puVar8[-1] = 0;
            *puVar8 = 0;
            puVar8[1] = 0;
            puVar8[-4] = 0;
            puVar8[-3] = 0;
            puVar8[-2] = 0;
            puVar8[2] = 0;
            puVar8[3] = 0;
            puVar8 = puVar8 + 8;
            lVar11 = lVar11 + -1;
          } while (lVar11 != 0);
        }
      }
      iVar20 = 0x11;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*MESH_NUMTVERTEX";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if (bVar25) {
        _Str = (uint *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        lVar11 = _atol((char *)_Str);
        local_5a4 = operator_new(lVar11 * 8);
        iVar20 = 0;
        if (0 < lVar11) {
          do {
            *(undefined4 *)((int)local_5a4 + iVar20 * 8) = 0;
            *(undefined4 *)((int)local_5a4 + iVar20 * 8 + 4) = 0;
            iVar20 = iVar20 + 1;
          } while (iVar20 < lVar11);
        }
      }
      iVar20 = 0xd;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*MESH_VERTEX";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if (bVar25) {
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        lVar11 = _atol(pcVar13);
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar26 = _atof(pcVar13);
        fVar2 = (float)dVar26;
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar26 = _atof(pcVar13);
        fVar3 = (float)dVar26;
        _Str = (uint *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar26 = _atof((char *)_Str);
        fVar4 = (float)dVar26;
        pfVar15 = (float *)((int)local_5a0 + lVar11 * 0x20);
        pfVar15[1] = fVar4;
        *pfVar15 = fVar2;
        pfVar15[2] = fVar3;
        if (fVar2 < *(float *)((int)this + 0x45c)) {
          *(float *)((int)this + 0x45c) = fVar2;
        }
        if (fVar4 < *(float *)((int)this + 0x460)) {
          *(float *)((int)this + 0x460) = fVar4;
        }
        if (fVar3 < *(float *)((int)this + 0x464)) {
          *(float *)((int)this + 0x464) = fVar3;
        }
        if (*(float *)((int)this + 0x468) < fVar2) {
          *(float *)((int)this + 0x468) = fVar2;
        }
        if (*(float *)((int)this + 0x46c) < fVar4) {
          *(float *)((int)this + 0x46c) = fVar4;
        }
        if (*(float *)((int)this + 0x470) < fVar3) {
          *(float *)((int)this + 0x470) = fVar3;
        }
      }
      iVar20 = 0xf;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*MESH_NUMFACES";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if (bVar25) {
        _Str = (uint *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        lVar11 = _atol((char *)_Str);
        local_5b0 = operator_new(lVar11 * 0x60);
        puVar8 = local_5b0;
        for (uVar9 = (uint)(lVar11 * 0x60) >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
          *puVar8 = 0;
          puVar8 = puVar8 + 1;
        }
        for (iVar20 = 0; iVar20 != 0; iVar20 = iVar20 + -1) {
          *(undefined1 *)puVar8 = 0;
          puVar8 = (undefined4 *)((int)puVar8 + 1);
        }
      }
      iVar20 = 0xc;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*MESH_TVERT";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if (bVar25) {
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        lVar11 = _atol(pcVar13);
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar26 = _atof(pcVar13);
        _Str = (uint *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar27 = _atof((char *)_Str);
        *(float *)((int)local_5a4 + lVar11 * 8) = (float)dVar26;
        *(float *)((int)local_5a4 + lVar11 * 8 + 4) = (float)dVar27;
      }
      iVar20 = 0xb;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*MESH_FACE";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if (bVar25) {
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        lVar11 = _atol(pcVar13);
        FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        lVar16 = _atol(pcVar13);
        puVar17 = (undefined4 *)(lVar16 * 0x20 + (int)local_5a0);
        puVar8 = local_5b0 + lVar11 * 0x18;
        *puVar8 = *puVar17;
        puVar8[1] = puVar17[1];
        puVar8[2] = puVar17[2];
        puVar8[6] = puVar17[6];
        puVar8[7] = puVar17[7];
        FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        lVar11 = _atol(pcVar13);
        iVar20 = lVar11 * 0x20;
        puVar8[0x10] = *(undefined4 *)(iVar20 + (int)local_5a0);
        puVar8[0x11] = *(undefined4 *)((int)local_5a0 + iVar20 + 4);
        puVar8[0x12] = *(undefined4 *)((int)local_5a0 + iVar20 + 8);
        puVar8[0x16] = *(undefined4 *)((int)local_5a0 + iVar20 + 0x18);
        puVar8[0x17] = *(undefined4 *)((int)local_5a0 + iVar20 + 0x1c);
        FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        _Str = (uint *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        lVar11 = _atol((char *)_Str);
        iVar20 = lVar11 * 0x20;
        puVar8[8] = *(undefined4 *)(iVar20 + (int)local_5a0);
        puVar8[9] = *(undefined4 *)((int)local_5a0 + iVar20 + 4);
        puVar8[10] = *(undefined4 *)((int)local_5a0 + iVar20 + 8);
        puVar8[0xe] = *(undefined4 *)((int)local_5a0 + iVar20 + 0x18);
        puVar8[0xf] = *(undefined4 *)((int)local_5a0 + iVar20 + 0x1c);
        FUN_00453780(local_5a8 + 3,(int)puVar8);
      }
      iVar20 = 0xc;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*MESH_TFACE";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if (bVar25) {
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        lVar11 = _atol(pcVar13);
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        lVar16 = _atol(pcVar13);
        local_5b0[lVar11 * 0x18 + 6] = *(undefined4 *)((int)local_5a4 + lVar16 * 8);
        local_5b0[lVar11 * 0x18 + 7] =
             (float)_DAT_004cf3c8 - *(float *)((int)local_5a4 + lVar16 * 8 + 4);
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        lVar16 = _atol(pcVar13);
        local_5b0[lVar11 * 0x18 + 0x16] = *(undefined4 *)((int)local_5a4 + lVar16 * 8);
        local_5b0[lVar11 * 0x18 + 0x17] =
             (float)_DAT_004cf3c8 - *(float *)((int)local_5a4 + lVar16 * 8 + 4);
        _Str = (uint *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        lVar16 = _atol((char *)_Str);
        local_5b0[lVar11 * 0x18 + 0xe] = *(undefined4 *)((int)local_5a4 + lVar16 * 8);
        local_5b0[lVar11 * 0x18 + 0xf] =
             (float)_DAT_004cf3c8 - *(float *)((int)local_5a4 + lVar16 * 8 + 4);
      }
      iVar20 = 0x11;
      bVar25 = true;
      puVar14 = _Str;
      pcVar13 = "*MESH_FACENORMAL";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*puVar14 == *pcVar13;
        puVar14 = (uint *)((int)puVar14 + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if (bVar25) {
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        lVar11 = _atol(pcVar13);
        local_59c = 0;
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar26 = _atof(pcVar13);
        local_548 = (float)dVar26;
        dVar26 = _atof(pcVar13);
        local_53c = (float)dVar26;
        dVar26 = _atof(pcVar13);
        local_530 = (float)dVar26;
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar26 = _atof(pcVar13);
        local_544 = (float)dVar26;
        dVar26 = _atof(pcVar13);
        local_538 = (float)dVar26;
        dVar26 = _atof(pcVar13);
        local_52c = (float)dVar26;
        _Str = (uint *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar26 = _atof((char *)_Str);
        local_540 = (float)dVar26;
        dVar26 = _atof((char *)_Str);
        local_534 = (float)dVar26;
        dVar26 = _atof((char *)_Str);
        local_528 = (undefined1  [4])(float)dVar26;
        thunk_FUN_00459b24();
        thunk_FUN_00459b24();
        thunk_FUN_00459b24();
        iVar20 = lVar11 * 0x60 + unaff_EBP;
        *(undefined4 *)(iVar20 + 0xc) = uStack_590;
        *(undefined4 *)(iVar20 + 0x2c) = uStack_584;
        *(undefined4 *)(iVar20 + 0x4c) = local_578;
        *(undefined4 *)(iVar20 + 0x10) = uStack_588;
        *(undefined4 *)(iVar20 + 0x30) = uStack_57c;
        *(undefined4 *)(iVar20 + 0x50) = uStack_570;
        *(undefined4 *)(iVar20 + 0x14) = uStack_58c;
        *(undefined4 *)(iVar20 + 0x34) = uStack_580;
        *(undefined4 *)(iVar20 + 0x54) = uStack_574;
      }
      iVar20 = 0x13;
      bVar25 = true;
      pcVar13 = "*MESH_VERTEXNORMAL";
      do {
        if (iVar20 == 0) break;
        iVar20 = iVar20 + -1;
        bVar25 = (char)*_Str == *pcVar13;
        _Str = (uint *)((int)_Str + 1);
        pcVar13 = pcVar13 + 1;
      } while (bVar25);
      if (bVar25) {
        FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        sVar22 = 0;
        sVar23 = (short)local_59c;
        if (sVar23 != 0) {
          if (sVar23 == 1) {
            sVar22 = 2;
          }
          else if (sVar23 == 2) {
            sVar22 = 1;
          }
        }
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar26 = _atof(pcVar13);
        fStack_554 = (float)dVar26;
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar26 = _atof(pcVar13);
        fStack_550 = (float)dVar26;
        pcVar13 = (char *)FUN_004bc0d1((byte *)0x0,&DAT_004d9e54);
        dVar26 = _atof(pcVar13);
        fStack_54c = (float)dVar26;
        thunk_FUN_00459b24();
        iVar20 = ((short)local_5a0 * 3 + (int)sVar22) * 0x20 + local_5b8;
        *(undefined4 *)(iVar20 + 0xc) = uStack_58c;
        *(undefined4 *)(iVar20 + 0x10) = uStack_584;
        local_5a4 = (void *)(local_59c + 1);
        *(undefined4 *)(iVar20 + 0x14) = uStack_588;
      }
      pbVar10 = FUN_0047d6b0(&local_594,'\x01');
    }
    _free(local_5a0);
    _free(local_5a4);
    if (param_2 == '\x01') {
      FUN_00470680((int)this);
    }
    else {
      *(undefined4 *)((int)this + 0x34) = 0;
      if (*(int *)((int)this + 0x30) < 1) {
        iVar20 = 0;
      }
      else {
        iVar20 = **(int **)((int)this + 0x438);
        *(undefined4 *)((int)this + 0x34) = 1;
      }
      while (iVar20 != 0) {
        puVar8 = (undefined4 *)0x0;
        FUN_00453210(auStack_424,0);
        local_4 = CONCAT31(local_4._1_3_,4);
        FUN_00453280((int)auStack_424);
        *(undefined4 *)(iVar20 + 0x14) = 0;
        if (0 < *(int *)(iVar20 + 0x10)) {
          puVar8 = (undefined4 *)**(undefined4 **)(iVar20 + 0x418);
          *(undefined4 *)(iVar20 + 0x14) = 1;
        }
        while (puVar8 != (undefined4 *)0x0) {
          puVar18 = operator_new(0x60);
          puVar17 = puVar18;
          for (iVar21 = 0x18; iVar21 != 0; iVar21 = iVar21 + -1) {
            *puVar17 = *puVar8;
            puVar8 = puVar8 + 1;
            puVar17 = puVar17 + 1;
          }
          FUN_00453780(auStack_424,(int)puVar18);
          iVar21 = *(int *)(iVar20 + 0x14);
          if (*(int *)(iVar20 + 0x10) <= iVar21) break;
          puVar8 = *(undefined4 **)(*(int *)(iVar20 + 0x418) + iVar21 * 4);
          *(int *)(iVar20 + 0x14) = iVar21 + 1;
        }
        FUN_00453280(iVar20 + 0xc);
        FUN_00453820((void *)(iVar20 + 0xc),(int)auStack_424);
        local_4 = local_4 & 0xffffff00;
        FUN_00453250(auStack_424);
        iVar21 = *(int *)((int)this + 0x34);
        if (*(int *)((int)this + 0x30) <= iVar21) break;
        iVar20 = *(int *)(*(int *)((int)this + 0x438) + iVar21 * 4);
        *(int *)((int)this + 0x34) = iVar21 + 1;
      }
    }
    _free(local_5b0);
    local_4 = 0xffffffff;
    uVar19 = FUN_0047d680(&local_594);
    uVar9 = CONCAT31((int3)((uint)uVar19 >> 8),1);
  }
  else {
    local_4 = 0xffffffff;
    uVar9 = FUN_0047d680(&local_594);
    uVar9 = uVar9 & 0xffffff00;
  }
  ExceptionList = local_c;
  return uVar9;
}
```
