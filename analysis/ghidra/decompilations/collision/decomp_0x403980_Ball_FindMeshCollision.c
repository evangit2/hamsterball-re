/* Ghidra Decompilation
 * Function: Ball_FindMeshCollision
 * Address: 0x403980
 * Decompiled: 2026-06-19
 */


undefined4 * __thiscall
Ball_FindMeshCollision
          (void *this,undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
          undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8)

{
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 uStack_8;
  
  uStack_8 = param_8;
  if (&local_14 != &param_5) {
    local_14 = param_5;
    local_10 = param_6;
    local_c = param_7;
  }
  if (&local_20 != &param_2) {
    local_20 = param_2;
    local_1c = param_3;
    local_18 = param_4;
  }
  Mesh_FindClosestCollision(this,param_1);
  return param_1;
}

