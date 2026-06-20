/*
 * Function: Ball_ApplyTrajectory
 * Address: 0x00403750
 * Signature: Ball_ApplyTrajectory(...)
 *
 * Patterns: audio, ball. Calls: Ball_ApplyTrajectory, SQRT, Vec3_NormalizeAndScale, Sound_PlayChannel, Ball_CreateTrailParticles. Offsets: 10, Lines: 44
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* Ball_ApplyTrajectory: Launch pad boost. Reads physics_body+0xCA4 trajectory, normalizes+scale,
   damps Y by _DAT_004CF434. Plays boost sound, sets impact_counter=100 (1.67s force block), creates
   trail particles, increments player boost counter. */

void __fastcall Ball_ApplyTrajectory(int param_1)

{
  float *pfVar1;
  float fVar2;
  float fVar3;
  int iVar4;
  float local_c;
  float local_8;
  float local_4;
  
  iVar4 = *(int *)(param_1 + 0x1a4);
  pfVar1 = (float *)(iVar4 + 0xca4);
  *(undefined1 *)(param_1 + 0x14d) = 1;
  if (&local_c != pfVar1) {
    local_c = *pfVar1;
    local_8 = *(float *)(iVar4 + 0xca8);
    local_4 = *(float *)(iVar4 + 0xcac);
  }
  fVar2 = local_8 * local_8 + local_c * local_c + local_4 * local_4;
  fVar3 = _DAT_004cf368;
  if (fVar2 < _DAT_004cf368 == (fVar2 == _DAT_004cf368)) {
    fVar3 = SQRT(fVar2);
  }
  Vec3_NormalizeAndScale(&local_c,fVar3 * _DAT_004cf3f0);
  if (pfVar1 != &local_c) {
    *(float *)(iVar4 + 0xca8) = *(float *)(iVar4 + 0xca8) * _DAT_004cf434;
    *pfVar1 = local_c;
    *(float *)(iVar4 + 0xcac) = local_4;
  }
  Sound_PlayChannel(*(int *)(*(int *)(param_1 + 0x10) + 0x4dc));
  *(undefined4 *)(param_1 + 0x2f0) = 100;
  Ball_CreateTrailParticles(param_1);
  if (*(int *)(param_1 + 0x18) != -1) {
    iVar4 = *(int *)(param_1 + 0x18) * 0xa0;
    *(int *)(iVar4 + 0x5f8 + *(int *)(param_1 + 0x10)) =
         *(int *)(iVar4 + 0x5f8 + *(int *)(param_1 + 0x10)) + 1;
  }
  return;
}
