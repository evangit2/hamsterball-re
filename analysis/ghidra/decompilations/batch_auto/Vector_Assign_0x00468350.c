
void * __thiscall Vector_Assign(void *this,void *param_1)

{
  undefined4 *puVar1;
  undefined4 *_Memory;
  undefined4 uVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  
  if (this == param_1) {
    return this;
  }
  puVar1 = *(undefined4 **)((int)param_1 + 4);
  if (puVar1 != (undefined4 *)0x0) {
    uVar3 = (int)*(undefined4 **)((int)param_1 + 8) - (int)puVar1 >> 2;
    if (uVar3 != 0) {
      _Memory = *(undefined4 **)((int)this + 4);
      if (_Memory == (undefined4 *)0x0) {
        uVar5 = 0;
      }
      else {
        uVar5 = *(int *)((int)this + 8) - (int)_Memory >> 2;
      }
      if (uVar3 <= uVar5) {
        Array_CopyDWords3(puVar1,*(undefined4 **)((int)param_1 + 8),_Memory);
        if (*(int *)((int)param_1 + 4) != 0) {
          *(int *)((int)this + 8) =
               *(int *)((int)this + 4) +
               (*(int *)((int)param_1 + 8) - *(int *)((int)param_1 + 4) >> 2) * 4;
          return this;
        }
        *(undefined4 *)((int)this + 8) = *(undefined4 *)((int)this + 4);
        return this;
      }
      if (_Memory == (undefined4 *)0x0) {
        uVar5 = 0;
      }
      else {
        uVar5 = *(int *)((int)this + 0xc) - (int)_Memory >> 2;
      }
      if (uVar5 < uVar3) {
        _free(_Memory);
        if (*(int *)((int)param_1 + 4) == 0) {
          uVar3 = 0;
        }
        else {
          uVar3 = *(int *)((int)param_1 + 8) - *(int *)((int)param_1 + 4) >> 2;
        }
        uVar2 = Vector_Init(this,uVar3);
        if ((char)uVar2 == '\0') {
          return this;
        }
        uVar2 = Array_CopyDWordsThunk
                          (*(undefined4 **)((int)param_1 + 4),*(undefined4 **)((int)param_1 + 8),
                           *(undefined4 **)((int)this + 4));
        *(undefined4 *)((int)this + 8) = uVar2;
        return this;
      }
      if (_Memory == (undefined4 *)0x0) {
        iVar4 = 0;
      }
      else {
        iVar4 = *(int *)((int)this + 8) - (int)_Memory >> 2;
      }
      Array_CopyDWords3(puVar1,puVar1 + iVar4,_Memory);
      uVar2 = Array_CopyDWords(puVar1 + iVar4,*(undefined4 **)((int)param_1 + 8),
                               *(undefined4 **)((int)this + 8));
      *(undefined4 *)((int)this + 8) = uVar2;
      return this;
    }
  }
  if (*(void **)((int)this + 4) != (void *)0x0) {
    _free(*(void **)((int)this + 4));
  }
  *(undefined4 *)((int)this + 4) = 0;
  *(undefined4 *)((int)this + 8) = 0;
  *(undefined4 *)((int)this + 0xc) = 0;
  return this;
}

