
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Ball_ApplyForce(void *this,float param_1,float param_2,float param_3,float param_4)

{
  float10 fVar1;
  
  if ((((*(char *)((int)this + 0x2f9) == '\0') && (*(char *)((int)this + 0x2cc) == '\0')) &&
      (*(int *)((int)this + 0x808) == 0)) && (*(int *)((int)this + 0x2f0) < 0x51)) {
    if (*(int *)((int)this + 0x2f0) != 0) {
      param_4 = param_4 * _DAT_004cf380;
    }
    if (*(char *)((int)this + 0x324) != '\0') {
      param_4 = param_4 * (float)_DAT_004cf378;
    }
    if (*(int *)((int)this + 0xc5c) != 0) {
      param_4 = param_4 * _DAT_004cf374;
      *(float *)((int)this + 0xfc) = param_1 * _DAT_004cf3e8;
      *(float *)((int)this + 0x100) = param_2 * _DAT_004cf3e8;
      *(float *)((int)this + 0x104) = param_3 * _DAT_004cf3e8;
    }
    if (*(char *)((int)this + 0xc4c) != '\0') {
      param_4 = param_4 * _DAT_004cf36c;
    }
    *(float *)((int)this + 0x170) = param_1 * param_4 + *(float *)((int)this + 0x170);
    *(float *)((int)this + 0x174) = param_2 * param_4 + *(float *)((int)this + 0x174);
    *(float *)((int)this + 0x178) = param_3 * param_4 + *(float *)((int)this + 0x178);
    if (((param_1 != _DAT_004cf368) || (param_2 != _DAT_004cf368)) || (param_3 != _DAT_004cf368)) {
      if (*(int *)((int)this + 0x748) == 0) {
        fVar1 = FUN_00457de0(0x4f7188,param_1,param_3,0.0,0.0);
        *(float *)((int)this + 0x198) = (float)fVar1;
      }
      if (*(int *)((int)this + 0x748) == 1) {
        fVar1 = FUN_00457de0(0x4f7188,-param_2,param_3,0.0,0.0);
        *(float *)((int)this + 0x198) = (float)fVar1;
      }
      if (*(int *)((int)this + 0x748) == 2) {
        fVar1 = FUN_00457de0(0x4f7188,param_1,param_2,0.0,0.0);
        *(float *)((int)this + 0x198) = (float)fVar1;
      }
      *(undefined1 *)((int)this + 0x19c) = 1;
    }
  }
  return;
}

