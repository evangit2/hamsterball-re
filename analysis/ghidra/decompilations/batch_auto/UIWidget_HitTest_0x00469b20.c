
uint __thiscall UIWidget_HitTest(void *this,int param_1,int param_2)

{
  void *this_00;
  ushort uVar1;
  int iVar2;
  uint uVar3;
  
  this_00 = *(void **)((int)this + 0x420);
  if (this_00 != (void *)0x0) {
    uVar1 = Rect_ContainsPoint(this_00,(float)param_1,(float)param_2);
    return -(uint)((char)uVar1 != '\0') & (uint)this_00;
  }
  iVar2 = AthenaList_GetSize((int)this + 4);
  iVar2 = iVar2 + -1;
  if (-1 < iVar2) {
    do {
      if (iVar2 < *(int *)((int)this + 8)) {
        uVar3 = *(uint *)(*(int *)((int)this + 0x410) + iVar2 * 4);
      }
      else {
        uVar3 = 0;
      }
      if (*(float *)(uVar3 + 4) <= (float)param_1) {
        if (*(float *)(uVar3 + 8) <= (float)param_2) {
          if ((float)param_1 < *(float *)(uVar3 + 0xc) + *(float *)(uVar3 + 4)) {
            if ((float)param_2 < *(float *)(uVar3 + 0x10) + *(float *)(uVar3 + 8)) {
              return uVar3;
            }
          }
        }
      }
      iVar2 = iVar2 + -1;
    } while (-1 < iVar2);
  }
  return 0;
}

