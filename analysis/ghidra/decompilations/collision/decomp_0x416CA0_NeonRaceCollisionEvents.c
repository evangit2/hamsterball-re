// NeonRaceCollisionEvents @ 0x416CA0
// Verified: vtable[0x1D] handler, created+decompiled June 2026


void __thiscall NeonRaceCollisionEvents(void *param_1,int *param_2,int *param_3)

{
  int iVar1;
  
  iVar1 = __strnicmp(*(char **)(param_3[1] + 0x864),"N:NEONPLATFORM",0xe);
  if (iVar1 == 0) {
    Lifter_PlaySound(*(int *)(*param_3 + 0x47c));
  }
  iVar1 = __strnicmp(*(char **)(param_3[1] + 0x864),"E:ZOOP",6);
  if ((iVar1 == 0) && (param_2[0x1fc] == 0)) {
    Sound_Play3D(*(void **)(*(int *)((int)param_1 + 0x878) + 0x524),(float)param_2[0x59],
                 (float)param_2[0x5a],(float)param_2[0x5b]);
    param_2[0x1fc] = 100;
  }
  iVar1 = __strnicmp(*(char **)(param_3[1] + 0x864),"E:LIGHTSOFF",0xb);
  if ((iVar1 == 0) && (param_2[0x1ed] == 0)) {
    Sound_Play3D(*(void **)(*(int *)((int)param_1 + 0x878) + 0x528),(float)param_2[0x59],
                 (float)param_2[0x5a],(float)param_2[0x5b]);
    (**(code **)(**(int **)((int)param_1 + param_2[6] * 4 + 0x436c) + 0x10))(0);
    Scene_RegisterObject
              (*(void **)(*(int *)((int)param_1 + 0x878) + 0x174),param_2[6],
               *(int **)((int)param_1 + param_2[6] * 4 + 0x436c));
    if (*(int *)((int)param_1 + 0x4390) == 0) {
      AthenaList_Append((void *)((int)param_1 + 0x2578),*(int *)((int)param_1 + 0x438c));
    }
    *(int *)((int)param_1 + 0x4390) = *(int *)((int)param_1 + 0x4390) + 1;
    param_2[0x1ed] = 100;
  }
  iVar1 = __strnicmp(*(char **)(param_3[1] + 0x864),"E:LIGHTSON",10);
  if ((iVar1 == 0) && (param_2[0x1ee] == 0)) {
    Sound_Play3D(*(void **)(*(int *)((int)param_1 + 0x878) + 0x528),(float)param_2[0x59],
                 (float)param_2[0x5a],(float)param_2[0x5b]);
    (**(code **)(**(int **)((int)param_1 + param_2[6] * 4 + 0x436c) + 0x10))(1);
    Scene_RegisterObject
              (*(void **)(*(int *)((int)param_1 + 0x878) + 0x174),param_2[6],
               *(int **)((int)param_1 + param_2[6] * 4 + 0x436c));
    param_2[0x1ee] = 100;
    iVar1 = *(int *)((int)param_1 + 0x4390) + -1;
    *(int *)((int)param_1 + 0x4390) = iVar1;
    if (iVar1 < 1) {
      *(undefined4 *)((int)param_1 + 0x4390) = 0;
      thunk_Gfx_SetRenderState((void *)((int)param_1 + 0x2578),*(int *)((int)param_1 + 0x438c));
      *(undefined4 *)(*(int *)((int)param_1 + 0x438c) + 0x10dc) = 2;
      *(undefined4 *)(*(int *)((int)param_1 + 0x438c) + 0x10e0) = 0;
    }
  }
  DispatchCollisionEvents(param_1,param_2,param_3);
  return;
}

