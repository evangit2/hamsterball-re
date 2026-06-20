
undefined4 D3DX_CreateMeshFromDescriptor(void)

{
  int *piVar1;
  void *pvVar2;
  uint uVar3;
  void *this;
  int *this_00;
  uint uVar4;
  int iVar5;
  int unaff_EBP;
  uint uVar6;
  
  __security_init_cookie();
  uVar4 = *(uint *)(unaff_EBP + 0xc);
  if ((((uVar4 & 0xe0fe2000) != 0) || ((uVar4 & 1) != 0)) || ((uVar4 & 0x400) != 0)) {
    *(undefined4 *)(unaff_EBP + 8) = 0x8876086c;
    goto LAB_00486c66;
  }
  pvVar2 = *(void **)(unaff_EBP + 8);
  uVar3 = *(uint *)((int)pvVar2 + 8);
  if (((uVar3 & 0x400) != 0) || ((uVar3 & 0x40) != 0)) {
    *(undefined4 *)(unaff_EBP + 8) = 0x8876086c;
    goto LAB_00486c66;
  }
  uVar6 = uVar4 & 0x1cfff;
  if (uVar6 == 0) {
    if ((uVar4 & 0x1000) == 0) {
      uVar6 = uVar3 & 0xffffefff;
    }
    else {
      uVar6 = uVar3 | 0x1000;
    }
  }
  else {
    if ((uVar4 & 0x1000) != 0) {
      uVar6 = uVar6 | 0x1000;
    }
    if ((uVar3 & 1) != 0) {
      uVar6 = uVar6 | 1;
    }
    *(uint *)(unaff_EBP + 0xc) = uVar4 & 0xfffe3000;
  }
  this = operator_new(0x70);
  *(void **)(unaff_EBP + 8) = this;
  *(undefined4 *)(unaff_EBP + -4) = 0;
  if (this == (void *)0x0) {
    this_00 = (int *)0x0;
  }
  else {
    this_00 = D3DTexture_Init(this,*(int **)((int)pvVar2 + 0x24),*(uint *)((int)pvVar2 + 4),uVar6);
  }
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  *(int **)(unaff_EBP + -0x10) = this_00;
  if (this_00 == (int *)0x0) {
    *(undefined4 *)(unaff_EBP + 8) = 0x8007000e;
  }
  else {
    if ((*(byte *)(this_00 + 3) & 0x20) == 0) {
      uVar4 = 0;
    }
    else {
      uVar4 = *(uint *)((int)pvVar2 + 0x30);
    }
    iVar5 = D3DTexture_ResizeAndValidate(this_00,*(uint *)((int)pvVar2 + 0x58),uVar4);
    *(int *)(unaff_EBP + 8) = iVar5;
    if (-1 < iVar5) {
      if ((*(byte *)(this_00 + 3) & 0x20) != 0) {
        *(uint *)((int)pvVar2 + 0xc) = *(uint *)((int)pvVar2 + 0xc) | 0x20;
        this_00[10] = *(int *)((int)pvVar2 + 0x28);
        (**(code **)(**(int **)((int)pvVar2 + 0x28) + 4))(*(int **)((int)pvVar2 + 0x28));
        *(byte *)(unaff_EBP + 0xf) = *(byte *)(unaff_EBP + 0xf) | 0x10;
      }
      iVar5 = Mesh_CopyFrom(this_00,pvVar2);
      *(int *)(unaff_EBP + 8) = iVar5;
      if (-1 < iVar5) {
        if ((*(byte *)(this_00 + 3) & 0x20) == 0) {
          this_00[0xc] = *(int *)((int)pvVar2 + 0x30);
        }
        piVar1 = this_00 + 0x10;
        iVar5 = Graphics_DrawIndexedPrimitive(this_00,piVar1,0);
        *(int *)(unaff_EBP + 8) = iVar5;
        if (-1 < iVar5) {
          iVar5 = D3DXMesh_OptimizeFull
                            (this_00,*(uint **)(unaff_EBP + 0xc),*(uint **)(unaff_EBP + 0x10),
                             *(uint **)(unaff_EBP + 0x14),*(uint **)(unaff_EBP + 0x18),
                             *(undefined4 **)(unaff_EBP + 0x1c),*(int **)((int)pvVar2 + 0x28),
                             *(uint **)((int)pvVar2 + 4));
          *(int *)(unaff_EBP + 8) = iVar5;
          if (-1 < iVar5) {
            if (*piVar1 != 0) {
              (**(code **)(*(int *)this_00[0xf] + 0x30))((int *)this_00[0xf]);
              *piVar1 = 0;
            }
            **(undefined4 **)(unaff_EBP + 0x20) = this_00;
            this_00 = (int *)0x0;
            goto LAB_00486c4a;
          }
        }
        this_00 = *(int **)(unaff_EBP + -0x10);
      }
    }
  }
LAB_00486c4a:
  if (this_00 != (int *)0x0) {
    if (this_00[0x10] != 0) {
      (**(code **)(*(int *)this_00[0xf] + 0x30))((int *)this_00[0xf]);
    }
    (**(code **)(*this_00 + 8))(this_00);
  }
LAB_00486c66:
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return *(undefined4 *)(unaff_EBP + 8);
}

