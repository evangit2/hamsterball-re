
/* WARNING (jumptable): Unable to track spacebase fully for stack */

int * D3DX_CreateMeshFromFormat(void)

{
  int iVar1;
  void *pvVar2;
  undefined4 *puVar3;
  int unaff_EBP;
  int *piVar4;
  
  __security_init_cookie();
  puVar3 = *(undefined4 **)(unaff_EBP + 8);
  iVar1 = puVar3[1];
  piVar4 = (int *)0x0;
  if (iVar1 < 0x31545845) {
    if (iVar1 == 0x31545844) {
      pvVar2 = operator_new(0x10c4);
      *(void **)(unaff_EBP + 8) = pvVar2;
      *(undefined4 *)(unaff_EBP + -4) = 0x20;
      if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
      piVar4 = MeshNode_ctor_SetVFmt_A(pvVar2,puVar3);
    }
    else if (iVar1 < 0x29) {
      if (iVar1 == 0x28) {
        pvVar2 = operator_new(0x106c);
        *(void **)(unaff_EBP + 8) = pvVar2;
        *(undefined4 *)(unaff_EBP + -4) = 0xd;
        if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
        piVar4 = MeshTexture_InitVolume(pvVar2,puVar3);
      }
      else {
        switch(iVar1) {
        case 0x14:
          pvVar2 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar2;
          *(undefined4 *)(unaff_EBP + -4) = 0;
          if (pvVar2 == (void *)0x0) {
LAB_00497dd1:
            piVar4 = (int *)0x0;
          }
          else {
            piVar4 = MeshNode_ctor_24(pvVar2,puVar3);
          }
          break;
        case 0x15:
          pvVar2 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar2;
          *(undefined4 *)(unaff_EBP + -4) = 1;
          if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
          piVar4 = MeshNode_ctor_32(pvVar2,puVar3);
          break;
        case 0x16:
          pvVar2 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar2;
          *(undefined4 *)(unaff_EBP + -4) = 2;
          if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
          piVar4 = MeshNode_ctor_32b(pvVar2,puVar3);
          break;
        case 0x17:
          pvVar2 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar2;
          *(undefined4 *)(unaff_EBP + -4) = 3;
          if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
          piVar4 = MeshNode_ctor_16(pvVar2,puVar3);
          break;
        case 0x18:
          pvVar2 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar2;
          *(undefined4 *)(unaff_EBP + -4) = 4;
          if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
          piVar4 = MeshNode_ctor_16b(pvVar2,puVar3);
          break;
        case 0x19:
          pvVar2 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar2;
          *(undefined4 *)(unaff_EBP + -4) = 5;
          if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
          piVar4 = MeshNode_ctor_16c(pvVar2,puVar3);
          break;
        case 0x1a:
          pvVar2 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar2;
          *(undefined4 *)(unaff_EBP + -4) = 6;
          if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
          piVar4 = MeshTexture_Init3D(pvVar2,puVar3);
          break;
        case 0x1b:
          pvVar2 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar2;
          *(undefined4 *)(unaff_EBP + -4) = 7;
          if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
          piVar4 = MeshNode_ctor_8(pvVar2,puVar3);
          break;
        case 0x1c:
          pvVar2 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar2;
          *(undefined4 *)(unaff_EBP + -4) = 8;
          if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
          piVar4 = MeshNode_ctor_8b(pvVar2,puVar3);
          break;
        case 0x1d:
          pvVar2 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar2;
          *(undefined4 *)(unaff_EBP + -4) = 9;
          if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
          piVar4 = MeshNode_ctor_16d(pvVar2,puVar3);
          break;
        case 0x1e:
          pvVar2 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar2;
          *(undefined4 *)(unaff_EBP + -4) = 10;
          if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
          piVar4 = MeshNode_ctor_16e(pvVar2,puVar3);
          break;
        case 0x1f:
          pvVar2 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar2;
          *(undefined4 *)(unaff_EBP + -4) = 0xb;
          if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
          piVar4 = MeshNode_ctor_32(pvVar2,puVar3);
          break;
        default:
          goto switchD_0049771c_caseD_20;
        case 0x22:
          pvVar2 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar2;
          *(undefined4 *)(unaff_EBP + -4) = 0xc;
          if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
          piVar4 = MeshNode_ctor_32b(pvVar2,puVar3);
        }
      }
    }
    else if (iVar1 < 0x3f) {
      if (iVar1 == 0x3e) {
        pvVar2 = operator_new(0x106c);
        *(void **)(unaff_EBP + 8) = pvVar2;
        *(undefined4 *)(unaff_EBP + -4) = 0x14;
        if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
        piVar4 = MeshNode_ctor_32x2(pvVar2,puVar3);
      }
      else if (iVar1 == 0x29) {
        pvVar2 = operator_new(0x106c);
        *(void **)(unaff_EBP + 8) = pvVar2;
        *(undefined4 *)(unaff_EBP + -4) = 0xe;
        if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
        piVar4 = MeshNode_ctor_8c(pvVar2,puVar3);
      }
      else if (iVar1 == 0x32) {
        pvVar2 = operator_new(0x106c);
        *(void **)(unaff_EBP + 8) = pvVar2;
        *(undefined4 *)(unaff_EBP + -4) = 0xf;
        if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
        piVar4 = MeshNode_ctor_8d(pvVar2,puVar3);
      }
      else if (iVar1 == 0x33) {
        pvVar2 = operator_new(0x106c);
        *(void **)(unaff_EBP + 8) = pvVar2;
        *(undefined4 *)(unaff_EBP + -4) = 0x10;
        if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
        piVar4 = MeshNode_ctor_16f(pvVar2,puVar3);
      }
      else if (iVar1 == 0x34) {
        pvVar2 = operator_new(0x106c);
        *(void **)(unaff_EBP + 8) = pvVar2;
        *(undefined4 *)(unaff_EBP + -4) = 0x11;
        if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
        piVar4 = MeshNode_ctor_8e(pvVar2,puVar3);
      }
      else if (iVar1 == 0x3c) {
        pvVar2 = operator_new(0x106c);
        *(void **)(unaff_EBP + 8) = pvVar2;
        *(undefined4 *)(unaff_EBP + -4) = 0x12;
        if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
        piVar4 = MeshNode_ctor_16x2(pvVar2,puVar3);
      }
      else {
        if (iVar1 != 0x3d) goto switchD_0049771c_caseD_20;
        pvVar2 = operator_new(0x106c);
        *(void **)(unaff_EBP + 8) = pvVar2;
        *(undefined4 *)(unaff_EBP + -4) = 0x13;
        if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
        piVar4 = MeshNode_ctor_16bx2(pvVar2,puVar3);
      }
    }
    else if (iVar1 == 0x3f) {
      pvVar2 = operator_new(0x106c);
      *(void **)(unaff_EBP + 8) = pvVar2;
      *(undefined4 *)(unaff_EBP + -4) = 0x15;
      if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
      piVar4 = MeshNode_ctor_32x3(pvVar2,puVar3);
    }
    else if (iVar1 == 0x40) {
      pvVar2 = operator_new(0x106c);
      *(void **)(unaff_EBP + 8) = pvVar2;
      *(undefined4 *)(unaff_EBP + -4) = 0x16;
      if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
      piVar4 = MeshNode_ctor_32cx2(pvVar2,puVar3);
    }
    else if (iVar1 == 0x41) {
      pvVar2 = operator_new(0x106c);
      *(void **)(unaff_EBP + 8) = pvVar2;
      *(undefined4 *)(unaff_EBP + -4) = 0x17;
      if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
      piVar4 = MeshNode_ctor_32dx2(pvVar2,puVar3);
    }
    else if (iVar1 == 0x43) {
      pvVar2 = operator_new(0x106c);
      *(void **)(unaff_EBP + 8) = pvVar2;
      *(undefined4 *)(unaff_EBP + -4) = 0x18;
      if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
      piVar4 = MeshNode_ctor_32ex2(pvVar2,puVar3);
    }
    else {
      if (iVar1 != 0x46) goto switchD_0049771c_caseD_20;
      pvVar2 = operator_new(0x106c);
      *(void **)(unaff_EBP + 8) = pvVar2;
      *(undefined4 *)(unaff_EBP + -4) = 0x19;
      if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
      piVar4 = MeshNode_ctor_16g(pvVar2,puVar3);
    }
  }
  else if (iVar1 < 0x36314c21) {
    if (iVar1 == 0x36314c20) {
      pvVar2 = operator_new(0x106c);
      *(void **)(unaff_EBP + 8) = pvVar2;
      *(undefined4 *)(unaff_EBP + -4) = 0x1a;
      if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
      piVar4 = MeshNode_ctor_16h(pvVar2,puVar3);
    }
    else if (iVar1 == 0x32545844) {
      pvVar2 = operator_new(0x10c4);
      *(void **)(unaff_EBP + 8) = pvVar2;
      *(undefined4 *)(unaff_EBP + -4) = 0x21;
      if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
      piVar4 = MeshNode_ctor_SetVFmt_B(pvVar2,puVar3);
    }
    else if (iVar1 == 0x32595559) {
      puVar3 = operator_new(0x109c);
      *(undefined4 **)(unaff_EBP + 8) = puVar3;
      *(undefined4 *)(unaff_EBP + -4) = 0x1f;
      if (puVar3 == (undefined4 *)0x0) goto LAB_00497dd1;
      piVar4 = MeshTexture_Init2D_B(puVar3);
    }
    else if (iVar1 == 0x33545844) {
      pvVar2 = operator_new(0x10c4);
      *(void **)(unaff_EBP + 8) = pvVar2;
      *(undefined4 *)(unaff_EBP + -4) = 0x22;
      if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
      piVar4 = MeshNode_ctor_SetVFmt_C(pvVar2,puVar3);
    }
    else if (iVar1 == 0x34545844) {
      pvVar2 = operator_new(0x10c4);
      *(void **)(unaff_EBP + 8) = pvVar2;
      *(undefined4 *)(unaff_EBP + -4) = 0x23;
      if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
      piVar4 = MeshNode_ctor_DXT1(pvVar2,puVar3);
    }
    else {
      if (iVar1 != 0x35545844) goto switchD_0049771c_caseD_20;
      pvVar2 = operator_new(0x10c4);
      *(void **)(unaff_EBP + 8) = pvVar2;
      *(undefined4 *)(unaff_EBP + -4) = 0x24;
      if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
      piVar4 = MeshNode_ctor_DXT5(pvVar2,puVar3);
    }
  }
  else if (iVar1 == 0x36314c41) {
    pvVar2 = operator_new(0x106c);
    *(void **)(unaff_EBP + 8) = pvVar2;
    *(undefined4 *)(unaff_EBP + -4) = 0x1b;
    if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
    piVar4 = MeshTexture_InitCubeMap(pvVar2,puVar3);
  }
  else if (iVar1 == 0x36315220) {
    pvVar2 = operator_new(0x106c);
    *(void **)(unaff_EBP + 8) = pvVar2;
    *(undefined4 *)(unaff_EBP + -4) = 0x1c;
    if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
    piVar4 = MeshNode_ctor_48(pvVar2,puVar3);
  }
  else if (iVar1 == 0x36315241) {
    pvVar2 = operator_new(0x106c);
    *(void **)(unaff_EBP + 8) = pvVar2;
    *(undefined4 *)(unaff_EBP + -4) = 0x1d;
    if (pvVar2 == (void *)0x0) goto LAB_00497dd1;
    piVar4 = MeshNode_ctor_64(pvVar2,puVar3);
  }
  else {
    if (iVar1 != 0x59565955) goto switchD_0049771c_caseD_20;
    puVar3 = operator_new(0x109c);
    *(undefined4 **)(unaff_EBP + 8) = puVar3;
    *(undefined4 *)(unaff_EBP + -4) = 0x1e;
    if (puVar3 == (undefined4 *)0x0) goto LAB_00497dd1;
    piVar4 = MeshTexture_Init2D_A(puVar3);
  }
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  if ((piVar4 != (int *)0x0) && (piVar4[4] != 0)) {
    (**(code **)(*piVar4 + 0xc))();
  }
switchD_0049771c_caseD_20:
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return piVar4;
}

