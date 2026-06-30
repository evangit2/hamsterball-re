/* Ghidra Decompilation
 * Function: Ball_Shatter
 * Address: 0x408D70
 * Decompiled: 2026-06-19
 */


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Ball_Shatter(void *this,int param_1)

{
  int *piVar1;
  void *this_00;
  int iVar2;
  int iVar3;
  undefined1 *puVar4;
  int *this_01;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004c947b;
  local_c = ExceptionList;
  if (*(char *)((int)this + 0x324) == '\0') {
    ExceptionList = &local_c;
    if (*(void **)((int)this + 0xc28) != (void *)0x0) {
      ExceptionList = &local_c;
      _free(*(void **)((int)this + 0xc28));
      *(undefined4 *)((int)this + 0xc28) = 0;
    }
    (**(code **)(**(int **)((int)this + 0x14) + 0x78))();
    *(undefined1 *)((int)this + 0x2e8) = 1;
    if (*(int *)((int)this + 0x744) == 0) {
      if (*(float *)((int)this + 0xc50) < _DAT_004cf310) {
        if (*(char *)((int)this + 0xc4c) == '\0') {
          iVar3 = *(int *)(*(int *)((int)this + 0x10) + 0x44c);
        }
        else {
          iVar3 = *(int *)(*(int *)((int)this + 0x10) + 0x450);
        }
      }
      else {
        iVar3 = *(int *)(*(int *)((int)this + 0x10) + 0x500);
      }
      Sound_PlayChannel(iVar3);
      Scene_ForEachBall_SetVelocity
                (*(void **)((int)this + 0x14),*(float *)((int)this + 0x164),
                 *(float *)((int)this + 0x168),*(float *)((int)this + 0x16c));
      puVar4 = (undefined1 *)0x0;
      do {
        this_00 = operator_new(0xc64);
        this_01 = (int *)0x0;
        uStack_4 = 0;
        if (this_00 != (void *)0x0) {
          this_01 = Ball_Split_ctor(this_00,*(int *)((int)this + 0x14));
        }
        uStack_4 = 0xffffffff;
        *(undefined1 *)(this_01 + 0xc9) = 1;
        (**(code **)(*this_01 + 4))();
        iVar3 = *(int *)((int)this + 0x1a4);
        iVar2 = this_01[0x69];
        if ((undefined4 *)(iVar2 + 0xca4) != (undefined4 *)(iVar3 + 0xca4)) {
          *(undefined4 *)(iVar2 + 0xca4) = *(undefined4 *)(iVar3 + 0xca4);
          *(undefined4 *)(iVar2 + 0xca8) = *(undefined4 *)(iVar3 + 0xca8);
          *(undefined4 *)(iVar2 + 0xcac) = *(undefined4 *)(iVar3 + 0xcac);
        }
        if ((((int)puVar4 < 0) || (*(int *)(param_1 + 4) <= (int)puVar4)) ||
           (piVar1 = *(int **)(*(int *)(param_1 + 0x40c) + (int)puVar4 * 4), piVar1 == (int *)0x0))
        {
          this_01[0x59] = *(int *)((int)this + 0x164);
          this_01[0x5a] = *(int *)((int)this + 0x168);
          iVar3 = *(int *)((int)this + 0x16c);
        }
        else {
          this_01[0x59] = *piVar1;
          this_01[0x5a] = piVar1[1];
          iVar3 = piVar1[2];
        }
        this_01[0x5b] = iVar3;
        Ball_SetTrajectory(this_01,&PTR_Vec3_dtor_004cf300,*(float *)((int)this + 0x2ac),
                           *(float *)((int)this + 0x2b0),*(float *)((int)this + 0x2b4),
                           *(float *)((int)this + 0x2b8));
        this_01[6] = -1;
        this_01[0x9e] = 0x3c23d70a;
        this_01[0x9f] = 0x3f000000;
        this_01[0xa1] = *(int *)((int)this + 0x284);
        (**(code **)(*this_01 + 0x14))();
        this_01[0x1d5] = (int)(puVar4 + 1);
        AthenaList_Append((void *)(*(int *)((int)this + 0x14) + 0x3204),(int)this_01);
        iVar3 = *this_01;
        *(undefined1 *)((int)this_01 + 0x31d) = 1;
        this_01[0xc6] = 0x41200000;
        RNG_Rand(&PTR_OBJ_VTABLE,5,'\x01');
        RNG_Rand(&PTR_OBJ_VTABLE,10,'\0');
        iVar2 = RNG_Rand(&PTR_OBJ_VTABLE,5,'\x01');
        (**(code **)(iVar3 + 0x14))((float)iVar2);
        if (puVar4 == (undefined1 *)0x0) {
          this_01[0xca] = 1;
        }
        else if (puVar4 == (undefined1 *)0x1) {
          this_01[0xca] = 2;
        }
        else if (puVar4 == (undefined1 *)0x2) {
          this_01[0xca] = 4;
        }
        puVar4 = &stack0xffffffc8;
      } while ((int)&stack0xffffffc8 < 3);
      Ball_CreateTrailParticles((int)this);
    }
  }
  ExceptionList = local_c;
  return;
}

