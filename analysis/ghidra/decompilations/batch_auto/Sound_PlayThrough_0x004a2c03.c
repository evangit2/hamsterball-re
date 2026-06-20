
uint __cdecl Sound_PlayThrough(int *param_1)

{
  int *piVar1;
  undefined4 uVar2;
  int iVar3;
  uint uVar4;
  
  if (param_1[4] == 0xca) {
    uVar2 = Audio_CodecCreateContext((int)param_1);
    if ((char)param_1[0x10] != '\0') {
      param_1[4] = 0xcf;
      return CONCAT31((int3)((uint)uVar2 >> 8),1);
    }
    param_1[4] = 0xcb;
  }
  if (param_1[4] == 0xcb) {
    if (*(char *)(param_1[99] + 0x10) != '\0') {
      while( true ) {
        if ((undefined4 *)param_1[2] != (undefined4 *)0x0) {
          (**(code **)param_1[2])(param_1);
        }
        iVar3 = (**(code **)param_1[99])(param_1);
        if (iVar3 == 0) {
          return 0;
        }
        if (iVar3 == 2) break;
        if ((param_1[2] != 0) && ((iVar3 == 3 || (iVar3 == 1)))) {
          piVar1 = (int *)(param_1[2] + 4);
          *piVar1 = *piVar1 + 1;
          iVar3 = param_1[2];
          if (*(int *)(iVar3 + 8) <= *(int *)(iVar3 + 4)) {
            *(int *)(iVar3 + 8) = param_1[0x46] + *(int *)(iVar3 + 8);
          }
        }
      }
    }
    param_1[0x21] = param_1[0x1f];
  }
  else if (param_1[4] != 0xcc) {
    *(undefined4 *)(*param_1 + 0x14) = 0x12;
    *(int *)(*param_1 + 0x18) = param_1[4];
    (**(code **)*param_1)(param_1);
  }
  uVar4 = Sound_RenderLoop();
  return uVar4;
}

