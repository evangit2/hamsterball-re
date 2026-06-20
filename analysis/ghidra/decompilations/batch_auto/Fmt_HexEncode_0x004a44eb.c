
void __thiscall Fmt_HexEncode(void *this,int param_1,undefined4 *param_2)

{
  undefined4 *puVar1;
  byte bVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  undefined4 *puVar7;
  
  iVar3 = 0;
  iVar6 = 0;
  do {
    bVar2 = *(byte *)(param_1 + 0x10c + iVar6);
    uVar5 = (uint)bVar2;
    iVar6 = iVar6 + 1;
    if (((uVar5 < 0x29) || (0x7a < uVar5)) || ((0x5a < uVar5 && (uVar5 < 0x61)))) {
      *(undefined1 *)(iVar3 + (int)this) = 0x5b;
      *(char *)(iVar3 + 1 + (int)this) = "0123456789ABCDEF1.0.5"[(int)uVar5 >> 4];
      *(char *)(iVar3 + 2 + (int)this) = "0123456789ABCDEF1.0.5"[uVar5 & 0xf];
      iVar4 = iVar3 + 3;
      *(undefined1 *)(iVar4 + (int)this) = 0x5d;
    }
    else {
      *(byte *)(iVar3 + (int)this) = bVar2;
      iVar4 = iVar3;
    }
    iVar3 = iVar4 + 1;
  } while (iVar6 < 4);
  if (param_2 == (undefined4 *)0x0) {
    *(undefined1 *)(iVar3 + (int)this) = 0;
  }
  else {
    *(undefined1 *)(iVar3 + (int)this) = 0x3a;
    *(undefined1 *)(iVar4 + 2 + (int)this) = 0x20;
    puVar1 = (undefined4 *)(iVar4 + 3 + (int)this);
    puVar7 = puVar1;
    for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
      *puVar7 = *param_2;
      param_2 = param_2 + 1;
      puVar7 = puVar7 + 1;
    }
    *(undefined1 *)((int)puVar1 + 0x3f) = 0;
  }
  return;
}

