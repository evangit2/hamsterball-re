
void * __thiscall LoadFont(void *this,undefined4 param_1,undefined4 param_2)

{
  void *pvVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  undefined4 *puVar6;
  int iVar7;
  ulonglong uVar8;
  char *pcVar9;
  undefined4 uVar10;
  char *pcVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  float local_240;
  int local_23c;
  int local_238;
  undefined1 local_234 [4];
  undefined1 local_230 [4];
  int local_22c;
  undefined4 local_228;
  undefined1 local_224 [4];
  undefined1 local_220 [4];
  undefined4 local_21c;
  undefined4 local_218;
  void *local_214;
  char local_210 [256];
  char local_110 [260];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ccd0e;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined ***)this = &PTR_FontList_ScalarDtor_004d8e30;
  local_214 = this;
  AthenaList_Init((void *)((int)this + 8),0);
  *(undefined4 *)((int)this + 4) = param_1;
  puVar6 = (undefined4 *)((int)this + 0x42c);
  for (iVar5 = 0x500; iVar5 != 0; iVar5 = iVar5 + -1) {
    *puVar6 = 0;
    puVar6 = puVar6 + 1;
  }
  pcVar9 = local_110;
  pcVar11 = "%s\\font.description";
  local_4 = 0;
  *(undefined4 *)((int)this + 0x428) = 0x3f800000;
  *(undefined4 *)((int)this + 0x420) = 0;
  AthenaString_SprintfToBuffer(pcVar9,(byte *)"%s\\font.description");
  iVar5 = FID_conflict___open(local_110,0x8000,pcVar9,pcVar11,param_2);
  if (iVar5 != -1) {
    __read(iVar5,&local_23c,4);
    __read(iVar5,&local_238,4);
    iVar7 = 0;
    if (0 < local_23c) {
      do {
        AthenaString_SprintfToBuffer(local_210,(byte *)"%s\\data%d.png");
        pvVar1 = Graphics_FindOrCreateTexture(*(void **)((int)this + 4),local_210,'\x01');
        AthenaList_Append((void *)((int)this + 8),(int)pvVar1);
        iVar7 = iVar7 + 1;
      } while (iVar7 < local_23c);
    }
    iVar7 = 0;
    *(undefined4 *)((int)this + 0x424) = 0;
    if (0 < local_238) {
      do {
        __read(iVar5,local_224,1);
        __read(iVar5,&local_22c,4);
        __read(iVar5,local_230,4);
        __read(iVar5,local_220,4);
        __read(iVar5,local_234,4);
        __read(iVar5,&local_240,4);
        __read(iVar5,&local_228,4);
        __read(iVar5,&local_218,4);
        __read(iVar5,&local_21c,4);
        if ((float)*(int *)((int)this + 0x424) < local_240) {
          uVar8 = __ftol2();
          *(int *)((int)this + 0x424) = (int)uVar8;
        }
        uVar10 = local_228;
        uVar12 = local_218;
        uVar13 = local_21c;
        uVar8 = __ftol2();
        iVar2 = (int)uVar8;
        uVar8 = __ftol2();
        iVar3 = (int)uVar8;
        uVar8 = __ftol2();
        iVar4 = (int)uVar8;
        uVar8 = __ftol2();
        Gadget_AddSpriteSlot
                  (this,local_224[0],local_22c,(int)uVar8,iVar4,iVar3,iVar2,uVar10,uVar12,uVar13);
        iVar7 = iVar7 + 1;
      } while (iVar7 < local_238);
    }
    __close(iVar5);
  }
  ExceptionList = local_c;
  return this;
}

