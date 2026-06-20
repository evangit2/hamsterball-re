
int D3DTexture_CloneFromDesc16(void)

{
  uint *puVar1;
  uint uVar2;
  uint uVar3;
  void *this;
  void *pvVar4;
  int iVar5;
  int unaff_EBP;
  int *piVar6;
  
  __security_init_cookie();
  uVar2 = *(uint *)(unaff_EBP + 0x10);
  piVar6 = (int *)0x0;
  *(undefined4 *)(unaff_EBP + -0x18) = 0;
  *(undefined4 *)(unaff_EBP + -0x14) = 0;
  if (((byte)uVar2 & 0xe) == 4) {
    iVar5 = -0x7789f794;
    goto LAB_004857c7;
  }
  if ((uVar2 == 0) || ((uVar2 & 1) != 0)) {
    iVar5 = -0x7789f794;
LAB_004857a2:
    if (piVar6 == (int *)0x0) goto LAB_004857c7;
    (**(code **)(*piVar6 + 8))(piVar6);
LAB_004857ac:
    piVar6 = *(int **)(unaff_EBP + -0x14);
    if (piVar6 == (int *)0x0) goto LAB_004857c7;
    (**(code **)(*piVar6 + 8))(piVar6);
LAB_004857b9:
    piVar6 = *(int **)(unaff_EBP + -0x18);
    if (piVar6 != (int *)0x0) {
      (**(code **)(*piVar6 + 8))(piVar6);
    }
  }
  else {
    uVar3 = *(uint *)(unaff_EBP + 0xc);
    if ((uVar3 & 0xfffe2000) == 0) {
      this = *(void **)(unaff_EBP + 8);
      *(uint *)(unaff_EBP + -0x10) = uVar3;
      puVar1 = (uint *)(unaff_EBP + -0x10);
      *puVar1 = *puVar1 & 0x1000;
      if ((((*puVar1 == 0) || (uVar2 == *(uint *)((int)this + 4))) &&
          ((piVar6 = *(int **)(unaff_EBP + 0x14), *(int *)(unaff_EBP + -0x10) == 0 ||
           (piVar6 == *(int **)((int)this + 0x24))))) &&
         (((*(uint *)((int)this + 8) & 0x400) == 0 && ((*(uint *)((int)this + 8) & 0x40) == 0)))) {
        if ((uVar3 & 1) != 0) {
          pvVar4 = operator_new(0x70);
          *(void **)(unaff_EBP + 8) = pvVar4;
          *(undefined4 *)(unaff_EBP + -4) = 0;
          if (pvVar4 == (void *)0x0) {
            piVar6 = (int *)0x0;
          }
          else {
            piVar6 = D3DTexture_Init(pvVar4,piVar6,uVar2,*(uint *)(unaff_EBP + 0xc));
          }
          *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
          *(int **)(unaff_EBP + -0x18) = piVar6;
          if (piVar6 == (int *)0x0) {
            iVar5 = -0x7ff8fff2;
          }
          else {
            iVar5 = D3DTexture_CopyIndexData16(this,piVar6);
            if (-1 < iVar5) {
              if (*(int *)(unaff_EBP + -0x10) != 0) {
                *(uint *)((int)this + 0xc) = *(uint *)((int)this + 0xc) | 0x20;
                piVar6[10] = *(int *)((int)this + 0x28);
                (**(code **)(**(int **)((int)this + 0x28) + 4))(*(int **)((int)this + 0x28));
              }
              *(undefined4 *)(unaff_EBP + -0x18) = 0;
LAB_0048577c:
              if ((*(int *)(unaff_EBP + -0x10) != 0) ||
                 (iVar5 = D3DTexture_CopySurfaceData
                                    (this,*(uint *)(unaff_EBP + 0x10),(int *)piVar6[10]), -1 < iVar5
                 )) {
                **(undefined4 **)(unaff_EBP + 0x18) = piVar6;
                goto LAB_004857c7;
              }
              goto LAB_004857a2;
            }
          }
          goto LAB_004857b9;
        }
        if ((*(uint *)((int)this + 0x58) < 0x10000) && (*(uint *)((int)this + 0x30) < 0x10000)) {
          pvVar4 = operator_new(0x70);
          *(void **)(unaff_EBP + 8) = pvVar4;
          *(undefined4 *)(unaff_EBP + -4) = 1;
          if (pvVar4 == (void *)0x0) {
            piVar6 = (int *)0x0;
          }
          else {
            piVar6 = D3DTexture_InitLocked(pvVar4,piVar6,uVar2,*(uint *)(unaff_EBP + 0xc));
          }
          *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
          *(int **)(unaff_EBP + -0x14) = piVar6;
          if (piVar6 == (int *)0x0) {
            iVar5 = -0x7ff8fff2;
          }
          else {
            iVar5 = D3DTexture_CopyLockedData16(this,piVar6);
            if (-1 < iVar5) {
              if (*(int *)(unaff_EBP + -0x10) != 0) {
                *(uint *)((int)this + 0xc) = *(uint *)((int)this + 0xc) | 0x20;
                piVar6[10] = *(int *)((int)this + 0x28);
                (**(code **)(**(int **)((int)this + 0x28) + 4))(*(int **)((int)this + 0x28));
              }
              *(undefined4 *)(unaff_EBP + -0x14) = 0;
              goto LAB_0048577c;
            }
          }
          goto LAB_004857ac;
        }
      }
    }
    iVar5 = -0x7789f794;
  }
LAB_004857c7:
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return iVar5;
}

