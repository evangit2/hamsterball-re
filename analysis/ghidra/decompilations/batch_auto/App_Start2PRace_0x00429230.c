
void __thiscall App_Start2PRace(void *this,undefined4 param_1)

{
  int iVar1;
  void *pvVar2;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cae8b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  App_StartRace((int)this);
  *(undefined1 *)((int)this + 0x237) = 1;
  *(undefined1 *)((int)this + 0x234) = 1;
  *(undefined1 *)((int)this + 0x677) = 1;
  *(undefined1 *)((int)this + 0x717) = 1;
  *(undefined1 *)((int)this + 0x7b7) = 1;
  *(undefined1 *)((int)this + 0x5d7) = 0;
  if (*(int *)((int)this + 0xb2c) != 100) {
    *(undefined1 *)((int)this + 0x677) = 0;
  }
  if (*(int *)((int)this + 0xb30) != 100) {
    *(undefined1 *)((int)this + 0x717) = 0;
  }
  if (*(int *)((int)this + 0xb34) != 100) {
    *(undefined1 *)((int)this + 0x7b7) = 0;
  }
  iVar1 = *(int *)((int)this + 0x174);
  *(undefined1 *)(iVar1 + 0x7d2) = 0;
  Gfx_SetCullMode(iVar1);
  *(undefined4 *)(*(int *)((int)this + 0x27c) + 4) =
       *(undefined4 *)(*(int *)((int)this + 0x280) + 4);
  *(undefined4 *)(*(int *)((int)this + 0x288) + 4) =
       *(undefined4 *)(*(int *)((int)this + 0x28c) + 4);
  pvVar2 = operator_new(0x98);
  local_4 = 0;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = PlayerProfile_ctor(pvVar2,this,*(undefined1 *)((int)this + 0x234));
  }
  *(void **)((int)this + 0x220) = pvVar2;
  *(undefined4 *)((int)pvVar2 + 8) = param_1;
  *(undefined1 *)(*(int *)((int)this + 0x220) + 0x11) = 1;
  local_4 = 0xffffffff;
  Tournament_AdvanceRace(*(void **)((int)this + 0x220),'\0');
  ExceptionList = local_c;
  return;
}

