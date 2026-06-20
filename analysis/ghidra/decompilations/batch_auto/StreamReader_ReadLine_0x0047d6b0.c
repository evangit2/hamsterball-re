
void * __thiscall StreamReader_ReadLine(void *this,char param_1)

{
  bool bVar1;
  char cVar2;
  void *pvVar3;
  int iVar4;
  size_t _NewSize;
  int iVar5;
  undefined4 uStack_4;
  
  iVar5 = 0;
  uStack_4 = this;
  if (*(void **)((int)this + 8) != (void *)0x0) {
    _free(*(void **)((int)this + 8));
    *(undefined4 *)((int)this + 8) = 0;
  }
  _NewSize = 0x400;
  pvVar3 = _malloc(0x400);
  *(void **)((int)this + 8) = pvVar3;
  bVar1 = true;
  uStack_4._0_3_ = (uint3)(ushort)uStack_4;
  iVar4 = __read(*(int *)((int)this + 4),(void *)((int)&uStack_4 + 3),1);
  if (0 < (short)iVar4) {
    do {
      if (uStack_4._3_1_ == '\r') break;
      if (('\x1f' < uStack_4._3_1_) || (!bVar1)) {
        if ((param_1 == '\x01') && (uStack_4._3_1_ < ' ')) {
          uStack_4 = (void *)CONCAT13(0x20,(uint3)uStack_4);
          cVar2 = ' ';
LAB_0047d72d:
          if ((uStack_4._2_1_ == '\x01') && (param_1 == '\x01')) goto LAB_0047d772;
          uStack_4._0_3_ = CONCAT12(1,(ushort)uStack_4);
        }
        else {
          cVar2 = uStack_4._3_1_;
          if (uStack_4._3_1_ == ' ') goto LAB_0047d72d;
          uStack_4._0_3_ = (uint3)(ushort)uStack_4;
        }
        *(char *)(iVar5 + *(int *)((int)this + 8)) = cVar2;
        bVar1 = false;
        *(undefined1 *)(iVar5 + 1 + *(int *)((int)this + 8)) = 0;
        iVar5 = iVar5 + 1;
        if ((int)(_NewSize - 1) <= iVar5) {
          _NewSize = _NewSize + 0x400;
          pvVar3 = _realloc(*(void **)((int)this + 8),_NewSize);
          *(void **)((int)this + 8) = pvVar3;
        }
      }
LAB_0047d772:
      iVar4 = __read(*(int *)((int)this + 4),(void *)((int)&uStack_4 + 3),1);
    } while (0 < (short)iVar4);
    if (iVar5 != 0) {
      pvVar3 = _realloc(*(void **)((int)this + 8),iVar5 + 1);
      *(void **)((int)this + 8) = pvVar3;
      return pvVar3;
    }
  }
  return (void *)0x0;
}

