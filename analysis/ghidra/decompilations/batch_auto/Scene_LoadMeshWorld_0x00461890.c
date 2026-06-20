
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Scene_LoadMeshWorld(void *this,int param_1,int param_2)

{
  code *pcVar1;
  void *pvVar2;
  int *piVar3;
  undefined4 *puVar4;
  int iVar5;
  char *pcVar6;
  int iVar7;
  undefined4 local_160;
  int *local_15c;
  undefined4 local_158;
  float local_154;
  undefined4 local_150;
  int local_14c;
  int local_148;
  int local_144;
  int local_140;
  int local_13c;
  size_t local_138;
  int local_134;
  undefined4 local_130;
  int local_12c;
  undefined4 local_128;
  uint local_124;
  undefined4 local_120;
  undefined4 local_11c;
  undefined4 local_118;
  int local_114;
  undefined4 *local_110;
  char local_10c [256];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd026;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(int *)((int)this + 0x47c) = param_2;
  *(undefined4 *)((int)this + 0x434) = *(undefined4 *)(param_2 + 0x434);
  local_15c = operator_new(0x488);
  iVar7 = 0;
  local_4 = 0;
  if (local_15c == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(local_15c,*(undefined4 *)((int)this + 4));
  }
  *(void **)((int)this + 8) = pvVar2;
  *(undefined1 *)((int)pvVar2 + 0x459) = 1;
  local_4 = 0xffffffff;
  *(undefined1 *)((int)this + 0xd) = 1;
  __read(param_1,(void *)(*(int *)((int)this + 8) + 0x45c),0x18);
  __read(param_1,&local_13c,4);
  if (local_13c < 1) {
    __read(param_1,&local_148,4);
    *(int *)(*(int *)((int)this + 8) + 0x24) = local_148;
    iVar7 = *(int *)(*(int *)((int)this + 8) + 0x24);
    local_15c = operator_new(iVar7 * 0x50 + 4);
    local_4 = 2;
    if (local_15c == (int *)0x0) {
      piVar3 = (int *)0x0;
    }
    else {
      piVar3 = local_15c + 1;
      *local_15c = iVar7;
      _eh_vector_constructor_iterator_(piVar3,0x50,iVar7,RenderContext_Init,Matrix4_Identity);
    }
    *(int **)(*(int *)((int)this + 8) + 0x28) = piVar3;
    local_4 = 0xffffffff;
    local_14c = 0;
    if (0 < local_148) {
      local_144 = 0;
      do {
        iVar7 = local_144;
        local_15c = operator_new(0x874);
        local_4 = 3;
        if (local_15c == (int *)0x0) {
          puVar4 = (undefined4 *)0x0;
        }
        else {
          puVar4 = CreateMeshBuffer(local_15c);
        }
        *(undefined1 *)(puVar4 + 0x217) = 1;
        local_4 = 0xffffffff;
        AthenaList_Append((void *)(*(int *)((int)this + 8) + 0x2c),(int)puVar4);
        __read(param_1,&local_138,4);
        if (0x100 < (int)local_138) {
          CRT_FlsAlloc(0);
          pcVar1 = (code *)swi(3);
          (*pcVar1)();
          return;
        }
        pvVar2 = _malloc(local_138);
        puVar4[0x219] = pvVar2;
        __read(param_1,pvVar2,local_138);
        if ((char *)puVar4[0x219] != (char *)0x0) {
          iVar5 = __strnicmp("O:",(char *)puVar4[0x219],2);
          if (iVar5 == 0) {
            *(undefined1 *)((int)puVar4 + 0x862) = 1;
          }
          iVar5 = __strnicmp("T:",(char *)puVar4[0x219],2);
          if (iVar5 == 0) {
            *(undefined1 *)((int)puVar4 + 0x85f) = 1;
          }
          iVar5 = __strnicmp("N:GLASS",(char *)puVar4[0x219],7);
          if (iVar5 == 0) {
            *(undefined1 *)(puVar4 + 0x218) = 1;
          }
          iVar5 = __strnicmp("E:",(char *)puVar4[0x219],2);
          if (iVar5 == 0) {
            *(undefined1 *)((int)puVar4 + 0x863) = 1;
          }
          pcVar6 = strstr((char *)puVar4[0x219],"(NOSHADOW)");
          if (pcVar6 != (char *)0x0) {
            *(undefined1 *)((int)puVar4 + 0x85e) = 1;
          }
          pcVar6 = strstr((char *)puVar4[0x219],"(WANTZ)");
          if (pcVar6 != (char *)0x0) {
            *(undefined1 *)((int)puVar4 + 0x861) = 1;
          }
        }
        puVar4[1] = local_14c;
        __read(param_1,&local_160,4);
        __read(param_1,&local_150,4);
        __read(param_1,&local_158,4);
        __read(param_1,&local_154,4);
        iVar5 = *(int *)(*(int *)((int)this + 8) + 0x28);
        *(undefined4 *)(iVar5 + 0x18 + iVar7) = local_150;
        iVar5 = iVar5 + iVar7;
        *(undefined4 *)(iVar5 + 0x1c) = local_158;
        *(undefined4 *)(iVar5 + 0x14) = local_160;
        *(float *)(iVar5 + 0x20) = local_154;
        *(bool *)(iVar5 + 0x4c) = *(float *)(iVar5 + 0x10) != (float)_DAT_004cf3c8;
        __read(param_1,&local_160,4);
        __read(param_1,&local_150,4);
        __read(param_1,&local_158,4);
        __read(param_1,&local_154,4);
        iVar5 = *(int *)(*(int *)((int)this + 8) + 0x28);
        *(undefined4 *)(iVar5 + 8 + iVar7) = local_150;
        iVar5 = iVar5 + iVar7;
        *(undefined4 *)(iVar5 + 0xc) = local_158;
        *(undefined4 *)(iVar5 + 4) = local_160;
        *(float *)(iVar5 + 0x10) = local_154;
        *(bool *)(iVar5 + 0x4c) = local_154 != (float)_DAT_004cf3c8;
        __read(param_1,&local_130,4);
        __read(param_1,&local_128,4);
        __read(param_1,&local_11c,4);
        __read(param_1,&local_118,4);
        iVar5 = *(int *)(*(int *)((int)this + 8) + 0x28);
        *(undefined4 *)(iVar5 + 0x28 + iVar7) = local_128;
        iVar5 = iVar5 + iVar7;
        *(undefined4 *)(iVar5 + 0x2c) = local_11c;
        *(undefined4 *)(iVar5 + 0x24) = local_130;
        *(undefined4 *)(iVar5 + 0x30) = local_118;
        *(bool *)(iVar5 + 0x4c) = *(float *)(iVar5 + 0x10) != (float)_DAT_004cf3c8;
        __read(param_1,&local_160,4);
        __read(param_1,&local_150,4);
        __read(param_1,&local_158,4);
        __read(param_1,&local_154,4);
        iVar5 = *(int *)(*(int *)((int)this + 8) + 0x28);
        *(undefined4 *)(iVar5 + 0x38 + iVar7) = local_150;
        iVar5 = iVar5 + iVar7;
        *(undefined4 *)(iVar5 + 0x3c) = local_158;
        *(undefined4 *)(iVar5 + 0x34) = local_160;
        *(float *)(iVar5 + 0x40) = local_154;
        *(bool *)(iVar5 + 0x4c) = *(float *)(iVar5 + 0x10) != (float)_DAT_004cf3c8;
        __read(param_1,&local_120,4);
        *(undefined4 *)(*(int *)(*(int *)((int)this + 8) + 0x28) + 0x44 + iVar7) = local_120;
        __read(param_1,&local_114,4);
        *(bool *)(*(int *)(*(int *)((int)this + 8) + 0x28) + 0x4d + iVar7) = local_114 != 0;
        __read(param_1,&local_12c,4);
        if (local_12c == 1) {
          __read(param_1,&local_124,4);
          __read(param_1,local_10c,local_124);
          pvVar2 = Graphics_FindOrCreateTexture(*(void **)((int)this + 4),local_10c,'\x01');
          *(void **)(*(int *)(*(int *)((int)this + 8) + 0x28) + 0x48 + iVar7) = pvVar2;
        }
        __read(param_1,&local_140,4);
        local_134 = 0;
        if (0 < local_140) {
          local_15c = puVar4 + 0x109;
          do {
            local_110 = operator_new(0x14);
            local_4 = 4;
            if (local_110 == (undefined4 *)0x0) {
              iVar7 = 0;
            }
            else {
              iVar7 = Scene_CtorBase(local_110);
            }
            local_4 = 0xffffffff;
            AthenaList_Append(local_15c,iVar7);
            __read(param_1,(int *)(iVar7 + 4),4);
            __read(param_1,(void *)(iVar7 + 0xc),4);
            piVar3 = (int *)(*(int *)(*(int *)((int)this + 0x47c) + 0x480) + 0x43c);
            *piVar3 = *piVar3 + *(int *)(iVar7 + 4);
            local_134 = local_134 + 1;
            iVar7 = local_144;
          } while (local_134 < local_140);
        }
        local_14c = local_14c + 1;
        local_144 = iVar7 + 0x50;
      } while (local_14c < local_148);
    }
  }
  else {
    *(undefined1 *)((int)this + 0x430) = 1;
    do {
      local_15c = operator_new(0x10d0);
      local_4 = 1;
      if (local_15c == (void *)0x0) {
        piVar3 = (int *)0x0;
      }
      else {
        piVar3 = Level_ctor(local_15c,*(undefined4 *)((int)this + 4));
      }
      local_4 = 0xffffffff;
      AthenaList_Append((void *)((int)this + 0x18),(int)piVar3);
      (**(code **)(*piVar3 + 0x34))(param_1,param_2);
      iVar7 = iVar7 + 1;
    } while (iVar7 < local_13c);
  }
  ExceptionList = local_c;
  return;
}

