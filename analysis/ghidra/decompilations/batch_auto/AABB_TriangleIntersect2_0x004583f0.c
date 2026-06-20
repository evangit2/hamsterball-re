
bool AABB_TriangleIntersect2(float *param_1,float *param_2,float *param_3)

{
  bool bVar1;
  
  bVar1 = AABB_TriangleTest6Edges(param_1,param_2,param_3);
  if (bVar1) {
    return true;
  }
  bVar1 = AABB_TriangleTest6Edges(param_1,param_2,param_3);
  return bVar1;
}

