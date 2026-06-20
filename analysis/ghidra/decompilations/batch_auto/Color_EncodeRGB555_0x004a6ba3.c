
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint __fastcall Color_EncodeRGB555(float *param_1)

{
  float local_1c;
  float local_18;
  float local_14;
  
  if (_DAT_004cf368 <= *param_1) {
    if (_DAT_004cf310 < *param_1) {
      local_1c = 1.0;
    }
    else {
      local_1c = *param_1;
    }
  }
  else {
    local_1c = 0.0;
  }
  if (_DAT_004cf368 <= param_1[1]) {
    if (_DAT_004cf310 < param_1[1]) {
      local_18 = 1.0;
    }
    else {
      local_18 = param_1[1];
    }
  }
  else {
    local_18 = 0.0;
  }
  if (_DAT_004cf368 <= param_1[2]) {
    if (param_1[2] <= _DAT_004cf310) {
      local_14 = param_1[2];
    }
    else {
      local_14 = 1.0;
    }
  }
  else {
    local_14 = 0.0;
  }
  SSE2_SetFPControlWord(param_1);
  return ((int)ROUND(local_1c * _DAT_004d5c24 + _DAT_004cf3f0) << 6 |
         (int)ROUND(local_18 * _DAT_004dc064 + _DAT_004cf3f0)) << 5 |
         (int)ROUND(local_14 * _DAT_004d5c24 + _DAT_004cf3f0);
}

