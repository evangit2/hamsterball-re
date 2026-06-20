
void __fastcall Menu_AddDelay(int param_1)

{
  char *pcVar1;
  void *this;
  char *pcVar2;
  
  this = operator_new(0x48);
  if (this == (void *)0x0) {
    this = (void *)0x0;
  }
  else {
    *(undefined4 *)((int)this + 0x18) = 0xf;
    *(undefined4 *)((int)this + 0x14) = 0;
    *(undefined1 *)((int)this + 4) = 0;
    *(undefined4 *)((int)this + 0x34) = 0xf;
    *(undefined4 *)((int)this + 0x30) = 0;
    *(undefined1 *)((int)this + 0x20) = 0;
  }
  pcVar1 = "DELAY";
  do {
    pcVar2 = pcVar1;
    pcVar1 = pcVar2 + 1;
  } while (*pcVar2 != '\0');
  StdString_Assign(this,(undefined4 *)"DELAY",(uint)(pcVar2 + -0x4da154));
  AthenaList_Append(*(void **)(param_1 + 0x2d54),(int)this);
  return;
}

