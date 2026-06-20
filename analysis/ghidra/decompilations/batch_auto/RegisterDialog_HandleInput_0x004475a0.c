
void __thiscall RegisterDialog_HandleInput(void *this,void *param_1)

{
  void *pvVar1;
  char *pcVar2;
  void *this_00;
  ulonglong uVar3;
  ulonglong uVar4;
  
  pvVar1 = param_1;
  if (*(int *)((int)this + 0x1108) != 0) {
    AthenaString_SprintfToBuffer((char *)&param_1,&DAT_004d3b08);
    this_00 = (void *)((int)this + 0x1120);
    if (*(int *)((int)this + 0x1108) != (int)this + 0x1098) {
      this_00 = param_1;
    }
    if (*(int *)((int)this + 0x1108) == (int)this + 0x10b4) {
      this_00 = (void *)((int)this + 0x113c);
    }
    if (*(uint *)((int)this_00 + 0x18) < 0x10) {
      pcVar2 = (char *)((int)this_00 + 4);
    }
    else {
      pcVar2 = *(char **)((int)this_00 + 4);
    }
    uVar3 = Font_MeasureText(pcVar2);
    uVar4 = Font_MeasureText((char *)&param_1);
    if (0x13f < (int)uVar4 + (int)uVar3) {
      MessageBeep(0);
      return;
    }
    if ((0x1f < (int)pvVar1) && ((int)pvVar1 < 0x80)) {
      StdString_InsertCStr(this_00,*(uint *)((int)this + 0x1158),(char *)&param_1);
      *(int *)((int)this + 0x1158) = *(int *)((int)this + 0x1158) + 1;
    }
  }
  return;
}

