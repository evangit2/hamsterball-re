
int __thiscall D3DX_InitSurfaceLoader(void *param_1,int param_2,undefined4 param_3,uint param_4)

{
  int *piVar1;
  int iVar2;
  int unaff_EBX;
  
  *(undefined4 *)((int)param_1 + 4) = 0;
  *(undefined4 *)param_1 = 0;
  *(uint *)((int)param_1 + 8) = param_4;
  if ((((param_4 & 0xffff) != 0) && ((param_4 & 0xffff) < 6)) && ((param_4 & 0xfff00000) == 0)) {
    *(uint *)(param_2 + 0x40) = param_4 & 0x80000;
    piVar1 = D3DX_CreateMeshFromFormat();
    *(int **)((int)param_1 + 4) = piVar1;
    if (piVar1 != (int *)0x0) {
      piVar1 = D3DX_CreateMeshFromFormat();
      *(int **)param_1 = piVar1;
      if (((((piVar1 != (int *)0x0) && (iVar2 = D3DX_AllocVertexProcessBuffer(), -1 < iVar2)) &&
           ((iVar2 = DDSurface_CopyRects(param_1), iVar2 < 0 &&
            ((iVar2 = DDSurface_Blt3PointFilter(), iVar2 < 0 &&
             (iVar2 = D3DX_CopyRects_Point(), iVar2 < 0)))))) &&
          (iVar2 = D3DX_CopyRects_Stretch(), iVar2 < 0)) &&
         ((((iVar2 = DDSurface_Blt4PointFilter(), iVar2 < 0 &&
            (iVar2 = DDSurface_Blt5PointFilter(), iVar2 < 0)) &&
           (iVar2 = D3DX_TransformTex_Bilinear(), iVar2 < 0)) &&
          ((iVar2 = DDSurface_Blt3PointWBuffer(), iVar2 < 0 &&
           (iVar2 = SDFGrid_MultiplySparse(), iVar2 < 0)))))) {
        D3DX_SkinMesh_ProcessWeights();
      }
    }
    if (*(undefined4 **)((int)param_1 + 4) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)((int)param_1 + 4))(1);
      *(undefined4 *)((int)param_1 + 4) = 0;
    }
    if (*(undefined4 **)param_1 != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)param_1)(1);
      *(undefined4 *)param_1 = 0;
    }
  }
  return unaff_EBX;
}

