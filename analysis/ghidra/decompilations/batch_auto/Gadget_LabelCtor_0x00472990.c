
void * __thiscall Gadget_LabelCtor(void *this,int param_1,char *param_2,undefined4 param_3)

{
  char cVar1;
  char *pcVar2;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd9c8;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Gadget_ctor(this,param_1);
  *(undefined4 *)((int)this + 4) = 0xbf800000;
  *(undefined4 *)((int)this + 8) = 0xbf800000;
  *(undefined4 *)((int)this + 0xc) = 0x3f800000;
  *(undefined4 *)((int)this + 0x10) = 0x3f800000;
  local_4 = 0;
  *(undefined ***)this = &PTR_ArenaBoard_DeletingDtor_004d9e68;
  pcVar2 = param_2;
  do {
    cVar1 = *pcVar2;
    pcVar2 = pcVar2 + 1;
  } while (cVar1 != '\0');
  pcVar2 = operator_new((uint)(pcVar2 + (1 - (int)(param_2 + 1))));
  *(char **)((int)this + 0x878) = pcVar2;
  do {
    cVar1 = *param_2;
    param_2 = param_2 + 1;
    *pcVar2 = cVar1;
    pcVar2 = pcVar2 + 1;
  } while (cVar1 != '\0');
  *(undefined4 *)((int)this + 0x87c) = param_3;
  ExceptionList = local_c;
  return this;
}

