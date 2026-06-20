
void __fastcall D3DX_SurfaceClipBlit(int param_1)

{
  int iVar1;
  int *piVar2;
  int *piVar3;
  int *in_EAX;
  int iVar4;
  
  if (in_EAX != (int *)0x0) {
    while (in_EAX[2] <= param_1) {
      iVar1 = *in_EAX;
      piVar2 = (int *)in_EAX[3];
      param_1 = param_1 - in_EAX[2];
      piVar3 = *(int **)(iVar1 + 0xc);
      iVar4 = *(int *)(iVar1 + 8) + -1;
      *(int *)(iVar1 + 8) = iVar4;
      if (iVar4 == 0) {
        piVar3[2] = piVar3[2] + -1;
        *(int *)(iVar1 + 0xc) = *piVar3;
        *piVar3 = iVar1;
      }
      piVar3[2] = piVar3[2] + -1;
      in_EAX[3] = piVar3[1];
      piVar3[1] = (int)in_EAX;
      AthenaList_FreeAllChunks(piVar3);
      in_EAX = piVar2;
      if (piVar2 == (int *)0x0) {
        return;
      }
    }
    in_EAX[1] = in_EAX[1] + param_1;
    in_EAX[2] = in_EAX[2] - param_1;
  }
  return;
}

