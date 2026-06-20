
void __cdecl D3DX_InitMathDispatchTable(undefined4 *param_1)

{
  ulonglong uVar1;
  
  uVar1 = CPUID_DetectFeatures();
  if (((uVar1 & 0x20) != 0) && ((char)uVar1 < '\0')) {
    *param_1 = &LAB_004a1d48;
    param_1[1] = &LAB_004a128a;
    param_1[2] = &LAB_004a2009;
    param_1[3] = &LAB_0049d4da;
    param_1[5] = &LAB_004a1d9d;
    param_1[6] = &LAB_004a12fb;
    param_1[7] = Vec2_NormalizeSSE2;
    param_1[8] = &LAB_004a208b;
    param_1[9] = &LAB_004a1dd6;
    param_1[10] = &LAB_004a1842;
    param_1[0xb] = &LAB_0049d70c;
    param_1[0xc] = &LAB_004a2433;
    param_1[0xd] = &LAB_004a2476;
    param_1[0xe] = Vec2_ProjectMatrix;
    param_1[0xf] = Vec2_ProjectMatrixInverse;
    param_1[0x10] = &LAB_004a20ea;
    param_1[0x11] = &LAB_004a1e3a;
    param_1[0x12] = &LAB_004a126e;
    param_1[0x13] = &LAB_0049bf70;
    param_1[0x14] = &LAB_0049c009;
    param_1[0x15] = Vec2_Normalize;
    param_1[0x16] = &LAB_004a255a;
    param_1[0x17] = Vec2_NormalizeSSE;
    param_1[0x18] = &LAB_0049c119;
    param_1[0x19] = &LAB_0049c54a;
    param_1[0x1a] = &LAB_0049d7db;
    param_1[0x1b] = &LAB_0049d82b;
    param_1[0x1c] = &LAB_0049d916;
    param_1[0x1d] = Vec2_RotateByAngleSSE;
    param_1[0x1e] = &LAB_0049d9f2;
    param_1[0x1f] = &LAB_0049da55;
    param_1[0x20] = &LAB_0049dab3;
    param_1[0x21] = &LAB_0049db0c;
    param_1[0x22] = &LAB_004a267c;
    param_1[0x23] = SSE2_QuaternionSLERP;
    param_1[0x24] = CRT_InitThreadLocalStorage;
    param_1[0x25] = &LAB_0049db53;
    param_1[0x26] = &LAB_0049dbfb;
    param_1[0x27] = Matrix_BuildFromQuatScaleTranslate;
    param_1[0x28] = Matrix_BuildFromTanAngles;
    param_1[0x29] = &LAB_0049f0bf;
    param_1[0x2a] = Matrix_BuildFromNormTanAngles;
    param_1[0x2b] = CRT_InitSecurityCookie;
    param_1[0x2c] = &LAB_0049c8d1;
    param_1[0x36] = &LAB_004a221c;
    param_1[0x33] = &LAB_004a13c1;
    param_1[0x30] = &LAB_004a1ed2;
    param_1[0x2e] = &LAB_004a1f1b;
    param_1[0x34] = &LAB_004a228d;
    param_1[0x31] = &LAB_004a1432;
    param_1[0x37] = CRT_TerminateProcess;
    param_1[0x38] = &LAB_0049c9dc;
    param_1[0x35] = &LAB_004a2355;
    param_1[0x2f] = &LAB_004a1f5e;
    param_1[0x32] = &LAB_004a1506;
    param_1[4] = &LAB_004a0b42;
    param_1[0x2d] = Matrix_RotateEllipseIntersectionSSE;
    if (((uVar1 & 0x100) != 0) && ((uVar1 & 0x200) != 0)) {
      param_1[0xf] = Vec2_ProjectMatrixQuaternion;
      param_1[0x12] = Matrix_BuildFromQuaternion3x3;
      param_1[0x26] = &LAB_0049dccd;
      param_1[0x27] = CRT_RaiseException;
      param_1[0x18] = &LAB_0049c335;
    }
  }
  return;
}

