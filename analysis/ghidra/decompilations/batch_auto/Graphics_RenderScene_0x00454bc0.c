
void __fastcall Graphics_RenderScene(void *param_1)

{
  undefined1 uVar1;
  undefined4 uVar2;
  int iVar3;
  float *pfVar4;
  undefined4 *puVar5;
  int *piVar6;
  float *pfVar7;
  undefined4 *puVar8;
  undefined4 uStack_d8;
  undefined4 local_c0 [13];
  undefined4 auStack_8c [19];
  float local_40 [16];
  
  Graphics_SetupLights(param_1);
  puVar5 = (undefined4 *)((int)param_1 + 0x224);
  puVar8 = local_c0;
  for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar8 = *puVar5;
    puVar5 = puVar5 + 1;
    puVar8 = puVar8 + 1;
  }
  pfVar4 = (float *)((int)param_1 + 0x264);
  pfVar7 = local_40;
  for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
    *pfVar7 = *pfVar4;
    pfVar4 = pfVar4 + 1;
    pfVar7 = pfVar7 + 1;
  }
  puVar5 = (undefined4 *)((int)param_1 + 0x2a4);
  puVar8 = auStack_8c + 3;
  for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar8 = *puVar5;
    puVar5 = puVar5 + 1;
    puVar8 = puVar8 + 1;
  }
  uStack_d8 = 0x454c18;
  Gfx_SetViewMatrix(param_1,local_40);
  uStack_d8 = 3;
  (**(code **)(**(int **)((int)param_1 + 0x154) + 0x94))(*(int **)((int)param_1 + 0x154));
  puVar5 = auStack_8c;
  puVar8 = (undefined4 *)((int)param_1 + 0x2a4);
  for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar8 = *puVar5;
    puVar5 = puVar5 + 1;
    puVar8 = puVar8 + 1;
  }
  Matrix_ComputeFrustum(*(int *)((int)param_1 + 0x748));
  (**(code **)(**(int **)((int)param_1 + 0x154) + 0x94))
            (*(int **)((int)param_1 + 0x154),0x100,&stack0xffffff34);
  puVar5 = &uStack_d8;
  puVar8 = (undefined4 *)((int)param_1 + 0x224);
  for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar8 = *puVar5;
    puVar5 = puVar5 + 1;
    puVar8 = puVar8 + 1;
  }
  Matrix_ComputeFrustum(*(int *)((int)param_1 + 0x748));
  piVar6 = (int *)((int)param_1 + 0x710);
  iVar3 = 8;
  do {
    if ((int *)*piVar6 != (int *)0x0) {
      (**(code **)(*(int *)*piVar6 + 0xc))();
    }
    piVar6 = piVar6 + 1;
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  uVar2 = *(undefined4 *)((int)param_1 + 0x730);
  (**(code **)(**(int **)((int)param_1 + 0x154) + 200))(*(int **)((int)param_1 + 0x154),0x8b,uVar2);
  uVar1 = *(undefined1 *)((int)param_1 + 0x734);
  *(undefined4 *)((int)param_1 + 0x730) = uVar2;
  (**(code **)(**(int **)((int)param_1 + 0x154) + 200))(*(int **)((int)param_1 + 0x154),0x1c,uVar1);
  *(undefined1 *)((int)param_1 + 0x734) = uVar1;
  Graphics_SetViewportZ
            (param_1,*(undefined4 *)((int)param_1 + 0x73c),*(undefined4 *)((int)param_1 + 0x740));
  uVar2 = *(undefined4 *)((int)param_1 + 0x738);
  (**(code **)(**(int **)((int)param_1 + 0x154) + 200))(*(int **)((int)param_1 + 0x154),0x22,uVar2);
  *(undefined4 *)((int)param_1 + 0x738) = uVar2;
  (**(code **)(**(int **)((int)param_1 + 0x5c) + 0x88))();
  (**(code **)(**(int **)((int)param_1 + 0x5c) + 0x78))();
  return;
}

