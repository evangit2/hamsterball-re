
undefined4 AABB_TriangleIntersect(float *param_1,float *param_2)

{
  float fVar1;
  uint uVar2;
  float fVar3;
  undefined4 uVar4;
  float fVar6;
  float fVar7;
  float unaff_ESI;
  float fStack_34;
  float fStack_24;
  float fStack_20;
  float local_18;
  float fStack_14;
  undefined2 uVar5;
  
  uVar2 = D3DX_ShaderDispatch_noarg5();
  uVar4 = 0;
  if (uVar2 != 0) {
    fVar3 = (float)(uVar2 & 0xffff0000);
    if (*param_1 <= *param_2) {
      fVar3 = *param_2;
      fVar1 = *param_1;
      fStack_34 = fVar3;
    }
    else {
      fVar1 = *param_2;
      fStack_34 = *param_1;
    }
    fVar3 = (float)((uint)fVar3 & 0xffff0000);
    if (param_1[1] <= param_2[1]) {
      fVar3 = param_1[1];
      fVar6 = param_2[1];
      local_18 = fVar3;
    }
    else {
      fVar6 = param_1[1];
      local_18 = param_2[1];
    }
    fVar3 = (float)((uint)fVar3 & 0xffff0000);
    if (param_1[2] <= param_2[2]) {
      fStack_14 = param_1[2];
      fVar7 = param_2[2];
    }
    else {
      fVar3 = param_2[2];
      fVar7 = param_1[2];
      fStack_14 = fVar3;
    }
    uVar5 = (undefined2)((uint)fVar3 >> 0x10);
    uVar4 = CONCAT22(uVar5,(ushort)(unaff_ESI < fVar1) << 8 |
                           (ushort)(NAN(unaff_ESI) || NAN(fVar1)) << 10 |
                           (ushort)(unaff_ESI == fVar1) << 0xe);
    if (unaff_ESI >= fVar1) {
      uVar4 = CONCAT22(uVar5,(ushort)(unaff_ESI < fStack_34) << 8 |
                             (ushort)(NAN(unaff_ESI) || NAN(fStack_34)) << 10 |
                             (ushort)(unaff_ESI == fStack_34) << 0xe);
      if (unaff_ESI < fStack_34 != (unaff_ESI == fStack_34)) {
        uVar4 = CONCAT22(uVar5,(ushort)(fStack_24 < local_18) << 8 |
                               (ushort)(NAN(fStack_24) || NAN(local_18)) << 10 |
                               (ushort)(fStack_24 == local_18) << 0xe);
        if (fStack_24 >= local_18) {
          uVar4 = CONCAT22(uVar5,(ushort)(fStack_24 < fVar6) << 8 |
                                 (ushort)(NAN(fStack_24) || NAN(fVar6)) << 10 |
                                 (ushort)(fStack_24 == fVar6) << 0xe);
          if (fStack_24 < fVar6 != (fStack_24 == fVar6)) {
            uVar4 = CONCAT22(uVar5,(ushort)(fStack_20 < fStack_14) << 8 |
                                   (ushort)(NAN(fStack_20) || NAN(fStack_14)) << 10 |
                                   (ushort)(fStack_20 == fStack_14) << 0xe);
            if (fStack_20 >= fStack_14) {
              uVar4 = CONCAT22(uVar5,(ushort)(fStack_20 < fVar7) << 8 |
                                     (ushort)(NAN(fStack_20) || NAN(fVar7)) << 10 |
                                     (ushort)(fStack_20 == fVar7) << 0xe);
              if (fStack_20 < fVar7 != (fStack_20 == fVar7)) {
                return CONCAT31((int3)((uint)uVar4 >> 8),1);
              }
            }
          }
        }
      }
    }
  }
  return uVar4;
}

