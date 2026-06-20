/*
 * Function: Ball_TestPlaneIntersection
 * Address: 0x00402810
 * Signature: short __thiscall
Ball_TestPlaneIntersection(void *this,float param_1,float param_2,float param_3,float param_4)
 *
 * Patterns: ball. Calls: Ball_TestPlaneIntersection, NAN, CONCAT11. Offsets: 1, Lines: 24
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

short __thiscall
Ball_TestPlaneIntersection(void *this,float param_1,float param_2,float param_3,float param_4)

{
  float fVar1;
  float fVar2;
  byte bVar3;
  float *pfVar4;
  int iVar5;
  
  iVar5 = 0;
  fVar1 = -param_4;
  pfVar4 = (float *)((int)this + 0xc);
  do {
    fVar2 = param_2 * *pfVar4 + param_3 * pfVar4[1] + param_1 * pfVar4[-1] + pfVar4[2];
    bVar3 = fVar2 < fVar1 | (byte)((ushort)((ushort)(NAN(fVar2) || NAN(fVar1)) << 10) >> 8) |
            (byte)((ushort)((ushort)(fVar2 == fVar1) << 0xe) >> 8);
    if (fVar2 < fVar1) {
      return (ushort)bVar3 << 8;
    }
    iVar5 = iVar5 + 1;
    pfVar4 = pfVar4 + 4;
  } while (iVar5 < 6);
  return CONCAT11(bVar3,1);
}
