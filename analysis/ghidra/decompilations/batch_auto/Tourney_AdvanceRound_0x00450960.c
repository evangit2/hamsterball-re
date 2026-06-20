
void __fastcall Tourney_AdvanceRound(int *param_1)

{
  int iVar1;
  void *this;
  int *piVar2;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004cc99b;
  pvStack_c = ExceptionList;
  iVar1 = *(int *)(param_1[0x21e] + 0x220);
  if (*(char *)(iVar1 + 0x95) == '\0') {
    ExceptionList = &pvStack_c;
    *(int *)(iVar1 + 8) = *(int *)(iVar1 + 8) + -1;
  }
  else {
    ExceptionList = &pvStack_c;
    *(undefined1 *)(iVar1 + 0x95) = 0;
  }
  *(undefined1 *)(*(int *)(param_1[0x21e] + 0x220) + 0x94) = 1;
  (**(code **)(*param_1 + 0x40))();
  this = operator_new(0x111c);
  uStack_4 = 0;
  if (this == (void *)0x0) {
    piVar2 = (int *)0x0;
  }
  else {
    piVar2 = TourneyMenu_ctor(this,param_1[0x21e],'\x01');
  }
  uStack_4 = 0xffffffff;
  Scene_AddObject(*(void **)(param_1[0x21e] + 0x184),piVar2);
  ExceptionList = pvStack_c;
  return;
}

