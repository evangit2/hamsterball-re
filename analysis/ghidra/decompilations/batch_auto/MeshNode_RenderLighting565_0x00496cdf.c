
void __thiscall MeshNode_RenderLighting565(void *this,int param_1,int param_2,undefined4 *param_3)

{
  int iVar1;
  uint uVar2;
  undefined4 *puVar3;
  
  if (*(int *)((int)this + 0x1048) != 0) {
    param_3 = (undefined4 *)Vertex_Transform(this,(int)param_3);
  }
  iVar1 = D3DXMesh_ComputeLighting565
                    (this,param_1 + *(int *)((int)this + 0x1034),
                     param_2 + *(int *)((int)this + 0x1040),
                     (uint)(*(int *)((int)this + 0x1088) != *(int *)((int)this + 0x1058)));
  if (-1 < iVar1) {
    puVar3 = (undefined4 *)
             ((*(int *)((int)this + 0x1030) - *(int *)((int)this + 0x1070)) * 0x10 +
             *(int *)((int)this + 0x106c));
    for (uVar2 = (uint)(*(int *)((int)this + 0x1058) << 4) >> 2; uVar2 != 0; uVar2 = uVar2 - 1) {
      *puVar3 = *param_3;
      param_3 = param_3 + 1;
      puVar3 = puVar3 + 1;
    }
    for (iVar1 = 0; iVar1 != 0; iVar1 = iVar1 + -1) {
      *(undefined1 *)puVar3 = *(undefined1 *)param_3;
      param_3 = (undefined4 *)((int)param_3 + 1);
      puVar3 = (undefined4 *)((int)puVar3 + 1);
    }
    *(undefined4 *)((int)this + 0x108c) = 1;
  }
  return;
}

