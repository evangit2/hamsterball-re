
void __cdecl MeshArchive_SetPosition(int param_1,int param_2)

{
  int *unaff_ESI;
  
  if (*unaff_ESI != 0) {
    (*(code *)unaff_ESI[0x47])(*unaff_ESI,param_1,param_2,0);
    unaff_ESI[2] = param_1;
    unaff_ESI[3] = param_2;
    Pool_Reset(unaff_ESI[6]);
  }
  return;
}

