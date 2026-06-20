
uint __thiscall
StdString_CompareSubstr(void *this,uint param_1,uint param_2,byte *param_3,uint param_4)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  byte *pbVar4;
  bool bVar5;
  bool bVar6;
  
  if (*(uint *)((int)this + 0x14) < param_1) {
    CRT_ThrowInvalidStringPosition();
  }
  uVar1 = *(int *)((int)this + 0x14) - param_1;
  if (uVar1 < param_2) {
    param_2 = uVar1;
  }
  if (param_2 != 0) {
    uVar1 = param_2;
    if (param_4 <= param_2) {
      uVar1 = param_4;
    }
    if (*(uint *)((int)this + 0x18) < 0x10) {
      iVar2 = (int)this + 4;
    }
    else {
      iVar2 = *(int *)((int)this + 4);
    }
    bVar5 = false;
    uVar3 = 0;
    bVar6 = true;
    pbVar4 = (byte *)(iVar2 + param_1);
    do {
      if (uVar1 == 0) break;
      uVar1 = uVar1 - 1;
      bVar5 = *pbVar4 < *param_3;
      bVar6 = *pbVar4 == *param_3;
      pbVar4 = pbVar4 + 1;
      param_3 = param_3 + 1;
    } while (bVar6);
    if (!bVar6) {
      uVar3 = (1 - (uint)bVar5) - (uint)(bVar5 != 0);
    }
    if (uVar3 != 0) {
      return uVar3;
    }
  }
  if (param_4 <= param_2) {
    return (uint)(param_2 != param_4);
  }
  return 0xffffffff;
}

