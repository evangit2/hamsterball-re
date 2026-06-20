
void __thiscall MeshIter_InitEdge(void *this,int param_1,int param_2,char param_3)

{
  char cVar1;
  undefined3 extraout_var;
  
  *(bool *)((int)this + 0x19) = param_3 != '\0';
  *(int *)this = param_1;
  *(int *)((int)this + 8) = param_1;
  *(int *)((int)this + 4) = param_2;
  *(bool *)((int)this + 0x18) = param_3 != '\x02';
  cVar1 = FindInSmallIntArray32
                    ((int *)(*(int *)(*(int *)((int)this + 0x14) + 0x40) + param_1 * 0xc),param_2);
  *(int *)((int)this + 0x10) = CONCAT31(extraout_var,cVar1);
  if (param_3 == '\x02') {
    *(uint *)((int)this + 0x10) = (CONCAT31(extraout_var,cVar1) + 2U) % 3;
  }
  *(undefined4 *)((int)this + 0xc) = *(undefined4 *)((int)this + 0x10);
  return;
}

