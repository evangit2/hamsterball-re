
int D3DXMesh_DrawSubset(void *param_1,uint *param_2,uint *param_3,uint *param_4,uint *param_5,
                       undefined4 *param_6)

{
  int iVar1;
  
  if (((((uint)param_2 & 0xe0ffefff) == 0) && ((*(uint *)((int)param_1 + 8) & 0x400) == 0)) &&
     ((*(uint *)((int)param_1 + 8) & 0x40) == 0)) {
    iVar1 = Graphics_DrawIndexedPrimitive(param_1,(int)param_1 + 0x40,0);
    if (-1 < iVar1) {
      if ((*(byte *)((int)param_1 + 0xc) & 0x20) != 0) {
        param_2 = (uint *)((uint)param_2 | 0x10000000);
      }
      iVar1 = D3DXMesh_OptimizeInternal
                        (param_1,param_2,param_3,param_4,param_5,param_6,(int *)0x0,
                         *(uint **)((int)param_1 + 4));
    }
  }
  else {
    iVar1 = -0x7789f794;
  }
  if (*(int *)((int)param_1 + 0x40) != 0) {
    (**(code **)(**(int **)((int)param_1 + 0x3c) + 0x30))(*(int **)((int)param_1 + 0x3c));
    *(undefined4 *)((int)param_1 + 0x40) = 0;
  }
  return iVar1;
}

