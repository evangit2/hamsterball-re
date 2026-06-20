
ushort __thiscall HitBox_PointInBounds(void *this,int param_1,int param_2)

{
  float fVar1;
  float fVar2;
  float fVar3;
  ushort uVar4;
  
  fVar2 = (float)param_2;
  fVar3 = (float)param_1;
  fVar1 = *(float *)((int)this + 4);
  uVar4 = (ushort)(fVar3 < fVar1) << 8 | (ushort)(NAN(fVar3) || NAN(fVar1)) << 10 |
          (ushort)(fVar3 == fVar1) << 0xe;
  if (fVar3 >= fVar1) {
    fVar1 = *(float *)((int)this + 8);
    uVar4 = (ushort)(fVar2 < fVar1) << 8 | (ushort)(NAN(fVar2) || NAN(fVar1)) << 10 |
            (ushort)(fVar2 == fVar1) << 0xe;
    if (fVar2 >= fVar1) {
      fVar1 = *(float *)((int)this + 0xc) + *(float *)((int)this + 4);
      uVar4 = (ushort)(fVar3 < fVar1) << 8 | (ushort)(NAN(fVar3) || NAN(fVar1)) << 10 |
              (ushort)(fVar3 == fVar1) << 0xe;
      if (fVar3 < fVar1) {
        fVar1 = *(float *)((int)this + 0x10) + *(float *)((int)this + 8);
        uVar4 = (ushort)(fVar1 < fVar2) << 8 | (ushort)(NAN(fVar1) || NAN(fVar2)) << 10 |
                (ushort)(fVar1 == fVar2) << 0xe;
        if (fVar1 >= fVar2 && (fVar1 == fVar2) == 0) {
          return CONCAT11((char)(uVar4 >> 8),1);
        }
      }
    }
  }
  return uVar4;
}

