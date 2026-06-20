
void __thiscall MeshIter_InitShortEdge(void *this,ushort param_1,ushort param_2,char param_3)

{
  char cVar1;
  undefined3 extraout_var;
  
  *(ushort *)this = param_1;
  *(ushort *)((int)this + 4) = param_1;
  *(bool *)((int)this + 0x11) = param_3 != '\0';
  *(ushort *)((int)this + 2) = param_2;
  *(bool *)((int)this + 0x10) = param_3 != '\x02';
  cVar1 = WideString_MatchSlot
                    ((ushort *)(*(int *)(*(int *)((int)this + 0xc) + 0x40) + (uint)param_1 * 6),
                     (uint)param_2);
  *(short *)((int)this + 8) = (short)CONCAT31(extraout_var,cVar1);
  if (param_3 == '\x02') {
    *(short *)((int)this + 8) =
         (short)((ulonglong)((CONCAT31(extraout_var,cVar1) & 0xffff) + 2) % 3);
  }
  *(undefined2 *)((int)this + 6) = *(undefined2 *)((int)this + 8);
  return;
}

