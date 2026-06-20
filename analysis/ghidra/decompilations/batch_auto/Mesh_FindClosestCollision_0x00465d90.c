
/* WARNING: Function: __chkstk replaced with injection: alloca_probe */

undefined4 * __thiscall Mesh_FindClosestCollision(void *this,undefined4 *param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  undefined4 in_stack_00000020;
  undefined4 local_110c;
  undefined4 local_1108;
  undefined4 local_1104;
  undefined4 local_1100 [6];
  undefined1 local_10e7;
  float local_10e0 [3];
  undefined4 local_10d4 [262];
  undefined4 local_cbc [5];
  undefined1 local_ca8;
  undefined4 local_44;
  undefined4 uStack_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  int local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd241;
  pvStack_c = ExceptionList;
  uStack_10 = 0x465daf;
  ExceptionList = &pvStack_c;
  AthenaList_Init(local_10d4,0);
  local_4 = 0;
  AthenaList_Append(local_10d4,(int)this);
  CollisionMesh_ctor(local_cbc,0);
  local_4._0_1_ = 1;
  local_ca8 = 0;
  SpatialTree_ctor(local_1100,local_10d4);
  local_4._0_1_ = 2;
  local_10e7 = 1;
  CollisionMesh_AddTriangle(local_cbc,(int)local_1100);
  local_44 = in_stack_00000020;
  Vec3_NormalizeAndScale(&stack0x00000014,99999.0);
  local_110c = in_stack_00000020;
  local_1108 = in_stack_00000020;
  local_1104 = in_stack_00000020;
  puVar3 = (undefined4 *)
           Ball_AdvancePositionOrCollision
                     (local_cbc,local_10e0,(float *)&stack0x00000008,(float *)&stack0x00000014,
                      &local_110c,0.01);
  uVar1 = puVar3[2];
  uVar2 = puVar3[1];
  *param_1 = *puVar3;
  local_4._0_1_ = 1;
  param_1[1] = uVar2;
  param_1[2] = uVar1;
  SpatialTree_Free(local_1100);
  local_4 = (uint)local_4._1_3_ << 8;
  Mesh_Clear(local_cbc);
  local_4 = 0xffffffff;
  Vec3List_Free(local_10d4);
  ExceptionList = pvStack_c;
  return param_1;
}

