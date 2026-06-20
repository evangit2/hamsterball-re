
void __thiscall
Gadget_AddSpriteSlot
          (void *this,byte param_1,int param_2,int param_3,int param_4,int param_5,int param_6,
          undefined4 param_7,undefined4 param_8,undefined4 param_9)

{
  void *pvVar1;
  int iVar2;
  uint uVar3;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ccceb;
  local_c = ExceptionList;
  if (param_1 == 0x20) {
    *(undefined4 *)((int)this + 0x420) = param_7;
    *(undefined1 *)((int)this + 0x6ac) = 1;
  }
  else {
    if (param_2 < 0) {
      iVar2 = 0;
    }
    else if (param_2 < *(int *)((int)this + 0xc)) {
      iVar2 = *(int *)(*(int *)((int)this + 0x414) + param_2 * 4);
    }
    else {
      iVar2 = 0;
    }
    uVar3 = (uint)param_1;
    ExceptionList = &local_c;
    *(undefined4 *)((int)this + uVar3 * 0x14 + 0x430) = param_7;
    *(undefined1 *)((int)this + uVar3 * 0x14 + 0x42c) = 1;
    pvVar1 = operator_new(0xd4);
    local_4 = 0;
    if (pvVar1 == (void *)0x0) {
      pvVar1 = (void *)0x0;
    }
    else {
      pvVar1 = Sprite_Ctor(pvVar1,*(undefined4 *)((int)this + 4),iVar2,param_3,param_4,param_5 + 1,
                           param_6 + 1);
    }
    *(void **)((int)this + uVar3 * 0x14 + 0x43c) = pvVar1;
    *(undefined4 *)((int)this + uVar3 * 0x14 + 0x434) = param_8;
    *(undefined4 *)((int)this + (uVar3 * 5 + 0x10e) * 4) = param_9;
  }
  ExceptionList = local_c;
  return;
}

