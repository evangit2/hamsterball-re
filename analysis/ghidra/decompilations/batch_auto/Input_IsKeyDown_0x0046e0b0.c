
uint __thiscall Input_IsKeyDown(void *this,int param_1)

{
  uint uVar1;
  undefined3 uVar3;
  byte *pbVar2;
  int iVar4;
  byte bVar5;
  
  uVar1 = *(int *)((int)this + 8) - 1;
  uVar3 = (undefined3)((uint)param_1 >> 8);
  switch(uVar1) {
  case 0:
    if (param_1 == -1) {
      bVar5 = 0;
      pbVar2 = (byte *)(*(int *)(*(int *)((int)this + 4) + 0x434) + 0x12);
      iVar4 = 0x20;
      do {
        bVar5 = bVar5 | pbVar2[-6] | pbVar2[-5] | pbVar2[-4] | pbVar2[-3] | pbVar2[-2] | pbVar2[-1]
                        | pbVar2[1] | *pbVar2;
        pbVar2 = pbVar2 + 8;
        iVar4 = iVar4 + -1;
      } while (iVar4 != 0);
      return (uint)(bVar5 != 0);
    }
    if (param_1 != 0) {
      if (param_1 != 1) {
        return CONCAT31(uVar3,*(char *)(*(int *)(*(int *)((int)this + 4) + 0x434) + 0xc + param_1)
                              != '\0');
      }
      uVar1 = App_Is2PMode(*(int *)(*(int *)((int)this + 4) + 0x434));
      return uVar1;
    }
    return (uint)(*(byte *)(*(int *)(*(int *)((int)this + 4) + 0x434) + 0x45) >> 7);
  case 1:
    uVar1 = param_1 + 1;
    switch(uVar1) {
    case 0:
      iVar4 = *(int *)(*(int *)((int)this + 4) + 4);
      return (uint)(byte)(*(byte *)(iVar4 + 0x1ca) | *(byte *)(iVar4 + 0x1c9) |
                         *(byte *)(iVar4 + 0x1c8));
    case 1:
      iVar4 = *(int *)(*(int *)((int)this + 4) + 4);
      return CONCAT31((int3)((uint)iVar4 >> 8),*(undefined1 *)(iVar4 + 0x1c8));
    case 2:
      return CONCAT31((int3)(uVar1 >> 8),
                      *(undefined1 *)(*(int *)(*(int *)((int)this + 4) + 4) + 0x1c9));
    case 3:
      return CONCAT31((int3)((uint)*(int *)((int)this + 4) >> 8),
                      *(undefined1 *)(*(int *)(*(int *)((int)this + 4) + 4) + 0x1ca));
    }
    break;
  case 3:
  case 4:
  case 5:
  case 6:
    iVar4 = *(int *)((int)this + 0x10);
    if (iVar4 != 0) {
      if (param_1 == -1) {
        return CONCAT31(0xffffff,((*(char *)(iVar4 + 0x133) != '\0' ||
                                  *(char *)(iVar4 + 0x132) != '\0') ||
                                 *(char *)(iVar4 + 0x131) != '\0') ||
                                 *(char *)(iVar4 + 0x130) != '\0');
      }
      return CONCAT31(uVar3,*(char *)(iVar4 + 0x130 + param_1) != '\0');
    }
  }
  return uVar1 & 0xffffff00;
}

