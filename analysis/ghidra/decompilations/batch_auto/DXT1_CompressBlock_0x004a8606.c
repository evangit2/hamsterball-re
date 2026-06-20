
int __cdecl DXT1_CompressBlock(char *param_1)

{
  int iVar1;
  float local_104 [64];
  
  iVar1 = Color_PremultiplyAlpha(local_104);
  if (-1 < iVar1) {
    iVar1 = DXT1_CompressColorBlock(param_1,(int)local_104);
    if (-1 < iVar1) {
      iVar1 = 0;
    }
  }
  return iVar1;
}

