
void __thiscall Sawblade_SetBreakSound(void *this,int param_1)

{
  undefined4 *puVar1;
  undefined4 local_c [3];
  
  *(int *)((int)this + 0x10f8) = param_1;
  if (param_1 == 1) {
    puVar1 = (undefined4 *)
             AthenaHashTable_Lookup
                       (*(void **)(*(int *)((int)this + 0x10d0) + 0x8ac),local_c,"SAW1-BREAK",
                        (undefined1 *)0x0);
    if ((undefined4 *)((int)this + 0x1100) != puVar1) {
      *(undefined4 *)((int)this + 0x1100) = *puVar1;
      *(undefined4 *)((int)this + 0x1104) = puVar1[1];
      *(undefined4 *)((int)this + 0x1108) = puVar1[2];
    }
  }
  if (*(int *)((int)this + 0x10f8) == 2) {
    *(undefined4 *)((int)this + 0x10ec) = 0xc2b40000;
    puVar1 = (undefined4 *)
             AthenaHashTable_Lookup
                       (*(void **)(*(int *)((int)this + 0x10d0) + 0x8ac),local_c,"SAW2-BREAK",
                        (undefined1 *)0x0);
    if ((undefined4 *)((int)this + 0x1100) != puVar1) {
      *(undefined4 *)((int)this + 0x1100) = *puVar1;
      *(undefined4 *)((int)this + 0x1104) = puVar1[1];
      *(undefined4 *)((int)this + 0x1108) = puVar1[2];
    }
  }
  return;
}

