
void __cdecl Font_ComputeGlyphCoverage(int param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int *unaff_EDI;
  
  puVar1 = (undefined4 *)(**(code **)unaff_EDI[1])();
  puVar1[1] = 0x1f;
  puVar1[5] = 0x1f;
  *puVar1 = 0;
  puVar1[2] = 0;
  puVar1[3] = 0x3f;
  puVar1[4] = 0;
  Font_ComputeGlyphBounds();
  iVar2 = Font_SplitGlyphQuadrants(unaff_EDI,(int)puVar1,1,param_1);
  param_1 = 0;
  if (0 < iVar2) {
    do {
      Font_ComputeGlyphCentroid(param_1);
      param_1 = param_1 + 1;
    } while (param_1 < iVar2);
  }
  unaff_EDI[0x1c] = iVar2;
  *(undefined4 *)(*unaff_EDI + 0x14) = 0x5f;
  *(int *)(*unaff_EDI + 0x18) = iVar2;
  (**(code **)(*unaff_EDI + 4))();
  return;
}

