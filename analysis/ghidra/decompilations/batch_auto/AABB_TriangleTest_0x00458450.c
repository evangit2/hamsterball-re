
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool AABB_TriangleTest(float *param_1,float *param_2)

{
  bool bVar1;
  int iVar2;
  float local_84;
  float local_80;
  float local_7c;
  float local_78;
  float local_74;
  float local_70;
  float local_6c;
  float local_68;
  float local_64;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  float local_40;
  float local_3c;
  float local_38;
  float local_34;
  float local_30;
  float local_2c;
  float local_28;
  float local_24;
  float local_20;
  float local_1c;
  float local_18;
  float local_14;
  float local_10;
  float local_c;
  float local_8;
  float local_4;
  
  local_30 = *param_2 - _DAT_004cf310;
  local_2c = param_2[1] - _DAT_004cf310;
  local_28 = param_2[2] - _DAT_004cf310;
  local_24 = param_2[3] + _DAT_004cf310;
  local_14 = param_2[4] + _DAT_004cf310;
  local_4 = param_2[5] + _DAT_004cf310;
  local_3c = *param_1;
  local_48 = param_1[8];
  local_54 = param_1[0x10];
  local_38 = param_1[1];
  local_34 = param_1[2];
  local_44 = param_1[9];
  local_40 = param_1[10];
  local_50 = param_1[0x11];
  local_4c = param_1[0x12];
  iVar2 = 0;
  local_78 = local_3c;
  local_74 = local_38;
  local_70 = local_34;
  local_6c = local_3c;
  local_68 = local_38;
  local_64 = local_34;
  do {
    local_84 = local_54;
    local_80 = local_50;
    local_7c = local_4c;
    if (iVar2 == 0) {
      local_84 = local_48;
      local_80 = local_44;
      local_7c = local_40;
    }
    if (local_84 < local_78 != (local_84 == local_78)) {
      local_78 = local_84;
    }
    if (local_80 < local_74 != (local_80 == local_74)) {
      local_74 = local_80;
    }
    if (local_7c < local_70 != (local_7c == local_70)) {
      local_70 = local_7c;
    }
    if (local_6c <= local_84) {
      local_6c = local_84;
    }
    if (local_68 <= local_80) {
      local_68 = local_80;
    }
    if (local_64 <= local_7c) {
      local_64 = local_7c;
    }
    iVar2 = iVar2 + 1;
  } while (iVar2 < 2);
  if ((((local_78 < local_30) || (local_6c < local_24 == (local_6c == local_24))) ||
      (local_74 < local_2c)) ||
     (((local_68 < local_14 == (local_68 == local_14) || (local_70 < local_28)) ||
      (local_64 < local_4 == (local_64 == local_4))))) {
    if ((((local_24 < local_78) && (local_24 < local_6c)) ||
        ((local_78 < local_30 && (local_6c < local_30)))) ||
       (((local_14 < local_74 && (local_14 < local_68)) ||
        (((local_74 < local_2c && (local_68 < local_2c)) ||
         (((local_4 < local_70 && (local_4 < local_64)) ||
          ((local_70 < local_28 && (local_64 < local_28)))))))))) {
      return false;
    }
    local_20 = local_2c;
    local_1c = local_28;
    local_18 = local_30;
    local_10 = local_28;
    local_c = local_24;
    local_8 = local_2c;
    bVar1 = AABB_TriangleTest6Edges(&local_3c,&local_48,&local_54);
    if (((((!bVar1) && (bVar1 = AABB_TriangleTest6Edges(&local_3c,&local_48,&local_54), !bVar1)) &&
         (bVar1 = AABB_TriangleTest6Edges(&local_3c,&local_48,&local_54), !bVar1)) &&
        ((bVar1 = AABB_TriangleTest6Edges(&local_3c,&local_48,&local_54), !bVar1 &&
         (bVar1 = AABB_TriangleIntersect2(&local_3c,&local_48,&local_54), !bVar1)))) &&
       ((bVar1 = AABB_TriangleIntersect2(&local_3c,&local_48,&local_54), !bVar1 &&
        (bVar1 = AABB_TriangleIntersect2(&local_3c,&local_48,&local_54), !bVar1)))) {
      bVar1 = AABB_TriangleIntersect2(&local_3c,&local_48,&local_54);
      return bVar1;
    }
  }
  return true;
}

