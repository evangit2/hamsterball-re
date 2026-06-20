
void * __thiscall D3D_InitDisplayModes(void *this,uint param_1,uint param_2)

{
  undefined4 uVar1;
  uint uVar2;
  uint *puVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  
  VertexDecl_Init(this,param_1);
  VertexDecl_Init((void *)((int)this + 0x60),param_2);
  D3D_GetAdapterModes(this,(int)this + 0x40);
  puVar3 = (uint *)((int)this + 0xa0);
  D3D_GetAdapterModes((void *)((int)this + 0x60),(int)puVar3);
  uVar6 = *(uint *)((int)this + 0x9c);
  if (*(uint *)((int)this + 0x3c) < *(uint *)((int)this + 0x9c)) {
    uVar6 = *(uint *)((int)this + 0x3c);
  }
  uVar7 = 0;
  *(uint *)((int)this + 0xc0) = uVar6;
  if (uVar6 != 0) {
    do {
      uVar6 = *puVar3;
      uVar5 = puVar3[-0x18];
      if (uVar6 < puVar3[-0x18]) {
        uVar5 = uVar6;
      }
      puVar3[9] = uVar5;
      puVar3[0x11] = uVar6 - uVar5;
      uVar7 = uVar7 + 1;
      puVar3 = puVar3 + 1;
    } while (uVar7 < *(uint *)((int)this + 0xc0));
  }
  if ((*(int *)((int)this + 8) == 0) || (*(int *)((int)this + 0x68) == 0)) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  *(undefined4 *)((int)this + 0x10c) = uVar1;
  if ((*(int *)((int)this + 0x10) == 0) || (*(int *)((int)this + 0x70) == 0)) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  *(undefined4 *)((int)this + 0x110) = uVar1;
  if ((*(int *)((int)this + 0x14) == 0) || (*(int *)((int)this + 0x74) == 0)) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  *(undefined4 *)((int)this + 0x114) = uVar1;
  if ((*(int *)((int)this + 0xc) == 0) || (*(int *)((int)this + 0x6c) == 0)) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  *(undefined4 *)((int)this + 0x118) = uVar1;
  uVar6 = *(uint *)this & 0x1000;
  if ((((uVar6 == 0) || (*(int *)((int)this + 0x38) == 0)) ||
      ((*(uint *)((int)this + 0x60) & 0x1000) == 0)) || (*(int *)((int)this + 0x98) == 0)) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  *(undefined4 *)((int)this + 0x11c) = uVar1;
  if (((*(uint *)((int)this + 0x60) & 0x1000) == 0) || (*(int *)((int)this + 0x98) == 0)) {
    uVar7 = *(uint *)((int)this + 0x98);
  }
  else {
    uVar7 = *(int *)((int)this + 0x98) - 1;
  }
  if ((uVar6 == 0) || (uVar5 = *(uint *)((int)this + 0x38), uVar5 == 0)) {
    uVar2 = *(uint *)((int)this + 0x38);
    uVar5 = uVar2;
  }
  else {
    uVar2 = uVar5 - 1;
  }
  if (uVar7 < uVar2) {
    uVar2 = uVar7;
  }
  iVar4 = uVar7 - uVar2;
  *(uint *)((int)this + 0x104) = uVar2;
  *(int *)((int)this + 0x108) = iVar4;
  if (((uVar6 == 0) || (uVar5 == 0)) || (iVar4 == 0)) {
    *(undefined4 *)((int)this + 0x120) = 0;
  }
  else {
    *(int *)((int)this + 0x108) = iVar4 + -1;
    *(undefined4 *)((int)this + 0x120) = 1;
  }
  return this;
}

