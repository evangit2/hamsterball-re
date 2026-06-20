
int __thiscall Mesh_FindElement(void *this,float *param_1)

{
  float *pfVar1;
  int iVar2;
  
  iVar2 = 0;
  if (0 < *(int *)((int)this + 0x1c)) {
    pfVar1 = *(float **)((int)this + 0x20);
    do {
      if (*pfVar1 == *param_1) {
        if (pfVar1[1] == param_1[1]) {
          if (pfVar1[2] == param_1[2]) {
            if (pfVar1[3] == param_1[3]) {
              if (pfVar1[4] == param_1[4]) {
                if (pfVar1[5] == param_1[5]) {
                  if (pfVar1[6] == param_1[6]) {
                    if (pfVar1[7] == param_1[7]) {
                      return iVar2;
                    }
                  }
                }
              }
            }
          }
        }
      }
      iVar2 = iVar2 + 1;
      pfVar1 = pfVar1 + 0x109;
    } while (iVar2 < *(int *)((int)this + 0x1c));
  }
  return -1;
}

