
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __cdecl Float_IsBetween(float param_1,float param_2)

{
  float fVar1;
  
  fVar1 = param_1 - param_2;
  if ((_DAT_004d8f68 < fVar1 != (_DAT_004d8f68 == fVar1)) &&
     (fVar1 < _DAT_004d8f64 != (fVar1 == _DAT_004d8f64))) {
    return 1;
  }
  return 0;
}

