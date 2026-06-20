
int __cdecl D3D_ConvertFourCC(int param_1)

{
  if (param_1 == 0x36314c20) {
    return 0x32;
  }
  if (param_1 == 0x36314c41) {
    return 0x33;
  }
  if (param_1 == 0x36315220) {
    return 0x14;
  }
  if (param_1 != 0x36315241) {
    return param_1;
  }
  return 0x15;
}

