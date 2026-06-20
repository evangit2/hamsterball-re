
undefined4 * MeshTexture_Init2D(void)

{
  uint uVar1;
  uint uVar2;
  void *pvVar3;
  undefined4 *this;
  int iVar4;
  int unaff_EBP;
  
  __security_init_cookie();
  *(undefined4 **)(unaff_EBP + -0x14) = this;
  Mesh_InitTexture(this,*(undefined4 **)(unaff_EBP + 8),0,1);
  uVar1 = this[0x40c];
  uVar2 = this[0x40e] + 1 & 0xfffffffe;
  iVar4 = uVar2 - (uVar1 & 0xfffffffe);
  this[0x41e] = uVar2;
  *(undefined4 *)(unaff_EBP + -4) = 0;
  *this = &PTR_LAB_004dc0bc;
  this[0x41c] = uVar1 & 0xfffffffe;
  this[0x41d] = 0;
  this[0x420] = 0;
  this[0x41f] = 0;
  this[0x421] = 0;
  this[0x422] = iVar4;
  this[0x423] = 0;
  this[0x424] = 1;
  pvVar3 = operator_new(iVar4 * 0x10);
  *(void **)(unaff_EBP + -0x10) = pvVar3;
  *(undefined1 *)(unaff_EBP + -4) = 1;
  if (pvVar3 == (void *)0x0) {
    iVar4 = 0;
  }
  else {
    RepeatCall(pvVar3,0x10,iVar4,&LAB_0047d949);
    iVar4 = *(int *)(unaff_EBP + -0x10);
  }
  this[0x41b] = iVar4;
  if (iVar4 == 0) {
    this[0x424] = 0;
  }
  if (*(int *)(*(int *)(unaff_EBP + 8) + 4) == 0x59565955) {
    this[0x425] = 8;
    this[0x426] = 0;
  }
  else {
    this[0x425] = 0;
    this[0x426] = 8;
  }
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return this;
}

