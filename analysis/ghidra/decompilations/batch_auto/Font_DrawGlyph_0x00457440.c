
void __thiscall
Font_DrawGlyph(void *this,byte *param_1,int param_2,int param_3,undefined4 param_4,
              undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8)

{
  byte *pbVar1;
  byte bVar2;
  float fVar3;
  float fVar4;
  void **ppvVar5;
  byte *pbVar6;
  uint uVar7;
  int iVar8;
  byte *pbVar9;
  ulonglong uVar10;
  void *local_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_004ccd50;
  local_c = ExceptionList;
  local_4 = 0;
  ppvVar5 = &local_c;
  if (param_1 != (byte *)0x0) {
    bVar2 = *param_1;
    pbVar9 = param_1;
    pbVar6 = (byte *)param_2;
    iVar8 = param_3;
    param_1 = (byte *)param_2;
    ExceptionList = &local_c;
    while (ppvVar5 = ExceptionList, bVar2 != 0) {
      bVar2 = *pbVar9;
      if (bVar2 == 10) {
        iVar8 = iVar8 + *(int *)((int)this + 0x424);
        pbVar6 = (byte *)param_2;
        param_1 = (byte *)param_2;
        param_3 = iVar8;
      }
      else {
        uVar7 = (uint)bVar2;
        if (*(char *)((int)this + uVar7 * 0x14 + 0x42c) == '\0') {
          if (bVar2 != 0x20) goto LAB_0045764f;
        }
        else if (bVar2 != 0x20) {
          if (*(int *)((int)this + 0x428) == 0x3f800000) {
            uVar7 = (uint)*pbVar9;
            Sprite_DrawRect(*(void **)((int)this + uVar7 * 0x14 + 0x43c),
                            (float)(int)(pbVar6 + *(int *)((int)this + uVar7 * 0x14 + 0x434)),
                            (float)(*(int *)((int)this + (uVar7 * 5 + 0x10e) * 4) + iVar8));
          }
          else {
            fVar3 = (float)*(int *)((int)this + (uVar7 * 5 + 0x10e) * 4) *
                    *(float *)((int)this + 0x428);
            fVar4 = (float)*(int *)((int)this + uVar7 * 0x14 + 0x434) *
                    *(float *)((int)this + 0x428);
            local_4._1_3_ = (uint3)((uint)local_4 >> 8);
            local_4._0_1_ = 1;
            Scene_CreateObject4f
                      (*(void **)((int)this + (uint)*pbVar9 * 0x14 + 0x43c),&PTR_LAB_004cf584,
                       (float)(int)param_1 + fVar4,(float)param_3 + fVar3,
                       ((float)*(int *)((int)this + uVar7 * 0x14 + 0x430) - fVar4) *
                       *(float *)((int)this + 0x428),
                       ((float)*(int *)((int)this + 0x424) - fVar3) * *(float *)((int)this + 0x428),
                       &PTR_Vec3_dtor_004cf300,param_5,param_6,param_7,param_8);
            local_4 = (uint)local_4._1_3_ << 8;
          }
        }
        uVar10 = __ftol2();
        pbVar6 = (byte *)uVar10;
        param_1 = pbVar6;
      }
LAB_0045764f:
      pbVar1 = pbVar9 + 1;
      pbVar9 = pbVar9 + 1;
      bVar2 = *pbVar1;
    }
  }
  ExceptionList = ppvVar5;
  local_4 = 0xffffffff;
  Matrix_Identity(&param_4);
  ExceptionList = local_c;
  return;
}

