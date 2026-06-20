
void __thiscall Mesh_AddVertex(void *this,float *param_1)

{
  float *pfVar1;
  int iVar2;
  int iVar3;
  undefined4 local_28 [6];
  void *local_10;
  void *pvStack_c;
  undefined1 *local_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  local_8 = &LAB_004cd7e8;
  pvStack_c = ExceptionList;
  if (*(char *)((int)this + 4) == '\0') {
    ExceptionList = &pvStack_c;
    SpriteAnim_InitFromCalcTexCoords(local_28);
    iVar2 = *(int *)((int)this + 0x24);
    local_4 = 0;
    if (iVar2 < *(int *)((int)this + 0x1c)) {
      pfVar1 = (float *)(iVar2 * 0x20 + *(int *)((int)this + 0xc));
      do {
        if (((((*param_1 == *pfVar1) && (param_1[1] == pfVar1[1])) && (param_1[2] == pfVar1[2])) &&
            ((param_1[3] == pfVar1[3] && (param_1[4] == pfVar1[4])))) &&
           ((param_1[5] == pfVar1[5] && ((param_1[6] == pfVar1[6] && (param_1[7] == pfVar1[7]))))))
        {
          if (iVar2 != -1) goto LAB_0046fef3;
          break;
        }
        iVar2 = iVar2 + 1;
        pfVar1 = pfVar1 + 8;
      } while (iVar2 < *(int *)((int)this + 0x1c));
    }
    *(float *)(*(int *)((int)this + 0xc) + *(int *)((int)this + 0x1c) * 0x20) = *param_1;
    *(float *)(*(int *)((int)this + 0x1c) * 0x20 + 4 + *(int *)((int)this + 0xc)) = param_1[1];
    *(float *)(*(int *)((int)this + 0x1c) * 0x20 + 8 + *(int *)((int)this + 0xc)) = param_1[2];
    *(float *)(*(int *)((int)this + 0x1c) * 0x20 + 0xc + *(int *)((int)this + 0xc)) = param_1[3];
    *(float *)(*(int *)((int)this + 0x1c) * 0x20 + 0x10 + *(int *)((int)this + 0xc)) = param_1[4];
    *(float *)(*(int *)((int)this + 0x1c) * 0x20 + 0x14 + *(int *)((int)this + 0xc)) = param_1[5];
    *(float *)(*(int *)((int)this + 0x1c) * 0x20 + 0x18 + *(int *)((int)this + 0xc)) = param_1[6];
    *(float *)(*(int *)((int)this + 0x1c) * 0x20 + 0x1c + *(int *)((int)this + 0xc)) = param_1[7];
    iVar2 = *(int *)((int)this + 0x1c);
    CopyVec2((void *)(iVar2 * 0x1c + *(int *)((int)this + 0x14)),(int)local_28);
    iVar3 = *(int *)((int)this + 0x1c) + 1;
    *(int *)((int)this + 0x1c) = iVar3;
    if (*(int *)((int)this + 8) <= iVar3) {
      MessageBoxA((HWND)0x0,"Too many vertices","Oops",0);
      CRT_FlsAlloc(0);
    }
LAB_0046fef3:
    *(undefined4 *)(*(int *)((int)this + 0x18) + (*(int *)((int)this + 0x20) / 3) * 4) =
         *(undefined4 *)((int)this + 0x28);
    *(short *)(*(int *)((int)this + 0x10) + *(int *)((int)this + 0x20) * 2) = (short)iVar2;
    *(int *)((int)this + 0x20) = *(int *)((int)this + 0x20) + 1;
    local_8 = (undefined1 *)0xffffffff;
    StreamReaderVtbl_Init((undefined4 *)&stack0xffffffd4);
  }
  ExceptionList = local_10;
  return;
}

