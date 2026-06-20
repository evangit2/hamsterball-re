
void __thiscall Level_LoadCollision(void *this,int param_1)

{
  float fVar1;
  void *pvVar2;
  int *piVar3;
  undefined4 *puVar4;
  float *_DstBuf;
  int iVar5;
  int local_8c;
  size_t local_88;
  int local_84;
  int local_80;
  int local_7c;
  int local_78;
  undefined4 *local_74;
  int *local_70;
  float local_6c;
  float fStack_68;
  float fStack_64;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  float local_40;
  float local_3c;
  float local_38;
  float local_34;
  float fStack_30;
  float fStack_2c;
  float fStack_20;
  float fStack_1c;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd1c1;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  local_70 = this;
  local_74 = operator_new(0x488);
  iVar5 = 0;
  local_4 = 0;
  if (local_74 == (undefined4 *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(local_74,*(undefined4 *)((int)this + 4));
  }
  *(void **)((int)this + 8) = pvVar2;
  *(undefined1 *)((int)pvVar2 + 0x459) = 0;
  local_4 = 0xffffffff;
  *(undefined1 *)((int)this + 0xd) = 1;
  __read(param_1,(void *)(*(int *)((int)this + 8) + 0x45c),0x18);
  __read(param_1,&local_8c,4);
  if (local_8c < 1) {
    __read(param_1,&local_78,4);
    local_7c = 0;
    if (0 < local_78) {
      do {
        local_74 = operator_new(0x874);
        local_4 = 2;
        if (local_74 == (undefined4 *)0x0) {
          puVar4 = (undefined4 *)0x0;
        }
        else {
          puVar4 = CreateMeshBuffer(local_74);
        }
        *(undefined1 *)(puVar4 + 0x217) = 0;
        local_4 = 0xffffffff;
        AthenaList_Append((void *)(*(int *)((int)this + 8) + 0x2c),(int)puVar4);
        __read(param_1,&local_88,4);
        if (0 < (int)local_88) {
          pvVar2 = _malloc(local_88);
          puVar4[0x219] = pvVar2;
          __read(param_1,pvVar2,local_88);
          iVar5 = __strnicmp((char *)puVar4[0x219],"N:",2);
          if (iVar5 == 0) {
            *(undefined1 *)((int)puVar4 + 0x85d) = 1;
          }
          iVar5 = __strnicmp((char *)puVar4[0x219],"E:",2);
          if (iVar5 == 0) {
            *(undefined1 *)((int)puVar4 + 0x85d) = 1;
            *(undefined1 *)((int)puVar4 + 0x863) = 1;
          }
        }
        __read(param_1,&local_80,4);
        local_84 = 0;
        if (0 < local_80) {
          local_74 = puVar4 + 3;
          do {
            _DstBuf = operator_new(0x60);
            __read(param_1,_DstBuf,4);
            __read(param_1,_DstBuf + 1,4);
            __read(param_1,_DstBuf + 2,4);
            __read(param_1,_DstBuf + 8,4);
            __read(param_1,_DstBuf + 9,4);
            __read(param_1,_DstBuf + 10,4);
            __read(param_1,_DstBuf + 0x10,4);
            __read(param_1,_DstBuf + 0x11,4);
            __read(param_1,_DstBuf + 0x12,4);
            fVar1 = _DstBuf[1];
            fStack_20 = _DstBuf[2];
            puStack_8 = (undefined1 *)_DstBuf[10];
            local_4c = _DstBuf[0x10];
            local_58 = *_DstBuf - _DstBuf[8];
            local_48 = _DstBuf[0x11];
            local_44 = _DstBuf[0x12];
            local_54 = fVar1 - _DstBuf[9];
            local_50 = fStack_20 - (float)puStack_8;
            fStack_1c = *_DstBuf - local_4c;
            fVar1 = fVar1 - local_48;
            local_40 = fVar1 * local_50 - (fStack_20 - local_44) * local_54;
            local_3c = (fStack_20 - local_44) * local_58 - local_50 * fStack_1c;
            local_38 = local_54 * fStack_1c - fVar1 * local_58;
            local_34 = local_40;
            fStack_30 = local_3c;
            fStack_2c = local_38;
            Graphics_InitShaderDispatch();
            _DstBuf[3] = local_6c;
            _DstBuf[4] = -fStack_68;
            _DstBuf[5] = fStack_64;
            _DstBuf[0xb] = local_6c;
            _DstBuf[0xc] = -fStack_68;
            _DstBuf[0xd] = fStack_64;
            _DstBuf[0x13] = local_6c;
            _DstBuf[0x14] = -fStack_68;
            _DstBuf[0x15] = fStack_64;
            AthenaList_Append(local_74,(int)_DstBuf);
            local_84 = local_84 + 1;
            this = local_70;
          } while (local_84 < local_80);
        }
        local_7c = local_7c + 1;
      } while (local_7c < local_78);
    }
  }
  else {
    *(undefined1 *)((int)this + 0x430) = 1;
    do {
      piVar3 = operator_new(0x10d0);
      local_4 = 1;
      local_70 = piVar3;
      if (piVar3 == (int *)0x0) {
        piVar3 = (int *)0x0;
      }
      else {
        Level_ctor(piVar3,*(undefined4 *)((int)this + 4));
        *piVar3 = (int)&PTR_Level_DeletingDtor2_004d9068;
      }
      local_4 = 0xffffffff;
      AthenaList_Append((void *)((int)this + 0x18),(int)piVar3);
      (**(code **)(*piVar3 + 0x60))(param_1);
      iVar5 = iVar5 + 1;
    } while (iVar5 < local_8c);
  }
  ExceptionList = local_c;
  return;
}

