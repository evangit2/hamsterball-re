
int __thiscall D3DTexture_CopySurfaceData(void *this,uint param_1,int *param_2)

{
  uint uVar1;
  uint uVar2;
  uint *puVar3;
  uint *puVar4;
  undefined1 local_134 [4];
  int local_130;
  int local_d0;
  int local_10;
  uint *local_c;
  uint *local_8;
  
  local_c = (uint *)0x0;
  local_8 = (uint *)0x0;
  local_10 = (**(code **)(*param_2 + 0x2c))(param_2,0,0,&local_c,0x800);
  if ((-1 < local_10) &&
     (local_10 = Graphics_DrawIndexedPrimitiveUP(this,&local_8,0x10), -1 < local_10)) {
    if (param_1 == *(uint *)((int)this + 4)) {
      uVar1 = *(int *)((int)this + 0x30) * *(int *)((int)this + 0x2c);
      puVar3 = local_8;
      puVar4 = local_c;
      for (uVar2 = uVar1 >> 2; uVar2 != 0; uVar2 = uVar2 - 1) {
        *puVar4 = *puVar3;
        puVar3 = puVar3 + 1;
        puVar4 = puVar4 + 1;
      }
      for (uVar1 = uVar1 & 3; uVar1 != 0; uVar1 = uVar1 - 1) {
        *(char *)puVar4 = (char)*puVar3;
        puVar3 = (uint *)((int)puVar3 + 1);
        puVar4 = (uint *)((int)puVar4 + 1);
      }
    }
    else {
      D3D_InitDisplayModes(local_134,*(uint *)((int)this + 4),param_1);
      param_1 = 0;
      puVar3 = local_8;
      puVar4 = local_c;
      if (*(int *)((int)this + 0x30) != 0) {
        do {
          VertexDecl_CopyVertexData(local_134,puVar3,puVar4);
          puVar3 = (uint *)((int)puVar3 + local_130);
          puVar4 = (uint *)((int)puVar4 + local_d0);
          param_1 = param_1 + 1;
        } while (param_1 < *(uint *)((int)this + 0x30));
      }
    }
  }
  if (local_c != (uint *)0x0) {
    (**(code **)(*param_2 + 0x30))(param_2);
  }
  if (local_8 != (uint *)0x0) {
    (**(code **)(**(int **)((int)this + 0x28) + 0x30))(*(int **)((int)this + 0x28));
  }
  return local_10;
}

