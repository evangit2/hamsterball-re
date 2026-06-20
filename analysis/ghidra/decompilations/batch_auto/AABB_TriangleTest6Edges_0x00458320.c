
bool AABB_TriangleTest6Edges(float *param_1,float *param_2,float *param_3)

{
  undefined4 uVar1;
  undefined4 local_10;
  undefined4 uStack_c;
  undefined4 uStack_8;
  
  D3DX_DispatchThunk_71C4();
  D3DX_DispatchThunk_71C4();
  uVar1 = AABB_TriangleIntersect(param_1,param_2);
  if ((char)uVar1 == '\0') {
    uVar1 = AABB_TriangleIntersect(param_2,param_3);
    if ((char)uVar1 == '\0') {
      uVar1 = AABB_TriangleIntersect(param_3,param_1);
      if ((char)uVar1 == '\0') {
        uVar1 = AABB_TriangleIntersect(local_10,uStack_c);
        if ((char)uVar1 == '\0') {
          uVar1 = AABB_TriangleIntersect(uStack_c,uStack_8);
          if ((char)uVar1 == '\0') {
            uVar1 = AABB_TriangleIntersect(uStack_8,local_10);
            return (char)uVar1 != '\0';
          }
        }
      }
    }
  }
  return true;
}

