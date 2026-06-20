
void __thiscall Graphics_ApplyMaterialAndDraw(void *this,undefined4 *param_1)

{
  char cVar1;
  int iVar2;
  int *piVar3;
  byte bVar4;
  undefined4 *this_00;
  undefined4 uVar5;
  
  this_00 = *(undefined4 **)((int)this + 0x7c0);
  if (*(undefined4 **)((int)this + 0x7c0) == (undefined4 *)0x0) {
    this_00 = param_1;
  }
  if (*(char *)((int)this + 0x7a8) != '\0') {
    this_00 = Matrix_ScaleTransform
                        (this_00,&PTR_Vec3_dtor_004cf300,*(float *)((int)this + 0x7b0),
                         *(float *)((int)this + 0x7b4),*(float *)((int)this + 0x7b8),
                         *(float *)((int)this + 0x7bc));
  }
  iVar2 = *(int *)((int)this + 0x7c8);
  *(int *)((int)this + 0x7c8) = iVar2 + 1;
  if (this_00[0x12] == 0) {
    (**(code **)(**(int **)((int)this + 0x154) + 0xfc))();
    cVar1 = *(char *)(this_00 + 0x13);
    if (*(char *)((int)this + 0x700) != cVar1) {
      *(char *)((int)this + 0x700) = cVar1;
      piVar3 = *(int **)((int)this + 0x154);
      *(int *)((int)this + 0x7c8) = *(int *)((int)this + 0x7c8) + 1;
      if (cVar1 == '\x01') {
        (**(code **)(*piVar3 + 200))(piVar3,0x1b);
        uVar5 = 4;
      }
      else {
        (**(code **)(*piVar3 + 200))(piVar3,0x1b);
        uVar5 = 2;
      }
      (**(code **)(**(int **)((int)this + 0x154) + 0xfc))(*(int **)((int)this + 0x154),0,4,uVar5);
    }
    (**(code **)(**(int **)((int)this + 0x154) + 0xf4))(*(int **)((int)this + 0x154),0);
  }
  else {
    bVar4 = *(byte *)(this_00[0x12] + 0x1c) | *(byte *)(this_00 + 0x13);
    if (*(byte *)((int)this + 0x700) != bVar4) {
      *(byte *)((int)this + 0x700) = bVar4;
      *(int *)((int)this + 0x7c8) = iVar2 + 2;
      if (bVar4 == 1) {
        (**(code **)(**(int **)((int)this + 0x154) + 200))();
      }
      else {
        (**(code **)(**(int **)((int)this + 0x154) + 200))();
      }
      (**(code **)(**(int **)((int)this + 0x154) + 0xfc))(*(int **)((int)this + 0x154),0);
    }
    (**(code **)(**(int **)((int)this + 0x154) + 0xf4))();
    cVar1 = *(char *)(this_00[0x12] + 0x1e);
    if (*(char *)((int)this + 5) != cVar1) {
      piVar3 = *(int **)((int)this + 0x154);
      if (cVar1 == '\0') {
        (**(code **)(*piVar3 + 0xfc))(piVar3,0);
        uVar5 = 2;
      }
      else {
        (**(code **)(*piVar3 + 0xfc))(piVar3,0);
        uVar5 = 1;
      }
      (**(code **)(**(int **)((int)this + 0x154) + 0xfc))(*(int **)((int)this + 0x154),0,0x11,uVar5)
      ;
      *(char *)((int)this + 5) = cVar1;
    }
    piVar3 = *(int **)((int)this + 0x154);
    if (*(char *)(this_00[0x12] + 0x1d) == '\0') {
      (**(code **)(*piVar3 + 0xfc))(piVar3,0);
    }
    else {
      (**(code **)(*piVar3 + 0xfc))(piVar3,0);
    }
  }
  (**(code **)(**(int **)((int)this + 0x154) + 0xa8))(*(int **)((int)this + 0x154),this_00 + 1);
  piVar3 = *(int **)((int)this + 0x154);
  if (*(char *)((int)this_00 + 0x4d) == '\0') {
    (**(code **)(*piVar3 + 200))(piVar3,0x39,0);
  }
  else {
    (**(code **)(*piVar3 + 200))(piVar3,0x39,1);
  }
  Level_SetObjectTransform((void *)((int)this + 0xc),(int)this_00);
  if (*(char *)((int)this + 0x7a8) != '\0') {
    (**(code **)*this_00)(1);
  }
  return;
}

