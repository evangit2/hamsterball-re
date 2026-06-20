
void __thiscall StdString_AssignN(void *this,char *param_1,size_t param_2)

{
  uint uVar1;
  char *_Dest;
  
  if (param_1 == (char *)0x0) {
    param_1 = "";
  }
  _free(*(void **)((int)this + 4));
  *(undefined4 *)((int)this + 4) = 0;
  *(undefined4 *)((int)this + 0x14) = 0;
  *(undefined4 *)((int)this + 8) = 0;
  *(undefined1 *)((int)this + 0x18) = 0;
  if (param_1 == (char *)0x0) {
    param_1 = "";
    *(undefined1 *)((int)this + 0x18) = 1;
  }
  uVar1 = param_2 + 1 & ((int)(param_2 + 1) < 1) - 1;
  *(uint *)((int)this + 8) = uVar1;
  _Dest = operator_new(uVar1);
  *(char **)((int)this + 4) = _Dest;
  _strncpy(_Dest,param_1,param_2);
  *(undefined1 *)(param_2 + *(int *)((int)this + 4)) = 0;
  *(size_t *)((int)this + 0x14) = param_2;
  return;
}

