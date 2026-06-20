
void __thiscall MeshWorld_AddTexture(void *this,undefined4 param_1,char *param_2,char param_3)

{
  char cVar1;
  void *this_00;
  char *pcVar2;
  char *pcVar3;
  
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
  pcVar3 = "TEX";
  do {
    pcVar2 = pcVar3;
    pcVar3 = pcVar2 + 1;
  } while (*pcVar2 != '\0');
  StdString_Assign(this_00,(undefined4 *)&DAT_004da15c,(uint)(pcVar2 + -0x4da15c));
  *(undefined4 *)((int)this_00 + 0x38) = param_1;
  pcVar3 = param_2;
  do {
    cVar1 = *pcVar3;
    pcVar3 = pcVar3 + 1;
  } while (cVar1 != '\0');
  StdString_Assign((void *)((int)this_00 + 0x1c),(undefined4 *)param_2,
                   (int)pcVar3 - (int)(param_2 + 1));
  *(bool *)((int)this_00 + 0x40) = param_3 == '\0';
  AthenaList_Append((void *)((int)this + 0x293c),(int)this_00);
  *(void **)((int)this + 0x2d54) = (void *)((int)this + 0x293c);
  return;
}

