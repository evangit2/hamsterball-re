
void * __thiscall InputHandler_Ctor(void *this,undefined4 param_1)

{
  void *this_00;
  int *piVar1;
  HMODULE pHVar2;
  int iVar3;
  void *pvVar4;
  int iVar5;
  int *unaff_ESI;
  undefined4 *puVar6;
  undefined4 uVar7;
  undefined *puVar8;
  undefined4 *puVar9;
  undefined4 uVar10;
  undefined1 auStack_10 [4];
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd766;
  pvStack_c = ExceptionList;
  this_00 = (void *)((int)this + 0xc);
  ExceptionList = &pvStack_c;
  *(undefined ***)this = &PTR_SceneObject_DeletingDtor_004d9bfc;
  AthenaList_Init(this_00,0);
  *(undefined4 *)((int)this + 4) = param_1;
  uVar10 = 0;
  *(undefined4 *)((int)this + 8) = 0;
  *(undefined4 *)((int)this + 0x434) = 0;
  puVar6 = (undefined4 *)((int)this + 8);
  *(undefined4 *)((int)this + 0x424) = 0;
  puVar8 = &DAT_004db1fc;
  *(undefined4 *)((int)this + 0x428) = 0;
  uVar7 = 0x800;
  *(undefined4 *)((int)this + 0x42c) = 0;
  local_4 = 0;
  *(undefined4 *)((int)this + 0x430) = 0;
  puVar9 = puVar6;
  pHVar2 = GetModuleHandleA((LPCSTR)0x0);
  iVar3 = DirectInput8Create(pHVar2,uVar7,puVar8,puVar9,uVar10);
  if (iVar3 == 0) {
    iVar3 = (**(code **)(*(int *)*puVar6 + 0xc))((int *)*puVar6,&DAT_004db15c,auStack_10,0);
    if (-1 < iVar3) {
      pvVar4 = operator_new(0x524);
      if (pvVar4 == (void *)0x0) {
        pvVar4 = (void *)0x0;
      }
      else {
        pvVar4 = KeyboardDevice_Ctor(pvVar4,(int)this,unaff_ESI);
      }
      *(void **)((int)this + 0x434) = pvVar4;
    }
    (**(code **)(*(int *)*puVar6 + 0x10))((int *)*puVar6,4,&LAB_0046eaf0,this,1);
    iVar3 = AthenaList_NextIndex((int)this_00);
    *(undefined4 *)((int)this + iVar3 * 4 + 0x14) = 0;
    if (*(int *)((int)this + 0x10) < 1) {
      puVar6 = (undefined4 *)0x0;
    }
    else {
      puVar6 = (undefined4 *)**(undefined4 **)((int)this + 0x418);
      *(undefined4 *)((int)this + iVar3 * 4 + 0x14) = 1;
    }
    while (puVar6 != (undefined4 *)0x0) {
      iVar5 = (**(code **)(*(int *)puVar6[2] + 0x2c))((int *)puVar6[2],&DAT_004db144);
      if ((iVar5 < 0) ||
         (iVar5 = (**(code **)(*(int *)puVar6[2] + 0x34))
                            ((int *)puVar6[2],*(undefined4 *)(*(int *)((int)this + 4) + 8),5),
         iVar5 < 0)) {
LAB_0046ef5d:
        thunk_Gfx_SetRenderState(this_00,(int)puVar6);
        (**(code **)*puVar6)(1);
      }
      else {
        piVar1 = (int *)puVar6[2];
        iVar5 = (**(code **)(*piVar1 + 0x10))(piVar1,&LAB_0046df40,piVar1,0);
        if (iVar5 < 0) goto LAB_0046ef5d;
      }
      iVar5 = *(int *)((int)this + iVar3 * 4 + 0x14);
      if (*(int *)((int)this + 0x10) <= iVar5) break;
      puVar6 = *(undefined4 **)(*(int *)((int)this + 0x418) + iVar5 * 4);
      *(int *)((int)this + iVar3 * 4 + 0x14) = iVar5 + 1;
    }
  }
  if (*(int *)((int)this + 0x10) < 1) {
    uVar7 = 0;
  }
  else {
    uVar7 = **(undefined4 **)((int)this + 0x418);
  }
  *(undefined4 *)((int)this + 0x424) = uVar7;
  if (*(int *)((int)this + 0x10) < 2) {
    uVar7 = 0;
  }
  else {
    uVar7 = *(undefined4 *)(*(int *)((int)this + 0x418) + 4);
  }
  *(undefined4 *)((int)this + 0x428) = uVar7;
  if (*(int *)((int)this + 0x10) < 3) {
    uVar7 = 0;
  }
  else {
    uVar7 = *(undefined4 *)(*(int *)((int)this + 0x418) + 8);
  }
  *(undefined4 *)((int)this + 0x42c) = uVar7;
  if (*(int *)((int)this + 0x10) < 4) {
    uVar7 = 0;
  }
  else {
    uVar7 = *(undefined4 *)(*(int *)((int)this + 0x418) + 0xc);
  }
  *(undefined4 *)((int)this + 0x430) = uVar7;
  ExceptionList = unaff_ESI;
  return this;
}

