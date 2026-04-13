# Hamsterball - Key Function Decompilations

Binary: Hamsterball.exe (MD5: 7d25019366b8d7f55906325bd630d7fe)
3,958 functions found by Ghidra auto-analysis

## WinMain (0x004278E0)

Entry point. Creates the global App singleton, initializes, runs game loop, shuts down.
```c
undefined4 WinMain(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  App_Initialize_Full(&DAT_004fd680,param_1,param_3);
  App_Run((int *)&DAT_004fd680);
  App_Shutdown((int *)&DAT_004fd680);
  return 0;
}
```

## App_Initialize_Full (0x00429530) - The 26-Step Init Sequence

Called by WinMain. Sets up the entire game in 26 labeled steps.
```c
void __thiscall App_Initialize_Full(void *this,undefined4 param_1,undefined4 param_2)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  uint uVar5;
  void *pvVar6;
  void *pvVar7;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004caeec;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *(char **)((int)this + 0x208) = "Initialize(1)";
  App_Initialize(this,param_1,param_2);
  *(undefined1 *)(*(int *)((int)this + 0x174) + 0x7d1) = 1;
  pvVar7 = *(void **)((int)this + 4);
  *(char **)((int)this + 0x208) = "Initialize(3)";
  uVar3 = LoadCursorA(pvVar7,"BLANKCURSOR");
  *(undefined4 *)((int)this + 0x240) = uVar3;
  *(char **)((int)this + 0x208) = "Initialize(4)";
  (**(code **)(*(int *)this + 0x8c))(800,600);
  iVar1 = *(int *)((int)this + 0x174);
  *(char **)((int)this + 0x208) = "Initialize(5)";
  if (iVar1 == 0) {
    *(char **)((int)this + 0x208) = "** No Graphics **";
  }
  if (*(int *)(iVar1 + 0x154) == 0) {
    *(char **)((int)this + 0x208) = "** No Graphics Device **";
  }
  if (iVar1 != 0) {
    piVar2 = *(int **)(iVar1 + 0x154);
    if (*(char *)(iVar1 + 0x7d2) == '\0') {
      (**(code **)(*piVar2 + 200))(piVar2,0x16,3);
    }
    else {
      (**(code **)(*piVar2 + 200))(piVar2,0x16,2);
    }
    *(undefined4 *)(iVar1 + 0x708) = 3;
  }
  *(char **)((int)this + 0x208) = "Initialize(6)";
  puVar4 = FUN_00455c50(*(void **)((int)this + 0x174),"shadow.png",'\x01');
  *(undefined4 **)((int)this + 0x278) = puVar4;
  *(int *)((int)this + 0x1cc) = *(int *)((int)this + 0x1cc) + 1;
  *(char **)((int)this + 0x208) = "Initialize(7)";
  puVar4 = FUN_004743f0(*(void **)((int)this + 0x17c),"music\\music.mo3");
  *(undefined4 **)((int)this + 0x534) = puVar4;
  *(undefined4 *)((int)this + 0x53c) = 0;
  *(undefined4 *)((int)this + 0x538) = 0;
  *(char **)((int)this + 0x208) = "Initialize(8)";
  if (puVar4 != (undefined4 *)0x0) {
    FUN_0046a4d0("jukebox.xml");
    *(char **)((int)this + 0x208) = "Initialize(9)";
    puVar4 = FUN_0046a3c0(*(int *)((int)this + 0x534));
    *(undefined4 **)((int)this + 0x53c) = puVar4;
    *(char **)((int)this + 0x208) = "Initialize(10)";
    puVar4 = FUN_0046a3c0(*(int *)((int)this + 0x534));
    *(undefined4 **)((int)this + 0x538) = puVar4;
    *(char **)((int)this + 0x208) = "Initialize(11)";
  }
  *(char **)((int)this + 0x208) = "Initialize(12)";
  FUN_00472ec0(*(int *)((int)this + 0x54));
  *(char **)((int)this + 0x208) = "Initialize(13)";
  uVar3 = FUN_00473170(*(void **)((int)this + 0x54),"PlayCount");
  if ((char)uVar3 == '\0') {
    *(undefined4 *)((int)this + 0x914) = 0x14;
  }
  else {
    uVar5 = FUN_00473080(*(void **)((int)this + 0x54),"PlayCount");
    *(uint *)((int)this + 0x914) = uVar5;
  }
  *(undefined1 *)((int)this + 0x200) = 1;
  *(char **)((int)this + 0x208) = "Initialize(15)";
  puStack_8 = operator_new(0x14);
  if (puStack_8 == (void *)0x0) {
    pvVar6 = (void *)0x0;
  }
  else {
    pvVar6 = (void *)FUN_0046dfa0(puStack_8,*(undefined4 *)((int)this + 0x180));
  }
  *(void **)((int)this + 0x550) = pvVar6;
  *(char **)((int)this + 0x208) = "Initialize(16)";
  FUN_0046dfc0(pvVar6,1);
  *(char **)((int)this + 0x208) = "Initialize(17)";
  puStack_8 = operator_new(0x14);
  if (puStack_8 == (void *)0x0) {
    pvVar6 = (void *)0x0;
  }
  else {
    pvVar6 = (void *)FUN_0046dfa0(puStack_8,*(undefined4 *)((int)this + 0x180));
  }
  *(void **)((int)this + 0x554) = pvVar6;
  *(char **)((int)this + 0x208) = "Initialize(18)";
  FUN_0046dfc0(pvVar6,2);
  *(char **)((int)this + 0x208) = "Initialize(19)";
  puStack_8 = operator_new(0x14);
  if (puStack_8 == (void *)0x0) {
    pvVar6 = (void *)0x0;
  }
  else {
    pvVar6 = (void *)FUN_0046dfa0(puStack_8,*(undefined4 *)((int)this + 0x180));
  }
  *(void **)((int)this + 0x558) = pvVar6;
  *(char **)((int)this + 0x208) = "Initialize(20)";
  FUN_0046dfc0(pvVar6,4);
  *(char **)((int)this + 0x208) = "Initialize(21)";
  puStack_8 = operator_new(0x14);
  if (puStack_8 == (void *)0x0) {
    pvVar6 = (void *)0x0;
  }
  else {
    pvVar6 = (void *)FUN_0046dfa0(puStack_8,*(undefined4 *)((int)this + 0x180));
  }
  *(void **)((int)this + 0x55c) = pvVar6;
  *(char **)((int)this + 0x208) = "Initialize(22)";
  FUN_0046dfc0(pvVar6,5);
  *(char **)((int)this + 0x208) = "Initialize(23)";
  FUN_00472f30(*(int *)((int)this + 0x54));
  *(char **)((int)this + 0x208) = "Initialize(25)";
  (**(code **)(*(int *)this + 0xa0))();
  *(char **)((int)this + 0x208) = "Initialize(26)";
  ExceptionList = pvVar7;
  return;
}
```

## App_Initialize (0x0046bb40)

```c
void __thiscall App_Initialize(void *this,undefined4 param_1,undefined4 param_2)

{
  uint *puVar1;
  undefined4 unaff_retaddr;
  
  *(char **)((int)this + 0x208) = "App::Initialize(1)";
  (**(code **)(*(int *)this + 0x94))(param_2);
  *(undefined4 *)((int)this + 4) = unaff_retaddr;
  *(char **)((int)this + 0x208) = "App::Initialize(2)";
  FUN_00472f50(*(int *)((int)this + 0x54));
  *(char **)((int)this + 0x208) = "App::Initialize(3)";
  (**(code **)(*(int *)this + 0xc))();
  *(char **)((int)this + 0x208) = "App::Initialize(4)";
  (**(code **)(*(int *)this + 0x1c))();
  *(char **)((int)this + 0x208) = "App::Initialize(5)";
  (**(code **)(*(int *)this + 0x18))();
  *(char **)((int)this + 0x208) = "App::Initialize(6)";
  (**(code **)(*(int *)this + 0x30))();
  *(char **)((int)this + 0x208) = "App::Initialize(7)";
  (**(code **)(*(int *)this + 0x3c))();
  *(char **)((int)this + 0x208) = "App::Initialize(8)";
  (**(code **)(*(int *)this + 0x34))();
  *(char **)((int)this + 0x208) = "App::Initialize(9)";
  (**(code **)(*(int *)this + 0x38))();
  *(char **)((int)this + 0x208) = "App::Initialize(10)";
  (**(code **)(*(int *)this + 0x40))();
  *(char **)((int)this + 0x208) = "App::Initialize(11)";
  Graphics_Initialize(*(void **)((int)this + 0x174),*(undefined4 *)((int)this + 8),
                      *(undefined4 *)((int)this + 0x15c),*(undefined4 *)((int)this + 0x160),
                      *(undefined1 *)((int)this + 0x158));
  if (*(char *)(*(int *)((int)this + 0x174) + 0x60) != '\0') {
    MessageBoxA(0,
                "Hamsterball was not able to initialize DirectX!  Hamsterball requires DirectX8.0 or better to run.  To download the latest DirectX, visit http://www.microsoft.com/directx"
                ,"DirectX Initialization Error",0);
    FUN_004bbaeb(0);
  }
  *(char **)((int)this + 0x208) = "App::Initialize(12)";
  if (*(uint **)((int)this + 0x1b4) != (uint *)0x0) {
    puVar1 = FUN_004bacc0(*(uint **)((int)this + 0x1b4),'-');
    if (puVar1 != (uint *)0x0) {
      FUN_00454000(*(void **)((int)this + 0x174),(char *)((int)puVar1 + 1));
    }
  }
  *(char **)((int)this + 0x208) = "App::Initialize(Ok)";
  return;
}
```

## App_Run (0x0046bd80)

```c
void __fastcall App_Run(int *param_1)

{
  char cVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  int local_78;
  int local_74;
  int local_70;
  undefined1 auStack_6c [28];
  undefined4 local_50 [17];
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd5f8;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  FUN_00457ad0(local_50);
  local_70 = (int)(1000 / (longlong)param_1[0x5c]);
  local_4 = 0;
  local_74 = 0;
  local_78 = 0;
  iVar2 = GetTickCount();
  param_1[0x59] = iVar2;
  cVar1 = *(char *)((int)param_1 + 0x159);
  while (cVar1 == '\0') {
    iVar4 = 0;
    Sleep(0);
    param_1[0x84] = (int)"Background";
    param_1[0x5a] = (int)(1000 / (longlong)param_1[0x5b]);
    iVar2 = GetTickCount();
    if (DAT_005341e4 < iVar2) {
      if ((char)param_1[0x6b] == '\x01') {
        FUN_004bae43((char *)(param_1 + 0x66),&DAT_004d03f8);
      }
      param_1[0x65] = 0;
      iVar2 = GetTickCount();
      DAT_005341e4 = iVar2 + 1000;
    }
    iVar2 = PeekMessageA(&local_70,0,0,0,1);
    while (iVar2 != 0) {
      if (*(char *)((int)param_1 + 0x159) != '\0') goto LAB_0046bfc3;
      TranslateMessage(auStack_6c);
      DispatchMessageA(&local_70);
      iVar2 = PeekMessageA(&local_74,0,0,0,1);
    }
    if (*(char *)((int)param_1 + 0x159) != '\0') break;
    do {
      uVar3 = GetTickCount();
      if (((int)(uVar3 - param_1[0x59]) < param_1[0x5a] + -5) ||
         (local_78 = local_78 + 1, 9 < local_78)) {
        param_1[0x84] = (int)&DAT_004d9584;
        local_78 = 0;
        if ((uint)(local_70 + -5 + local_74) < uVar3) {
          if (((void *)param_1[0x5d] != (void *)0x0) &&
             ((*(char *)((int)param_1 + 0x15a) != '\0' || ((char)param_1[0x56] == '\0')))) {
            param_1[0x65] = param_1[0x65] + 1;
            FUN_00453b50((void *)param_1[0x5d],(int)local_50);
            (**(code **)(*param_1 + 0x24))();
            (**(code **)(*param_1 + 0x28))();
            (**(code **)(*param_1 + 0x2c))();
            FUN_00455a90((void *)param_1[0x5d],'\x01');
          }
          local_74 = GetTickCount();
        }
        break;
      }
      param_1[99] = param_1[99] + 1;
      param_1[0x84] = (int)"Update";
      FUN_00453b50((void *)param_1[0x5d],(int)local_50);
      (**(code **)(*param_1 + 0x20))();
      iVar2 = param_1[0x59];
      param_1[0x59] = iVar2 + param_1[0x5a];
      if (1000 < (int)(uVar3 - (iVar2 + param_1[0x5a]))) {
        param_1[0x59] = uVar3 - 1000;
      }
      iVar4 = iVar4 + 1;
    } while (iVar4 < 1);
    cVar1 = *(char *)((int)param_1 + 0x159);
  }
LAB_0046bfc3:
  local_4 = 0xffffffff;
  FUN_00457a40(local_50);
  ExceptionList = pvStack_c;
  return;
}
```

## Graphics_Initialize (0x00455380)

```c
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
  FUN_00453c90(this,*(undefined4 **)((int)this + 0x164),&piStack_154,
               (undefined1 *)((int)this + 0x180));
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
  FUN_00453c90(this,*(undefined4 **)((int)this + 0x174),&piStack_154,
               (undefined1 *)((int)this + 0x181));
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
  FUN_0042c810(this);
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(21)";
  (**(code **)(**(int **)((int)this + 0x7c) + 0x34))
            (*(int **)((int)this + 0x7c),0,1,(int)this + 0x80);
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(22)";
  FUN_00454f10(this,0,0);
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(23)";
  FUN_00454ab0(this,10.0,5000.0);
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
  FUN_00454630(this);
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(26)";
  (**(code **)(*(int *)*piVar1 + 0xfc))
            ((int *)*piVar1,0,0x13,*(float *)((int)this + 0x184) * _DAT_004cf41c - _DAT_004cf48c,
             iVar2,puVar9,puVar10,ppiVar11);
  *(int *)((int)this + 0x7c8) = *(int *)((int)this + 0x7c8) + 1;
  *(int *)((int)this + 0x184) = iVar2;
  *(char **)(*(int *)((int)this + 0x5c) + 0x208) = "Graphics::Initialize(27)";
  return;
}
```

## LoadMeshWorld (0x0045de30)

```c
undefined4 __thiscall LoadMeshWorld(void *this,char *param_1)

{
  code *pcVar1;
  int iVar2;
  undefined4 uVar3;
  void *this_00;
  undefined4 *this_01;
  char local_10c [248];
  void *pvStack_14;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cceae;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  FUN_004bae43(local_10c,(byte *)"%s.meshworld");
  iVar2 = FUN_004c8ff7((uint)local_10c);
  if (iVar2 == 0) {
    (**(code **)(*(int *)this + 0x38))(local_10c);
    uVar3 = (**(code **)(*(int *)this + 0x3c))(0);
  }
  else {
    this_00 = operator_new(0x488);
    local_4 = 0;
    if (this_00 == (void *)0x0) {
      this_01 = (undefined4 *)0x0;
    }
    else {
      this_01 = FUN_004706e0(this_00,*(undefined4 *)((int)this + 4));
    }
    local_4 = 0xffffffff;
    *(undefined4 **)((int)this + 8) = this_01;
    *(undefined1 *)((int)this + 0xd) = 1;
    uVar3 = FUN_00470930(this_01,param_1,'\0');
    if ((char)uVar3 != '\x01') {
      MessageBoxA(0,param_1,"COULD NOT LOAD",0);
      FUN_004bbaeb(0);
      pcVar1 = (code *)swi(3);
      uVar3 = (*pcVar1)();
      return uVar3;
    }
    *(undefined1 *)((int)this + 0xc) = 1;
    uVar3 = (**(code **)(*(int *)this + 4))();
  }
  ExceptionList = pvStack_14;
  return CONCAT31((int3)((uint)uVar3 >> 8),1);
}
```

## CameraLookAt (0x00413280)

```c
void __fastcall CameraLookAt(int *param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  void *pvVar4;
  undefined4 *puVar5;
  int *piVar6;
  undefined4 auStack_20 [3];
  void *pvStack_14;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c97a6;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  pvVar4 = operator_new(0x10d0);
  local_4 = 0;
  if (pvVar4 == (void *)0x0) {
    puVar5 = (undefined4 *)0x0;
  }
  else {
    puVar5 = FUN_00461510(pvVar4,*(undefined4 *)(param_1[0x21e] + 0x174),
                          "levels\\arena-spawnplatform");
  }
  local_4 = 0xffffffff;
  param_1[0x10e3] = (int)puVar5;
  pvVar4 = operator_new(0x10d0);
  local_4 = 1;
  if (pvVar4 == (void *)0x0) {
    puVar5 = (undefined4 *)0x0;
  }
  else {
    puVar5 = FUN_00461510(pvVar4,*(undefined4 *)(param_1[0x21e] + 0x174),"levels\\arena-stands");
  }
  param_1[0x10e4] = (int)puVar5;
  local_4 = 0xffffffff;
  (**(code **)(*param_1 + 0x90))(param_1[0x10e3]);
  (**(code **)(*param_1 + 0x90))(param_1[0x10e4]);
  FUN_0040b090((int)param_1);
  piVar6 = (int *)FUN_004605e0((void *)param_1[0x22b],auStack_20,"CAMERALOOKAT",(undefined1 *)0x0);
  iVar1 = piVar6[1];
  iVar3 = *piVar6;
  iVar2 = piVar6[2];
  param_1[0x10de] = iVar3;
  param_1[0x10df] = iVar1;
  param_1[0x10e0] = iVar2;
  param_1[0x10db] = iVar3;
  param_1[0x10dc] = iVar1;
  param_1[0x10dd] = iVar2;
  param_1[0xa6f] = 0x42340000;
  param_1[0xa70] = 0x44480000;
  param_1[0x10e1] = 0x44480000;
  param_1[0x10e2] = 1;
  (**(code **)(*param_1 + 0x54))();
  ExceptionList = pvStack_14;
  return;
}
```

## ESellerate_Init (0x00429200)

```c
void __fastcall ESellerate_Init(int param_1)

{
  int iVar1;
  
  if (*(char *)(param_1 + 0x218) == '\0') {
    *(undefined1 *)(param_1 + 0x218) = 1;
    iVar1 = FUN_00473355();
    if (iVar1 == 1) {
      MessageBoxA(0,"Count not install eSellerate Engine!",&DAT_004d2908,0);
    }
  }
  return;
}
```

## App_Shutdown (0x0046ba10)

```c
void __fastcall App_Shutdown(int *param_1)

{
  *(undefined1 *)((int)param_1 + 0x159) = 1;
                    /* WARNING: Could not recover jumptable at 0x0046ba19. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*param_1 + 8))();
  return;
}
```

