
void __thiscall
Graphics_Reset(void *this,undefined4 param_1,undefined4 param_2,undefined4 param_3,char param_4)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  undefined4 local_104 [65];
  
  *(undefined4 *)((int)this + 0x6c) = param_1;
  *(undefined4 *)((int)this + 0x1e0) = param_1;
  *(undefined4 *)((int)this + 0x1ac) = param_1;
  *(undefined4 *)((int)this + 0x70) = param_2;
  *(undefined4 *)((int)this + 0x74) = param_3;
  *(char *)((int)this + 0x78) = param_4;
  if (*(char *)((int)this + 0x18c) == '\0') {
    uVar2 = *(undefined4 *)((int)this + 0x17c);
  }
  else {
    uVar2 = *(undefined4 *)((int)this + 0x178);
  }
  *(undefined4 *)((int)this + 0x174) = uVar2;
  puVar1 = (undefined4 *)((int)this + 0x194);
  puVar4 = puVar1;
  for (iVar3 = 0xd; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar4 = 0;
    puVar4 = puVar4 + 1;
  }
  *(undefined4 *)((int)this + 0x1b0) = 1;
  *(undefined4 *)((int)this + 0x1a8) = 1;
  *(undefined4 *)((int)this + 0x1b4) = 1;
  *puVar1 = *(undefined4 *)((int)this + 0x70);
  *(undefined4 *)((int)this + 0x1ac) = *(undefined4 *)((int)this + 0x6c);
  *(undefined4 *)((int)this + 0x198) = *(undefined4 *)((int)this + 0x74);
  *(undefined4 **)((int)this + 0x19c) = *(undefined4 **)((int)this + 0x164);
  Graphics_CreateDevice
            (this,*(undefined4 **)((int)this + 0x164),local_104,(undefined1 *)((int)this + 0x180));
  puVar4 = (undefined4 *)((int)this + 0x1c8);
  *(undefined4 *)((int)this + 0x1b8) = local_104[0];
  puVar5 = puVar4;
  for (iVar3 = 0xd; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar5 = 0;
    puVar5 = puVar5 + 1;
  }
  *(undefined4 *)((int)this + 0x1e0) = *(undefined4 *)((int)this + 0x6c);
  *(undefined4 *)((int)this + 0x1cc) = *(undefined4 *)((int)this + 0x74);
  *(undefined4 *)((int)this + 0x1d8) = 0;
  *(undefined4 *)((int)this + 0x1e4) = 0;
  *puVar4 = *(undefined4 *)((int)this + 0x70);
  *(undefined4 *)((int)this + 0x1d4) = 1;
  *(undefined4 *)((int)this + 0x1e8) = 1;
  *(undefined4 **)((int)this + 0x1d0) = *(undefined4 **)((int)this + 0x174);
  *(undefined4 *)((int)this + 0x1dc) = 2;
  Graphics_CreateDevice
            (this,*(undefined4 **)((int)this + 0x174),local_104,(undefined1 *)((int)this + 0x181));
  *(undefined4 *)((int)this + 0x1ec) = local_104[0];
  *(undefined4 *)((int)this + 500) = 0;
  *(undefined4 *)((int)this + 0x1f8) = 1;
  if (param_4 == '\x01') {
    *(undefined4 **)((int)this + 400) = puVar4;
    *(undefined1 *)((int)this + 0x182) = *(undefined1 *)((int)this + 0x181);
  }
  else {
    *(undefined4 **)((int)this + 400) = puVar1;
    *(undefined1 *)((int)this + 0x182) = *(undefined1 *)((int)this + 0x180);
  }
  iVar3 = (**(code **)(**(int **)((int)this + 0x154) + 0x38))
                    (*(int **)((int)this + 0x154),*(undefined4 *)((int)this + 400));
  if (iVar3 < 0) {
    D3D_ErrorToString(iVar3,&stack0xfffffef8,0xff);
    MessageBoxA((HWND)0x0,&stack0xfffffef8,"Graphics::Reset",0);
    CRT_FlsAlloc(0);
  }
  Graphics_RenderScene(this);
  return;
}

