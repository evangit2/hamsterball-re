
void Audio_CodecInit(void)

{
  int iVar1;
  int iVar2;
  int *unaff_ESI;
  int *piVar3;
  
  if ((0xffdc < unaff_ESI[7]) || (0xffdc < unaff_ESI[6])) {
    *(undefined4 *)(*unaff_ESI + 0x14) = 0x28;
    *(undefined4 *)(*unaff_ESI + 0x18) = 0xffdc;
    (**(code **)*unaff_ESI)();
  }
  if (unaff_ESI[0x30] != 8) {
    *(undefined4 *)(*unaff_ESI + 0x14) = 0xd;
    *(int *)(*unaff_ESI + 0x18) = unaff_ESI[0x30];
    (**(code **)*unaff_ESI)();
  }
  if (10 < unaff_ESI[8]) {
    *(undefined4 *)(*unaff_ESI + 0x14) = 0x18;
    *(int *)(*unaff_ESI + 0x18) = unaff_ESI[8];
    *(undefined4 *)(*unaff_ESI + 0x1c) = 10;
    (**(code **)*unaff_ESI)();
  }
  iVar2 = 0;
  unaff_ESI[0x43] = 1;
  unaff_ESI[0x44] = 1;
  if (0 < unaff_ESI[8]) {
    piVar3 = (int *)(unaff_ESI[0x31] + 0xc);
    do {
      if ((((piVar3[-1] < 1) || (4 < piVar3[-1])) || (*piVar3 < 1)) || (4 < *piVar3)) {
        *(undefined4 *)(*unaff_ESI + 0x14) = 0x10;
        (**(code **)*unaff_ESI)();
      }
      iVar1 = unaff_ESI[0x43];
      if (unaff_ESI[0x43] <= piVar3[-1]) {
        iVar1 = piVar3[-1];
      }
      unaff_ESI[0x43] = iVar1;
      iVar1 = unaff_ESI[0x44];
      if (unaff_ESI[0x44] <= *piVar3) {
        iVar1 = *piVar3;
      }
      iVar2 = iVar2 + 1;
      unaff_ESI[0x44] = iVar1;
      piVar3 = piVar3 + 0x15;
    } while (iVar2 < unaff_ESI[8]);
  }
  iVar2 = 0;
  unaff_ESI[0x45] = 8;
  if (0 < unaff_ESI[8]) {
    piVar3 = (int *)(unaff_ESI[0x31] + 0x1c);
    do {
      piVar3[2] = 8;
      iVar1 = DivCeil(piVar3[-5] * unaff_ESI[6],unaff_ESI[0x43] << 3);
      *piVar3 = iVar1;
      iVar1 = DivCeil(piVar3[-4] * unaff_ESI[7],unaff_ESI[0x44] << 3);
      piVar3[1] = iVar1;
      iVar1 = DivCeil(piVar3[-5] * unaff_ESI[6],unaff_ESI[0x43]);
      piVar3[3] = iVar1;
      iVar1 = DivCeil(piVar3[-4] * unaff_ESI[7],unaff_ESI[0x44]);
      piVar3[0xc] = 0;
      piVar3[4] = iVar1;
      *(undefined1 *)(piVar3 + 5) = 1;
      iVar2 = iVar2 + 1;
      piVar3 = piVar3 + 0x15;
    } while (iVar2 < unaff_ESI[8]);
  }
  iVar2 = DivCeil(unaff_ESI[7],unaff_ESI[0x44] << 3);
  unaff_ESI[0x46] = iVar2;
  if ((unaff_ESI[8] <= unaff_ESI[0x48]) && ((char)unaff_ESI[0x32] == '\0')) {
    *(undefined1 *)(unaff_ESI[99] + 0x10) = 0;
    return;
  }
  *(undefined1 *)(unaff_ESI[99] + 0x10) = 1;
  return;
}

