
void __thiscall TourneyMenu_WriteSave(void *this,char *param_1)

{
  int iVar1;
  
  iVar1 = CRT_OpenFileExclusive(param_1,0x180);
  __close(iVar1);
  iVar1 = FID_conflict___open(param_1,0x8002);
  __write(iVar1,(void *)((int)this + 8),4);
  __write(iVar1,(void *)((int)this + 0x14),4);
  __write(iVar1,(void *)((int)this + 0x18),0x3c);
  __write(iVar1,(void *)((int)this + 0x54),0x3c);
  __write(iVar1,(void *)((int)this + 0x90),4);
  __write(iVar1,(void *)((int)this + 0x94),1);
  __write(iVar1,(void *)((int)this + 0x95),1);
  __write(iVar1,(void *)(*(int *)((int)this + 4) + 0x236),1);
  __write(iVar1,(void *)(*(int *)((int)this + 4) + 0x23c),4);
  __write(iVar1,(void *)(*(int *)((int)this + 4) + 0x5e8),4);
  __write(iVar1,(void *)(*(int *)((int)this + 4) + 0x5e4),4);
  __write(iVar1,(void *)(*(int *)((int)this + 4) + 0x5f4),4);
  __close(iVar1);
  return;
}

