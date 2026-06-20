
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined1 __fastcall SceneObject_ComputeCollisionSphere(int param_1)

{
  int iVar1;
  float fVar2;
  float fVar3;
  short sVar4;
  float local_18;
  float local_14;
  float local_10;
  float local_4;
  
  if (*(char *)(*(int *)(param_1 + 8) + 0x484) == '\0') {
    *(undefined1 *)(*(int *)(param_1 + 8) + 0x484) = 1;
    iVar1 = *(int *)(param_1 + 8);
    local_14 = (*(float *)(iVar1 + 0x46c) + *(float *)(iVar1 + 0x460)) * _DAT_004cf3f0;
    local_10 = (*(float *)(iVar1 + 0x470) + *(float *)(iVar1 + 0x464)) * _DAT_004cf3f0;
    if ((float *)(iVar1 + 0x474) != &local_18) {
      *(float *)(iVar1 + 0x474) =
           (*(float *)(iVar1 + 0x468) + *(float *)(iVar1 + 0x45c)) * _DAT_004cf3f0;
      *(float *)(iVar1 + 0x478) = local_14;
      *(float *)(iVar1 + 0x47c) = local_10;
    }
    iVar1 = *(int *)(param_1 + 8);
    local_4 = *(float *)(iVar1 + 0x464);
    local_18 = *(float *)(iVar1 + 0x45c) - *(float *)(iVar1 + 0x474);
    fVar2 = *(float *)(iVar1 + 0x460) - *(float *)(iVar1 + 0x478);
    fVar3 = local_4 - *(float *)(iVar1 + 0x47c);
    fVar2 = local_18 * local_18 + fVar2 * fVar2 + fVar3 * fVar3;
    fVar3 = _DAT_004cf368;
    if (fVar2 < _DAT_004cf368 == (fVar2 == _DAT_004cf368)) {
      fVar3 = SQRT(fVar2);
    }
    *(float *)(iVar1 + 0x480) = fVar3;
  }
  iVar1 = *(int *)(param_1 + 8);
  sVar4 = Ball_TestPlaneIntersection
                    (*(void **)(*(int *)(param_1 + 4) + 0x748),*(float *)(iVar1 + 0x474),
                     *(float *)(iVar1 + 0x478),*(float *)(iVar1 + 0x47c),*(float *)(iVar1 + 0x480));
  return (char)sVar4;
}

