
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall UIList_Render(void *this,void *param_1)

{
  void *this_00;
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  void *pvVar5;
  int *piVar6;
  int iVar7;
  ulonglong uVar8;
  undefined4 in_stack_ffffff7c;
  undefined4 in_stack_ffffff80;
  undefined4 in_stack_ffffff84;
  undefined4 in_stack_ffffff88;
  undefined4 in_stack_ffffff8c;
  undefined4 uVar9;
  int local_5c;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cc4d0;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Matrix_Scale4x4(&stack0xffffff7c,0,0,0x3f800000,0x3f400000);
  iVar2 = *(int *)((int)this + 0xcb4) + 0x14;
  iVar4 = *(int *)((int)this + 0xcb0) + 0x14;
  uVar8 = __ftol2();
  iVar3 = (int)uVar8;
  uVar8 = __ftol2();
  Graphics_DrawScreenRect(param_1,(int)uVar8,iVar3,iVar4,iVar2);
  if (*(int *)((int)this + 0x888) != 0) {
    Matrix_Scale4x4(&stack0xffffff7c,0x3f000000,0x3f000000,0x3f800000,0x3f400000);
    iVar2 = *(int *)((int)this + 0xcb0) + 0x14;
    iVar4 = 0x14;
    uVar8 = __ftol2();
    iVar3 = (int)uVar8;
    uVar8 = __ftol2();
    Graphics_DrawScreenRect(param_1,(int)uVar8,iVar3,iVar2,iVar4);
    Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f266666);
    uVar8 = __ftol2();
    iVar2 = (int)uVar8;
    uVar8 = __ftol2();
    Font_DrawGlyph(*(void **)(*(int *)((int)this + 0x878) + 0x31c),*(byte **)((int)this + 0x888),
                   (int)uVar8,iVar2,in_stack_ffffff7c,in_stack_ffffff80,in_stack_ffffff84,
                   in_stack_ffffff88,in_stack_ffffff8c);
  }
  uVar8 = __ftol2();
  iVar3 = (int)uVar8;
  uVar8 = __ftol2();
  iVar2 = (int)uVar8;
  uVar9 = 0x449f22;
  iVar4 = AthenaList_NextIndex((int)this + 0x88c);
  piVar6 = (int *)0x0;
  *(undefined4 *)((int)this + iVar4 * 4 + 0x894) = 0;
  if (0 < *(int *)((int)this + 0x890)) {
    piVar6 = (int *)**(undefined4 **)((int)this + 0xc98);
    *(undefined4 *)((int)this + iVar4 * 4 + 0x894) = 1;
  }
  while (piVar6 != (int *)0x0) {
    if ((char)piVar6[0x110] == '\0') {
      if (*piVar6 != 0) {
        pvVar5 = *(void **)((int)this + 0x87c);
        if (*(char *)((int)piVar6 + 0x441) != '\0') {
          pvVar5 = *(void **)((int)this + 0x880);
        }
        if (*(int **)((int)this + 0xcc0) == piVar6) {
          local_4 = 1;
          Matrix_Scale4x4(&stack0xffffff7c,0x3f000000,0x3f000000,0x3f800000,0x3f733333);
          UI_DrawRectAndReset(param_1);
          local_4 = 0xffffffff;
        }
        this_00 = (void *)piVar6[8];
        iVar7 = 0;
        if (this_00 != (void *)0x0) {
          iVar7 = *(int *)((int)this_00 + 200) + 5;
          Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
          Sprite_DrawRect(this_00,(float)iVar3,(float)(iVar2 + 10));
        }
        Matrix_Scale4x4(&stack0xffffff7c,0,0,0,0x3f800000);
        UI_DrawTextShadow(pvVar5,(byte *)*piVar6,iVar7 + iVar3,iVar2,2,1,&PTR_Vec3_dtor_004cf300,
                          piVar6[3],piVar6[4],piVar6[5],piVar6[6],in_stack_ffffff7c,
                          in_stack_ffffff80,in_stack_ffffff84,in_stack_ffffff88,uVar9);
        in_stack_ffffff88 = 0x44a23c;
        uVar8 = Font_MeasureText((char *)*piVar6);
        local_5c = (int)uVar8 + iVar7 + 5 + iVar3;
        uVar9 = 0x44a250;
        iVar7 = AthenaList_NextIndex((int)(piVar6 + 10));
        pvVar5 = (void *)0x0;
        piVar6[iVar7 + 0xc] = 0;
        if (0 < piVar6[0xb]) {
          pvVar5 = *(void **)piVar6[0x10d];
          piVar6[iVar7 + 0xc] = 1;
        }
        if (pvVar5 != (void *)0x0) {
          do {
            Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
            Sprite_DrawRect(pvVar5,(float)local_5c,(float)(iVar2 + 10));
            iVar1 = piVar6[iVar7 + 0xc];
            local_5c = local_5c + *(int *)((int)pvVar5 + 200);
            if (piVar6[0xb] <= iVar1) break;
            pvVar5 = *(void **)(piVar6[0x10d] + iVar1 * 4);
            piVar6[iVar7 + 0xc] = iVar1 + 1;
          } while (pvVar5 != (void *)0x0);
        }
      }
    }
    else {
      if (*(int **)((int)this + 0xcc0) == piVar6) {
        local_4 = 0;
        Matrix_Scale4x4(&stack0xffffff7c,0x3f000000,0x3f000000,0x3f800000,0x3f733333);
        UI_DrawRectAndReset(param_1);
        local_4 = 0xffffffff;
      }
      Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      iVar7 = piVar6[7];
      Gfx_DrawQuadRandomColor
                ((void *)piVar6[8],&PTR_LAB_004cf584,*(float *)(iVar7 + 4),
                 *(float *)(iVar7 + 8) - _DAT_004cf310,*(float *)(iVar7 + 0xc),256.0,
                 &PTR_LAB_004cf584,0,0,0x3f800000,0x3f800000);
    }
    iVar7 = *(int *)((int)this + iVar4 * 4 + 0x894);
    iVar2 = iVar2 + piVar6[9];
    if (*(int *)((int)this + 0x890) <= iVar7) break;
    piVar6 = *(int **)(*(int *)((int)this + 0xc98) + iVar7 * 4);
    *(int *)((int)this + iVar4 * 4 + 0x894) = iVar7 + 1;
  }
  if (*(char *)((int)this + 0xcd0) != '\0') {
    if ((*(int *)((int)this + 0xccc) != *(int *)((int)this + 0xcc4)) &&
       (*(int *)((int)this + 0xcd8) == 0)) {
      Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      Sprite_DrawCentered(*(void **)(*(int *)((int)this + 0x878) + 0x530),
                          (_DAT_004d6ad4 - (float)(*(int *)((int)this + 0xcb0) + 0x14)) -
                          _DAT_004cf370,80.0,in_stack_ffffff7c,in_stack_ffffff80,in_stack_ffffff84,
                          in_stack_ffffff88,uVar9);
      Vec3_Init(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000);
      Graphics_DrawPoint(param_1);
    }
    if ((*(int *)((int)this + 0xccc) != *(int *)((int)this + 0xcc8)) &&
       (*(int *)((int)this + 0xcd4) == 0)) {
      Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      Sprite_DrawCentered(*(void **)(*(int *)((int)this + 0x878) + 0x530),
                          (_DAT_004d6ad4 - (float)(*(int *)((int)this + 0xcb0) + 0x14)) -
                          _DAT_004cf370,520.0,in_stack_ffffff7c,in_stack_ffffff80,in_stack_ffffff84,
                          in_stack_ffffff88,uVar9);
      Vec3_Init(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000);
      Graphics_DrawPoint(param_1);
    }
  }
  ExceptionList = local_c;
  return;
}

