
undefined4 __thiscall
MeshStrip_GenerateNextStrip(void *this,int param_1,int *param_2,int *param_3,int param_4)

{
  int *piVar1;
  int *_Memory;
  int iVar2;
  uint uVar3;
  int *piVar4;
  int iVar5;
  int iVar6;
  
  piVar1 = param_3;
  _Memory = operator_new(*(int *)((int)this + 0x18) << 2);
  if ((char)param_4 == '\0') {
    *param_2 = 3;
    iVar5 = param_1 * 0x24;
    *param_3 = *(int *)(iVar5 + *(int *)((int)this + 0x28));
    param_3[1] = *(int *)(*(int *)((int)this + 0x28) + 4 + iVar5);
    param_3[2] = *(int *)(*(int *)((int)this + 0x28) + 8 + iVar5);
    *(undefined4 *)(*(int *)((int)this + 0x28) + 0x1c + iVar5) = 1;
LAB_0047d009:
    _free(_Memory);
    return 0;
  }
  iVar6 = -1;
  iVar2 = 0;
  iVar5 = 4;
  if (0 < *(int *)((int)this + 0x24)) {
    piVar4 = (int *)(*(int *)((int)this + 0x28) + 0xc);
    do {
      if ((piVar4[4] == 0) && (*piVar4 < iVar5)) {
        iVar5 = *piVar4;
        iVar6 = iVar2;
      }
      iVar2 = iVar2 + 1;
      piVar4 = piVar4 + 9;
    } while (iVar2 < *(int *)((int)this + 0x24));
    if (iVar6 != -1) {
      iVar5 = iVar6 * 0x24;
      *(undefined4 *)(iVar5 + 0x1c + *(int *)((int)this + 0x28)) = 1;
      *param_2 = 0;
      uVar3 = MeshStrip_ComputeWinding(this,_Memory,param_2,iVar6,&param_4);
      if (uVar3 == 0xffffffff) {
        _free(_Memory);
        return 0xffffffff;
      }
      iVar2 = 0;
      if (0 < *param_2) {
        do {
          param_3[iVar2] = _Memory[(*param_2 - iVar2) + -1];
          iVar2 = iVar2 + 1;
        } while (iVar2 < *param_2);
      }
      if (uVar3 == 1) {
        if (param_4 == iVar6) {
          *param_3 = *(int *)(iVar5 + *(int *)((int)this + 0x28));
          param_3[1] = *(int *)(*(int *)((int)this + 0x28) + 4 + iVar5);
          param_3[2] = *(int *)(*(int *)((int)this + 0x28) + 8 + iVar5);
          goto LAB_0047d009;
        }
        iVar5 = 1;
        if (1 < *param_2) {
          do {
            *param_3 = _Memory[(*param_2 - iVar5) + -1];
            param_3 = param_3 + 1;
            iVar5 = iVar5 + 1;
          } while (iVar5 < *param_2);
        }
        *param_2 = *param_2 + -1;
        *(undefined4 *)(*(int *)((int)this + 0x28) + 0x1c + param_4 * 0x24) = 0;
      }
      uVar3 = MeshStrip_ComputeWinding(this,piVar1,param_2,iVar6,&param_4);
      if (uVar3 == 0xffffffff) {
        _free(_Memory);
        return 0xffffffff;
      }
      goto LAB_0047d009;
    }
  }
  _free(_Memory);
  return 0xffffffff;
}

