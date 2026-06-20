
void Huffman_TreeAdvance(void)

{
  undefined4 *puVar1;
  int *in_EAX;
  int iVar2;
  
  if (in_EAX[2] < 1) {
    while (*(int *)(in_EAX[3] + 0xc) != 0) {
      in_EAX[5] = in_EAX[5] + *(int *)(in_EAX[3] + 8);
      puVar1 = *(undefined4 **)(in_EAX[3] + 0xc);
      in_EAX[3] = (int)puVar1;
      in_EAX[1] = (puVar1[1] + *(int *)*puVar1) - in_EAX[2];
      iVar2 = *(int *)(in_EAX[3] + 8) + in_EAX[2];
      in_EAX[2] = iVar2;
      if (0 < iVar2) {
        return;
      }
    }
    if ((in_EAX[2] < 0) || (*in_EAX != 0)) {
      puVar1 = (undefined4 *)in_EAX[3];
      in_EAX[1] = puVar1[2] + puVar1[1] + *(int *)*puVar1;
      in_EAX[2] = -1;
      *in_EAX = 0;
    }
  }
  return;
}

