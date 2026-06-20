
void __cdecl PNG_ProcessRowTransforms(int *param_1)

{
  int *piVar1;
  
  if (param_1[0x37] == 0) {
    longjmp_with_cleanup(param_1,"NULL row buffer");
  }
  if ((*(byte *)((int)param_1 + 0x61) & 0x10) != 0) {
    if ((char)param_1[0x3e] == '\x03') {
      PNG_ExpandPalette((uint *)(param_1 + 0x3c),param_1[0x37] + 1,param_1[0x41],param_1[0x57],
                        (uint)*(ushort *)((int)param_1 + 0x10a));
    }
    else {
      if (*(short *)((int)param_1 + 0x10a) == 0) {
        piVar1 = (int *)0x0;
      }
      else {
        piVar1 = param_1 + 0x58;
      }
      PNG_ExpandGrayAlpha(param_1 + 0x3c,param_1[0x37] + 1,(int)piVar1);
    }
  }
  if (((*(byte *)((int)param_1 + 0x61) & 0x20) != 0) && (*(char *)((int)param_1 + 0x116) != '\x03'))
  {
    PNG_ApplyGammaLUT(param_1 + 0x3c,(byte *)(param_1[0x37] + 1),param_1[0x4e],param_1[0x51],
                      (byte)param_1[0x4b]);
  }
  if ((*(byte *)((int)param_1 + 0x61) & 4) != 0) {
    PNG_StripHighByte16bit(param_1 + 0x3c,(undefined1 *)(param_1[0x37] + 1));
  }
  if ((*(byte *)(param_1 + 0x18) & 0x40) != 0) {
    PNG_QuantizeRGB(param_1 + 0x3c,(byte *)(param_1[0x37] + 1),param_1[0x5d],param_1[0x5e]);
    if (param_1[0x3d] == 0) {
      longjmp_with_cleanup(param_1,"png_do_dither returned rowbytes=0");
    }
  }
  if ((*(byte *)(param_1 + 0x18) & 8) != 0) {
    PNG_ShiftSignificantBits
              ((byte *)(param_1 + 0x3c),(byte *)(param_1[0x37] + 1),(byte *)((int)param_1 + 0x155));
  }
  if ((*(byte *)(param_1 + 0x18) & 4) != 0) {
    PNG_UnpackLowBits(param_1 + 0x3c,param_1[0x37] + 1);
  }
  if ((*(byte *)(param_1 + 0x18) & 1) != 0) {
    PNG_SwapBGRChannels(param_1 + 0x3c,(undefined1 *)(param_1[0x37] + 1));
  }
  if ((*(byte *)((int)param_1 + 0x61) & 0x80) != 0) {
    PNG_InsertFillerByte
              ((uint *)(param_1 + 0x3c),param_1[0x37] + 1,(uint)*(ushort *)((int)param_1 + 0x11e),
               (byte)param_1[0x17]);
  }
  if ((*(byte *)(param_1 + 0x18) & 0x10) != 0) {
    PNG_SwapBytes16bit(param_1 + 0x3c,(undefined1 *)(param_1[0x37] + 1));
  }
  return;
}

