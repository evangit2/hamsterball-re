
void __thiscall VertexDecl_Init(void *this,uint param_1)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  int *piVar4;
  int iStack_10;
  
  uVar2 = param_1 & 0xe;
  iStack_10 = 0;
  piVar4 = (int *)((int)this + 0x18);
  *(uint *)this = param_1;
  *(undefined4 *)((int)this + 8) = 0;
  *(undefined4 *)((int)this + 0xc) = 0;
  *(undefined4 *)((int)this + 0x10) = 0;
  *(undefined4 *)((int)this + 0x14) = 0;
  *(undefined4 *)((int)this + 0x38) = 0;
  *(undefined4 *)((int)this + 0x3c) = 0;
  *piVar4 = 0;
  if (uVar2 == 2) {
    iStack_10 = 0xc;
  }
  else if (uVar2 == 4) {
    iStack_10 = 0x10;
  }
  else if (5 < uVar2) {
    iVar1 = (uVar2 >> 1) - 2;
    *(int *)((int)this + 0x38) = iVar1;
    iStack_10 = iVar1 * 4 + 0xc;
  }
  if ((param_1 & 0x10) != 0) {
    *(int *)((int)this + 8) = iStack_10;
    iStack_10 = iStack_10 + 0xc;
  }
  if ((param_1 & 0x20) != 0) {
    *(int *)((int)this + 0xc) = iStack_10;
    iStack_10 = iStack_10 + 4;
  }
  if ((param_1 & 0x40) != 0) {
    *(int *)((int)this + 0x10) = iStack_10;
    iStack_10 = iStack_10 + 4;
  }
  if ((char)param_1 < '\0') {
    *(int *)((int)this + 0x14) = iStack_10;
    iStack_10 = iStack_10 + 4;
  }
  if ((param_1 & 0xf00) != 0) {
    uVar2 = param_1 >> 8 & 0xf;
    *piVar4 = iStack_10;
    *(uint *)((int)this + 0x3c) = uVar2;
    if (8 < uVar2) {
      *(undefined4 *)((int)this + 0x3c) = 8;
    }
    uVar3 = param_1 >> 0x10;
    uVar2 = 0;
    if (*(int *)((int)this + 0x3c) != 0) {
      do {
        *piVar4 = iStack_10;
        iStack_10 = iStack_10 + *(int *)(&DAT_004db348 + (uVar3 & 3) * 4);
        uVar3 = uVar3 >> 2;
        uVar2 = uVar2 + 1;
        piVar4 = piVar4 + 1;
      } while (uVar2 < *(uint *)((int)this + 0x3c));
    }
  }
  *(int *)((int)this + 4) = iStack_10;
  return;
}

