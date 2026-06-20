
uint __cdecl
MeshArchive_InitAndLoad
          (undefined4 param_1,int *param_2,undefined4 *param_3,uint param_4,int param_5,
          undefined *param_6,int param_7,int param_8)

{
  uint uVar1;
  
  uVar1 = MeshArchive_ctor(param_3,param_4,param_5,param_6,param_7,param_8);
  if (uVar1 == 0) {
    if (param_2[0x10] < 2) {
      param_2[0x10] = 2;
    }
    if (param_2[1] != 0) {
      uVar1 = MeshArchive_LoadComplete();
      if (uVar1 != 0) {
        *param_2 = 0;
        MeshArchive_dtor(param_2);
      }
      return uVar1;
    }
    uVar1 = 0;
  }
  return uVar1;
}

