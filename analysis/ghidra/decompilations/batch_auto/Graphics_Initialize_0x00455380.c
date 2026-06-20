
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
Graphics_Initialize(void *this,undefined4 param_1,undefined4 param_2,undefined4 param_3,
                   undefined1 param_4)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  undefined4 *puVar8;
  undefined4 *puVar9;
  undefined4 *puVar10;
  undefined1 *puVar11;
  undefined1 *puVar12;
  undefined4 uStack_17c;
  undefined4 uStack_178;
  int *piStack_174;
  int iStack_138;
  int aiStack_134 [2];
  int iStack_12c;
  
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(1)";
  *(undefined4 *)((int)this + 0x6c) = param_1;
  *(undefined4 *)((int)this + 0x70) = param_2;
  *(undefined1 *)((int)this + 0x78) = param_4;
  iVar6 = 0;
  *(undefined4 *)((int)this + 0x7c4) = 0;
  *(undefined4 *)((int)this + 0x74) = param_3;
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(2)";
  Gfx_LoadQualitySettings((int)this);
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(3)";
  iVar2 = Direct3DCreate8();
  *(int *)((int)this + 0x7c) = iVar2;
  if (iVar2 == 0) {
    MessageBoxA((HWND)0x0,"Failed: Direct3DCreate8(D3D_SDK_VERSION)","DirectX Error",0);
    *(undefined1 *)((int)this + 0x60) = 1;
    return;
  }
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(4)";
  iVar2 = (**(code **)(**(int **)((int)this + 0x7c) + 0x20))();
  if (iVar2 < 0) {
    MessageBoxA((HWND)0x0,"Failed: GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&mDesktopMode)",
                "DirectX Error",0);
    *(undefined1 *)((int)this + 0x60) = 1;
    return;
  }
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(5)";
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(6)";
  uVar7 = 0;
  uVar5 = 0;
  iVar2 = (**(code **)(**(int **)((int)this + 0x7c) + 0x18))();
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(7)";
  *(undefined4 *)((int)this + 0x17c) = 0x14;
  *(undefined4 *)((int)this + 0x178) = 0x17;
  *(undefined1 *)((int)this + 0x7d3) = 0;
  *(undefined1 *)((int)this + 0x7d4) = 0;
  *(undefined1 *)((int)this + 0x7d5) = 0;
  *(undefined1 *)((int)this + 0x7d6) = 0;
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(8)";
  if (0 < iVar2) {
    do {
      piStack_174 = *(int **)((int)this + 0x7c);
      uStack_178 = 0x4554e7;
      iVar3 = (**(code **)(*piStack_174 + 0x1c))();
      if (-1 < iVar3) {
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
          if (uVar5 < 1000) {
            *(undefined4 *)((int)this + 0x17c) = 0x14;
            uVar5 = 1000;
          }
        }
        else if (iStack_12c == 0x16) {
          if (uVar5 < 900) {
            *(undefined4 *)((int)this + 0x17c) = 0x16;
            uVar5 = 900;
          }
        }
        else if (iStack_12c == 0x15) {
          if (uVar5 < 800) {
            *(undefined4 *)((int)this + 0x17c) = 0x15;
            uVar5 = 800;
          }
        }
        else if (iStack_12c == 0x17) {
          if (uVar7 < 1000) {
            *(undefined4 *)((int)this + 0x178) = 0x17;
            uVar7 = 1000;
          }
        }
        else if (iStack_12c == 0x18) {
          if (uVar7 < 900) {
            *(undefined4 *)((int)this + 0x178) = 0x18;
            uVar7 = 900;
          }
        }
        else if (iStack_12c == 0x19) {
          if (uVar7 < 800) {
            *(undefined4 *)((int)this + 0x178) = 0x19;
            uVar7 = 800;
          }
        }
        else if ((iStack_12c == 0x1a) && (uVar7 < 700)) {
          *(undefined4 *)((int)this + 0x178) = 0x1a;
          uVar7 = 700;
        }
      }
      iVar6 = iVar6 + 1;
    } while (iVar6 < iVar2);
  }
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(9)";
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(10)";
  if (*(char *)((int)this + 0x18c) == '\0') {
    uVar4 = *(undefined4 *)((int)this + 0x17c);
  }
  else {
    uVar4 = *(undefined4 *)((int)this + 0x178);
  }
  *(undefined4 *)((int)this + 0x174) = uVar4;
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(11)";
  puVar10 = (undefined4 *)((int)this + 0x194);
  puVar8 = puVar10;
  for (iVar6 = 0xd; iVar6 != 0; iVar6 = iVar6 + -1) {
    *puVar8 = 0;
    puVar8 = puVar8 + 1;
  }
  *(undefined4 *)((int)this + 0x1ac) = *(undefined4 *)((int)this + 0x6c);
  *(undefined4 *)((int)this + 0x1b0) = 1;
  *puVar10 = *(undefined4 *)((int)this + 0x70);
  *(undefined4 *)((int)this + 0x1a8) = 1;
  *(undefined4 *)((int)this + 0x1b4) = 1;
  *(undefined4 *)((int)this + 0x198) = *(undefined4 *)((int)this + 0x74);
  *(undefined4 *)((int)this + 0x19c) = *(undefined4 *)((int)this + 0x164);
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(12)";
  piStack_174 = (int *)0x4556e4;
  Graphics_CreateDevice
            (this,*(undefined4 **)((int)this + 0x164),(undefined4 *)&stack0xfffffeac,
             (undefined1 *)((int)this + 0x180));
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(13)";
  *(int *)((int)this + 0x1b8) = iVar2;
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(14)";
  puVar8 = (undefined4 *)((int)this + 0x1c8);
  puVar9 = puVar8;
  for (iVar6 = 0xd; iVar6 != 0; iVar6 = iVar6 + -1) {
    *puVar9 = 0;
    puVar9 = puVar9 + 1;
  }
  *(undefined4 *)((int)this + 0x1cc) = *(undefined4 *)((int)this + 0x74);
  *(undefined4 *)((int)this + 0x1d0) = *(undefined4 *)((int)this + 0x174);
  *puVar8 = *(undefined4 *)((int)this + 0x70);
  *(undefined4 *)((int)this + 0x1d4) = 1;
  *(undefined4 *)((int)this + 0x1d8) = 0;
  *(undefined4 *)((int)this + 0x1e4) = 0;
  *(undefined4 *)((int)this + 0x1e8) = 1;
  *(undefined4 *)((int)this + 0x1e0) = *(undefined4 *)((int)this + 0x6c);
  *(undefined4 *)((int)this + 0x1dc) = 2;
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(15)";
  piStack_174 = (int *)0x45578c;
  Graphics_CreateDevice
            (this,*(undefined4 **)((int)this + 0x174),(undefined4 *)&stack0xfffffeac,
             (undefined1 *)((int)this + 0x181));
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(16)";
  *(int *)((int)this + 0x1ec) = iVar2;
  *(undefined4 *)((int)this + 500) = 0;
  *(undefined4 *)((int)this + 0x1f8) = 1;
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(17)";
  if (*(char *)((int)this + 0x78) == '\x01') {
    *(undefined4 **)((int)this + 400) = puVar8;
    *(undefined1 *)((int)this + 0x182) = *(undefined1 *)((int)this + 0x181);
  }
  else {
    *(undefined4 **)((int)this + 400) = puVar10;
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
        D3D_ErrorToString(iVar2,(char *)aiStack_134,0xff);
        MessageBoxA((HWND)0x0,(LPCSTR)aiStack_134,"Graphics::Initialize",0);
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
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(24)";
  uStack_17c = 0;
  puVar12 = &stack0xfffffe9c;
  uStack_178 = 0;
  puVar11 = &stack0xfffffe90;
  piStack_174 = (int *)0x0;
  puVar10 = &uStack_17c;
  iVar2 = (int)this + 0x1fc;
  D3DX_DispatchThunk_71C4();
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(25)";
  Graphics_SetupLights(this);
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(26)";
  (**(code **)(*(int *)*piVar1 + 0xfc))
            ((int *)*piVar1,0,0x13,*(float *)((int)this + 0x184) * _DAT_004cf41c - _DAT_004cf48c,
             iVar2,puVar10,puVar11,puVar12);
  *(int *)((int)this + 0x7c8) = *(int *)((int)this + 0x7c8) + 1;
  *(int *)((int)this + 0x184) = iVar2;
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(27)";
  return;
}

