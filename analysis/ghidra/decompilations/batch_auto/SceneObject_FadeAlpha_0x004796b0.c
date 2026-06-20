
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall SceneObject_FadeAlpha(int *param_1)

{
  float fVar1;
  int iVar2;
  float fVar3;
  
  if (param_1[0x221] == -1) {
    fVar1 = (float)param_1[0x225];
    fVar3 = (float)_DAT_004d5d60;
    param_1[0x225] = (int)(fVar1 * fVar3);
    if (fVar1 * fVar3 < _DAT_004cf310) {
      fVar1 = (float)param_1[0x226] + _DAT_004da724;
      param_1[0x225] = 0x3f800000;
      param_1[0x226] = (int)fVar1;
      if (_DAT_004cf310 < fVar1) {
        param_1[0x226] = 0x3f800000;
        iVar2 = param_1[0x227];
        param_1[0x227] = iVar2 + 1;
        if (100 < iVar2 + 1) {
          (**(code **)(*param_1 + 0x40))();
                    /* WARNING: Could not recover jumptable at 0x004797ff. Too many branches */
                    /* WARNING: Treating indirect jump as call */
          (**(code **)(*(int *)param_1[0x21e] + 0x4c))();
          return;
        }
      }
    }
  }
  else {
    switch(param_1[0x221]) {
    case 0:
      iVar2 = param_1[0x223];
      param_1[0x223] = iVar2 + -1;
      if (iVar2 + -1 < 1) {
        param_1[0x221] = 1;
      }
      break;
    case 1:
      fVar1 = (float)param_1[0x222] + _DAT_004d3a28;
      param_1[0x222] = (int)fVar1;
      if ((float)_DAT_004cf3c8 < fVar1) {
        param_1[0x222] = 0x3f800000;
        param_1[0x221] = 2;
        param_1[0x223] = 0x96;
        return;
      }
      break;
    case 2:
      iVar2 = param_1[0x223];
      param_1[0x223] = iVar2 + -1;
      if (iVar2 + -1 < 1) {
        param_1[0x221] = 3;
        return;
      }
      break;
    case 3:
      fVar1 = (float)param_1[0x222] - _DAT_004d5c54;
      param_1[0x222] = (int)fVar1;
      if (fVar1 < _DAT_004cf368) {
        param_1[0x222] = 0;
        param_1[0x221] = -1;
        return;
      }
    }
  }
  return;
}

