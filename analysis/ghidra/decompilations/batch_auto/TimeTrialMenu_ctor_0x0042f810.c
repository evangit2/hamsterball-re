
void * __thiscall TimeTrialMenu_ctor(void *this,int param_1)

{
  char *pcVar1;
  int iVar2;
  int iVar3;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cb378;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  PracticeMenu_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_OptionsMenu_DeletingDtor4_004d4670;
  *(char **)((int)this + 0x868) = "Time Trial Menu";
  *(char **)((int)this + 0x888) = "CHOOSE A TIME TRIAL RACE!";
  iVar2 = 0;
  do {
    if (*(char *)(*(int *)((int)this + 0x878) + 0x8c0 + iVar2 * 4) != '\0') {
      iVar3 = *(int *)(*(int *)((int)this + 0x878) + 0x380);
      pcVar1 = (char *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
      Scene_AddTextureToList(this,pcVar1,iVar3);
    }
    if (*(char *)(*(int *)((int)this + 0x878) + 0x8c1 + iVar2 * 4) != '\0') {
      iVar3 = *(int *)(*(int *)((int)this + 0x878) + 900);
      pcVar1 = (char *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
      Scene_AddTextureToList(this,pcVar1,iVar3);
    }
    if (*(char *)(*(int *)((int)this + 0x878) + 0x8c2 + iVar2 * 4) != '\0') {
      iVar3 = *(int *)(*(int *)((int)this + 0x878) + 0x388);
      pcVar1 = (char *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
      Scene_AddTextureToList(this,pcVar1,iVar3);
    }
    if (*(char *)(*(int *)((int)this + 0x878) + 0x8c3 + iVar2 * 4) != '\0') {
      iVar3 = *(int *)(*(int *)((int)this + 0x878) + 0x38c);
      pcVar1 = (char *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
      Scene_AddTextureToList(this,pcVar1,iVar3);
    }
    iVar2 = iVar2 + 1;
  } while (iVar2 < 0xf);
  ExceptionList = local_c;
  return this;
}

