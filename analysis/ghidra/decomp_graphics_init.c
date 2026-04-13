
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
Graphics_Initialize(void *this,undefined4 param_1,undefined4 param_2,undefined4 param_3,
                   undefined1 param_4)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  undefined4 *puVar7;
  undefined4 *puVar8;
  undefined4 *puVar9;
  undefined1 *puVar10;
  int **ppiVar11;
  undefined4 uStack_17c;
  undefined4 uStack_178;
  int *piStack_174;
  int *piStack_164;
  char *pcStack_160;
  int *piStack_154;
  char *pcStack_150;
  int iStack_14c;
  undefined4 uStack_148;
  int iStack_138;
  int aiStack_134 [2];
  int iStack_12c;
  
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(1)";
  *(undefined4 *)((int)this + 0x6c) = param_1;
  *(undefined4 *)((int)this + 0x70) = param_2;
  *(undefined1 *)((int)this + 0x78) = param_4;
  iVar5 = 0;
  *(undefined4 *)((int)this + 0x7c4) = 0;
  *(undefined4 *)((int)this + 0x74) = param_3;
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(2)";
  uStack_148 = 0x4553d8;
  FUN_00453ed0((int)this);
  uStack_148 = 0xdc;
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(3)";
  iStack_14c = 0x4553ef;
  iVar2 = Direct3DCreate8();
  *(int *)((int)this + 0x7c) = iVar2;
  if (iVar2 == 0) {
    iStack_14c = 0;
    pcStack_150 = "DirectX Error";
    piStack_154 = (int *)0x4d8d78;
    MessageBoxA();
    *(undefined1 *)((int)this + 0x60) = 1;
    return;
  }
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(4)";
  piStack_154 = *(int **)((int)this + 0x7c);
  iStack_14c = (int)this + 0x158;
  pcStack_150 = (char *)0x0;
  iVar2 = (**(code **)(*piStack_154 + 0x20))();
  if (iVar2 < 0) {
    pcStack_160 = "Failed: GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&mDesktopMode)";
    piStack_164 = (int *)0x0;
    MessageBoxA();
    *(undefined1 *)((int)this + 0x60) = 1;
    return;
  }
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(5)";
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(6)";
  piStack_164 = *(int **)((int)this + 0x7c);
  pcStack_160 = (char *)0x0;
  uVar6 = 0;
  uVar4 = 0;
  piStack_154 = (int *)(**(code **)(*piStack_164 + 0x18))();
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(7)";
  *(undefined4 *)((int)this + 0x17c) = 0x14;
  *(undefined4 *)((int)this + 0x178) = 0x17;
  *(undefined1 *)((int)this + 0x7d3) = 0;
  *(undefined1 *)((int)this + 0x7d4) = 0;
  *(undefined1 *)((int)this + 0x7d5) = 0;
  *(undefined1 *)((int)this + 0x7d6) = 0;
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(8)";
  if (0 < (int)piStack_154) {
    do {
      piStack_174 = *(int **)((int)this + 0x7c);
      uStack_178 = 0x4554e7;
      iVar2 = (**(code **)(*piStack_174 + 0x1c))();
      if (-1 < iVar2) {
        if (iStack_138 == 0x280) {
          if (aiStack_134[0] == 0x1e0) {
            *(undefined1 *)((int)this + 0x7d3) = 1;
          }
        }
        else if (iStack_138 == 800) {
          if (aiStack_134[0] == 600) {
            *(undefined1 *)((int)this + 0x7d4) = 1;
          }
        }
        else if (iStack_138 == 0x400) {
          if (aiStack_134[0] == 0x300) {
            *(undefined1 *)((int)this + 0x7d5) = 1;
          }
        }
        else if ((iStack_138 == 0x500) && (aiStack_134[0] == 0x400)) {
          *(undefined1 *)((int)this + 0x7d6) = 1;
        }
        if (iStack_12c == 0x14) {
          if (uVar4 < 1000) {
            *(undefined4 *)((int)this + 0x17c) = 0x14;
            uVar4 = 1000;
          }
        }
        else if (iStack_12c == 0x16) {
          if (uVar4 < 900) {
            *(undefined4 *)((int)this + 0x17c) = 0x16;
            uVar4 = 900;
          }
        }
        else if (iStack_12c == 0x15) {
          if (uVar4 < 800) {
            *(undefined4 *)((int)this + 0x17c) = 0x15;
            uVar4 = 800;
          }
        }
        else if (iStack_12c == 0x17) {
          if (uVar6 < 1000) {
            *(undefined4 *)((int)this + 0x178) = 0x17;
            uVar6 = 1000;
          }
        }
        else if (iStack_12c == 0x18) {
          if (uVar6 < 900) {
            *(undefined4 *)((int)this + 0x178) = 0x18;
            uVar6 = 900;
          }
        }
        else if (iStack_12c == 0x19) {
          if (uVar6 < 800) {
            *(undefined4 *)((int)this + 0x178) = 0x19;
            uVar6 = 800;
          }
        }
        else if ((iStack_12c == 0x1a) && (uVar6 < 700)) {
          *(undefined4 *)((int)this + 0x178) = 0x1a;
          uVar6 = 700;
        }
      }
      iVar5 = iVar5 + 1;
    } while (iVar5 < (int)piStack_154);
  }
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(9)";
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(10)";
  if (*(char *)((int)this + 0x18c) == '\0') {
    uVar3 = *(undefined4 *)((int)this + 0x17c);
  }
  else {
    uVar3 = *(undefined4 *)((int)this + 0x178);
  }
  *(undefined4 *)((int)this + 0x174) = uVar3;
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(11)";
  puVar9 = (undefined4 *)((int)this + 0x194);
  puVar7 = puVar9;
  for (iVar2 = 0xd; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar7 = 0;
    puVar7 = puVar7 + 1;
  }
  *(undefined4 *)((int)this + 0x1ac) = *(undefined4 *)((int)this + 0x6c);
  *(undefined4 *)((int)this + 0x1b0) = 1;
  *puVar9 = *(undefined4 *)((int)this + 0x70);
  *(undefined4 *)((int)this + 0x1a8) = 1;
  *(undefined4 *)((int)this + 0x1b4) = 1;
  *(undefined4 *)((int)this + 0x198) = *(undefined4 *)((int)this + 0x74);
  *(undefined4 *)((int)this + 0x19c) = *(undefined4 *)((int)this + 0x164);
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(12)";
  piStack_174 = (int *)0x4556e4;
  Graphics_CreateDevice
            (this,*(undefined4 **)((int)this + 0x164),&piStack_154,(undefined1 *)((int)this + 0x180)
            );
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(13)";
  *(int **)((int)this + 0x1b8) = piStack_154;
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(14)";
  puVar7 = (undefined4 *)((int)this + 0x1c8);
  puVar8 = puVar7;
  for (iVar2 = 0xd; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar8 = 0;
    puVar8 = puVar8 + 1;
  }
  *(undefined4 *)((int)this + 0x1cc) = *(undefined4 *)((int)this + 0x74);
  *(undefined4 *)((int)this + 0x1d0) = *(undefined4 *)((int)this + 0x174);
  *puVar7 = *(undefined4 *)((int)this + 0x70);
  *(undefined4 *)((int)this + 0x1d4) = 1;
  *(undefined4 *)((int)this + 0x1d8) = 0;
  *(undefined4 *)((int)this + 0x1e4) = 0;
  *(undefined4 *)((int)this + 0x1e8) = 1;
  *(undefined4 *)((int)this + 0x1e0) = *(undefined4 *)((int)this + 0x6c);
  *(undefined4 *)((int)this + 0x1dc) = 2;
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(15)";
  piStack_174 = (int *)0x45578c;
  Graphics_CreateDevice
            (this,*(undefined4 **)((int)this + 0x174),&piStack_154,(undefined1 *)((int)this + 0x181)
            );
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(16)";
  *(int **)((int)this + 0x1ec) = piStack_154;
  *(undefined4 *)((int)this + 500) = 0;
  *(undefined4 *)((int)this + 0x1f8) = 1;
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(17)";
  if (*(char *)((int)this + 0x78) == '\x01') {
    *(undefined4 **)((int)this + 400) = puVar7;
    *(undefined1 *)((int)this + 0x182) = *(undefined1 *)((int)this + 0x181);
  }
  else {
    *(undefined4 **)((int)this + 400) = puVar9;
    *(undefined1 *)((int)this + 0x182) = *(undefined1 *)((int)this + 0x180);
  }
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(18)";
  piVar1 = (int *)((int)this + 0x154);
  piStack_174 = *(int **)((int)this + 0x6c);
  uStack_178 = 1;
  uStack_17c = 0;
  (**(code **)(**(int **)((int)this + 0x7c) + 0x3c))(*(int **)((int)this + 0x7c));
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(19)";
  if (*piVar1 == 0) {
    (**(code **)(**(int **)((int)this + 0x7c) + 0x3c))
              (*(int **)((int)this + 0x7c),0,1,*(undefined4 *)((int)this + 0x6c),0x80,
               *(undefined4 *)((int)this + 400),piVar1);
    if (*piVar1 == 0) {
      iVar2 = (**(code **)(**(int **)((int)this + 0x7c) + 0x3c))
                        (*(int **)((int)this + 0x7c),0,1,*(undefined4 *)((int)this + 0x6c),0x20,
                         *(undefined4 *)((int)this + 400),piVar1);
      if (*piVar1 == 0) {
        FUN_004764f0(iVar2,(char *)aiStack_134,0xff);
        MessageBoxA(0,aiStack_134,"Graphics::Initialize",0);
        *(undefined1 *)((int)this + 0x60) = 1;
        return;
      }
    }
  }
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(20)";
  Graphics_InitRenderStates(this);
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(21)";
  (**(code **)(**(int **)((int)this + 0x7c) + 0x34))
            (*(int **)((int)this + 0x7c),0,1,(int)this + 0x80);
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(22)";
  Graphics_SetViewport(this,0,0);
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(23)";
  Graphics_SetProjection(this,10.0,5000.0);
  piStack_164 = (int *)0x3f800000;
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(24)";
  pcStack_160 = (char *)0x0;
  piStack_154 = (int *)0x0;
  uStack_17c = 0;
  pcStack_150 = (char *)0x0;
  ppiVar11 = &piStack_164;
  iStack_14c = 0;
  uStack_178 = 0;
  puVar10 = &stack0xfffffe90;
  piStack_174 = (int *)0x0;
  puVar9 = &uStack_17c;
  iVar2 = (int)this + 0x1fc;
  thunk_FUN_0045c01a();
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(25)";
  Graphics_SetupLights(this);
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(26)";
  (**(code **)(*(int *)*piVar1 + 0xfc))
            ((int *)*piVar1,0,0x13,*(float *)((int)this + 0x184) * _DAT_004cf41c - _DAT_004cf48c,
             iVar2,puVar9,puVar10,ppiVar11);
  *(int *)((int)this + 0x7c8) = *(int *)((int)this + 0x7c8) + 1;
  *(int *)((int)this + 0x184) = iVar2;
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(27)";
  return;
}

