
void __thiscall Menu_AddBCMesh(void *this,undefined4 param_1,undefined4 param_2)

{
  char *pcVar1;
  void *this_00;
  char *pcVar2;
  
  this_00 = operator_new(0x48);
  if (this_00 == (void *)0x0) {
    this_00 = (void *)0x0;
  }
  else {
    *(undefined4 *)((int)this_00 + 0x18) = 0xf;
    *(undefined4 *)((int)this_00 + 0x14) = 0;
    *(undefined1 *)((int)this_00 + 4) = 0;
    *(undefined4 *)((int)this_00 + 0x34) = 0xf;
    *(undefined4 *)((int)this_00 + 0x30) = 0;
    *(undefined1 *)((int)this_00 + 0x20) = 0;
  }
  pcVar1 = "BCMESH";
  do {
    pcVar2 = pcVar1;
    pcVar1 = pcVar2 + 1;
  } while (*pcVar2 != '\0');
  StdString_Assign(this_00,(undefined4 *)&DAT_004da13c,(uint)(pcVar2 + -0x4da13c));
  *(undefined4 *)((int)this_00 + 0x3c) = param_2;
  *(undefined4 *)((int)this_00 + 0x38) = param_1;
  AthenaList_Append((void *)((int)this + 0x1cf4),(int)this_00);
  *(void **)((int)this + 0x2d54) = (void *)((int)this + 0x1cf4);
  return;
}

