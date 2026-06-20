
void __thiscall
Vector_InsertN(void *this,undefined4 *param_1,undefined4 *param_2,undefined4 *param_3)

{
  void *_Memory;
  undefined4 *puVar1;
  uint uVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  undefined4 uVar6;
  int iVar7;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puVar4 = param_2;
  local_8 = 0xffffffff;
  puStack_c = &LAB_004cd370;
  local_10 = ExceptionList;
  param_3 = (undefined4 *)*param_3;
  iVar3 = *(int *)((int)this + 4);
  if (iVar3 == 0) {
    uVar2 = 0;
  }
  else {
    uVar2 = *(int *)((int)this + 0xc) - iVar3 >> 2;
  }
  if (param_2 != (undefined4 *)0x0) {
    if (iVar3 == 0) {
      iVar7 = 0;
    }
    else {
      iVar7 = *(int *)((int)this + 8) - iVar3 >> 2;
    }
    if ((undefined4 *)(0x3fffffffU - iVar7) < param_2) {
      ExceptionList = &local_10;
      Exception_ThrowVectorLength();
      ExceptionList = local_10;
      return;
    }
    if (iVar3 == 0) {
      iVar7 = 0;
    }
    else {
      iVar7 = *(int *)((int)this + 8) - iVar3 >> 2;
    }
    if (uVar2 < (uint)(iVar7 + (int)param_2)) {
      if (0x3fffffff - (uVar2 >> 1) < uVar2) {
        uVar2 = 0;
      }
      else {
        uVar2 = uVar2 + (uVar2 >> 1);
      }
      if (iVar3 == 0) {
        iVar7 = 0;
      }
      else {
        iVar7 = *(int *)((int)this + 8) - iVar3 >> 2;
      }
      if (uVar2 < (uint)(iVar7 + (int)param_2)) {
        if (iVar3 == 0) {
          iVar3 = 0;
        }
        else {
          iVar3 = *(int *)((int)this + 8) - iVar3 >> 2;
        }
        uVar2 = iVar3 + (int)param_2;
      }
      ExceptionList = &local_10;
      puVar4 = operator_new(uVar2 * 4);
      local_8 = 0;
      puVar5 = (undefined4 *)Array_CopyDWords(*(undefined4 **)((int)this + 4),param_1,puVar4);
      Array_FillDWords(puVar5,(int)param_2,&param_3);
      Array_CopyDWords(param_1,*(undefined4 **)((int)this + 8),puVar5 + (int)param_2);
      _Memory = *(void **)((int)this + 4);
      if (_Memory == (void *)0x0) {
        iVar3 = 0;
      }
      else {
        iVar3 = *(int *)((int)this + 8) - (int)_Memory >> 2;
      }
      iVar3 = (int)param_2 + iVar3;
      if (_Memory != (void *)0x0) {
        _free(_Memory);
      }
      *(undefined4 **)((int)this + 0xc) = puVar4 + uVar2;
      *(undefined4 **)((int)this + 8) = puVar4 + iVar3;
      *(undefined4 **)((int)this + 4) = puVar4;
      ExceptionList = local_10;
      return;
    }
    puVar5 = *(undefined4 **)((int)this + 8);
    if ((undefined4 *)((int)puVar5 - (int)param_1 >> 2) < param_2) {
      iVar3 = (int)param_2 * 4;
      ExceptionList = &local_10;
      Array_CopyDWords4(param_1,puVar5,param_1 + (int)param_2);
      local_8 = 2;
      Array_FillAndAdvance
                (*(undefined4 **)((int)this + 8),
                 (int)param_2 - ((int)*(undefined4 **)((int)this + 8) - (int)param_1 >> 2),&param_3)
      ;
      iVar3 = *(int *)((int)this + 8) + iVar3;
      *(int *)((int)this + 8) = iVar3;
      puVar4 = (undefined4 *)(iVar3 + (int)puVar4 * -4);
    }
    else {
      param_2 = puVar5 + -(int)param_2;
      ExceptionList = &local_10;
      uVar6 = Array_CopyDWords4(param_2,puVar5,puVar5);
      puVar1 = param_2;
      *(undefined4 *)((int)this + 8) = uVar6;
      param_2 = (undefined4 *)&stack0xffffffcc;
      Array_CopyBackward(&param_2,(int)param_1,(int)puVar1,puVar5);
      puVar4 = param_1 + (int)puVar4;
    }
    param_2 = (undefined4 *)&stack0xffffffcc;
    Array_Fill(param_1,puVar4,&param_3);
  }
  ExceptionList = local_10;
  return;
}

