
/* OptionsMenu_RenderControls: Iterates 4 control bindings at App+0xB28,colors by
   type(99=kbd,100=mouse),highlights duplicates in red. See decompilations/ui/decomp_menu_system.c
    */

void __fastcall OptionsMenu_RenderControls(void *param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  undefined4 local_64 [5];
  undefined4 local_50 [5];
  undefined4 local_3c [5];
  char local_28 [28];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cb298;
  local_c = ExceptionList;
  local_7c = 0;
  iVar3 = 0xb28;
  ExceptionList = &local_c;
  do {
    Matrix_Scale4x4(&local_78,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    iVar1 = *(int *)(iVar3 + *(int *)((int)param_1 + 0x878));
    iVar4 = 0;
    local_4 = 0;
    if (iVar1 == 99) {
      iVar1 = Matrix_Scale4x4(local_64,0x3f000000,0x3f800000,0x3f000000,0x3f800000);
      local_74 = *(undefined4 *)(iVar1 + 4);
      local_70 = *(undefined4 *)(iVar1 + 8);
      local_6c = *(undefined4 *)(iVar1 + 0xc);
      local_68 = *(undefined4 *)(iVar1 + 0x10);
      Matrix_Identity(local_64);
    }
    else if (iVar1 == 100) {
      iVar1 = Matrix_Scale4x4(local_50,0x3f000000,0x3f800000,0x3f800000,0x3f800000);
      local_74 = *(undefined4 *)(iVar1 + 4);
      local_70 = *(undefined4 *)(iVar1 + 8);
      local_6c = *(undefined4 *)(iVar1 + 0xc);
      local_68 = *(undefined4 *)(iVar1 + 0x10);
      Matrix_Identity(local_50);
    }
    else {
      iVar1 = 0xb28;
      do {
        if ((iVar4 != local_7c) &&
           (*(int *)(iVar3 + *(int *)((int)param_1 + 0x878)) ==
            *(int *)(iVar1 + *(int *)((int)param_1 + 0x878)))) {
          iVar2 = Matrix_Scale4x4(local_3c,0x3f800000,0,0,0x3f800000);
          local_74 = *(undefined4 *)(iVar2 + 4);
          local_70 = *(undefined4 *)(iVar2 + 8);
          local_6c = *(undefined4 *)(iVar2 + 0xc);
          local_68 = *(undefined4 *)(iVar2 + 0x10);
          Matrix_Identity(local_3c);
        }
        iVar1 = iVar1 + 4;
        iVar4 = iVar4 + 1;
      } while (iVar1 < 0xb38);
    }
    local_7c = local_7c + 1;
    AthenaString_SprintfToBuffer(local_28,(byte *)"CONTROL%d");
    UIList_SetColorsByName
              (param_1,&PTR_Vec3_dtor_004cf300,local_74,local_70,local_6c,local_68,local_28);
    local_4 = 0xffffffff;
    Matrix_Identity(&local_78);
    iVar3 = iVar3 + 4;
  } while (iVar3 < 0xb38);
  ExceptionList = local_c;
  return;
}

