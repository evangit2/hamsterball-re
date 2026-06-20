
undefined4 __thiscall thunk_Gfx_SetRenderState(void *this,int param_1)

{
  int *_Memory;
  int *piVar1;
  void *_Memory_00;
  int *piVar2;
  int *piVar3;
  int iVar4;
  char cVar5;
  int iVar6;
  
  _Memory = _malloc(*(int *)((int)this + 4) << 2);
  piVar2 = *(int **)((int)this + 0x40c);
  cVar5 = '\0';
  iVar4 = 0;
  piVar1 = _Memory;
  if (0 < *(int *)((int)this + 4)) {
    do {
      if (*piVar2 == param_1) {
        piVar3 = (int *)((int)this + 0xc);
        iVar6 = 0x20;
        do {
          if (iVar4 <= piVar3[-1]) {
            piVar3[-1] = piVar3[-1] + -1;
          }
          if (iVar4 <= *piVar3) {
            *piVar3 = *piVar3 + -1;
          }
          if (iVar4 <= piVar3[1]) {
            piVar3[1] = piVar3[1] + -1;
          }
          if (iVar4 <= piVar3[2]) {
            piVar3[2] = piVar3[2] + -1;
          }
          if (iVar4 <= piVar3[3]) {
            piVar3[3] = piVar3[3] + -1;
          }
          if (iVar4 <= piVar3[4]) {
            piVar3[4] = piVar3[4] + -1;
          }
          if (iVar4 <= piVar3[5]) {
            piVar3[5] = piVar3[5] + -1;
          }
          if (iVar4 <= piVar3[6]) {
            piVar3[6] = piVar3[6] + -1;
          }
          piVar3 = piVar3 + 8;
          iVar6 = iVar6 + -1;
        } while (iVar6 != 0);
        cVar5 = '\x01';
      }
      else {
        *piVar1 = *piVar2;
        piVar1 = piVar1 + 1;
      }
      piVar2 = piVar2 + 1;
      iVar4 = iVar4 + 1;
    } while (iVar4 < *(int *)((int)this + 4));
    if (cVar5 == '\x01') {
      _free(*(void **)((int)this + 0x40c));
      _Memory_00 = _realloc(_Memory,*(int *)((int)this + 4) * 4 - 4);
      iVar4 = *(int *)((int)this + 4) + -1;
      *(void **)((int)this + 0x40c) = _Memory_00;
      *(int *)((int)this + 4) = iVar4;
      if (iVar4 < 1) {
        _free(_Memory_00);
        *(undefined4 *)((int)this + 0x40c) = 0;
      }
      goto LAB_004535e1;
    }
  }
  _free(_Memory);
LAB_004535e1:
  piVar2 = (int *)((int)this + 8);
  iVar4 = 0x100;
  do {
    if (*piVar2 < 0) {
      *piVar2 = 0;
    }
    piVar2 = piVar2 + 1;
    iVar4 = iVar4 + -1;
  } while (iVar4 != 0);
  return CONCAT31((int3)((uint)piVar2 >> 8),cVar5);
}

