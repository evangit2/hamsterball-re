/*
 * Function: CreateLimit
 * Address: 0x00410d00
 * Signature: void __thiscall CreateLimit(void *this,int *param_1,int *param_2)
 *
 * Patterns: audio, rendering. Calls: CreateLimit, __stricmp, Rotator_StartSound, AthenaList_ContainsValue, Pendulum_AddIndex, thunk_Gfx_SetRenderState, DispatchCollisionEvents. Offsets: 7, Lines: 37
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall CreateLimit(void *this,int *param_1,int *param_2)

{
  int iVar1;
  uint uVar2;
  
  iVar1 = __stricmp(*(char **)(param_2[1] + 0x864),"E:PEGS");
  if ((iVar1 == 0) && (param_1[0x1e2] == 0)) {
    *(int *)((int)this + 0x47f4) = *(int *)((int)this + 0x47f4) + 1;
    param_1[0x1e2] = 1;
  }
  iVar1 = __stricmp(*(char **)(param_2[1] + 0x864),"E:TRAPPOP");
  if ((iVar1 == 0) && (*(int *)(*(int *)((int)this + 0x878) + 0x23c) != 0)) {
    Rotator_StartSound(*(int *)((int)this + 0x4390));
  }
  iVar1 = __stricmp(*(char **)(param_2[1] + 0x864),"E:NOPEGS");
  if ((iVar1 == 0) && (param_1[0x1e3] == 0)) {
    *(int *)((int)this + 0x47f4) = *(int *)((int)this + 0x47f4) + -1;
    param_1[0x1e3] = 1;
  }
  iVar1 = __stricmp(*(char **)(param_2[1] + 0x864),"E:HEATON");
  if ((iVar1 == 0) && (*(int *)(*(int *)((int)this + 0x878) + 0x23c) != 0)) {
    uVar2 = AthenaList_ContainsValue((void *)(*(int *)((int)this + 0x47ac) + 0x2c),(int)param_1);
    if ((char)uVar2 == '\0') {
      Pendulum_AddIndex(*(void **)((int)this + 0x47ac),(int)param_1);
    }
  }
  iVar1 = __stricmp(*(char **)(param_2[1] + 0x864),"E:HEATOFF");
  if ((iVar1 == 0) && (*(int *)(*(int *)((int)this + 0x878) + 0x23c) != 0)) {
    thunk_Gfx_SetRenderState((void *)(*(int *)((int)this + 0x47ac) + 0x2c),(int)param_1);
  }
  iVar1 = __stricmp(*(char **)(param_2[1] + 0x864),"E:LIMIT");
  if ((iVar1 == 0) && (*(int *)(*(int *)((int)this + 0x878) + 0x23c) != 0)) {
    thunk_Gfx_SetRenderState((void *)(*(int *)((int)this + 0x47ac) + 0x2c),(int)param_1);
  }
  DispatchCollisionEvents(this,param_1,param_2);
  return;
}
