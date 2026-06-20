
void __fastcall Menu_MergeAllLists(int param_1)

{
  void *this;
  
  this = (void *)(param_1 + 0x87c);
  AthenaList_MergeSorted(this,param_1 + 0x14c4);
  AthenaList_MergeSorted(this,param_1 + 0x210c);
  AthenaList_MergeSorted(this,param_1 + 0x18dc);
  AthenaList_MergeSorted(this,param_1 + 0x2524);
  AthenaList_MergeSorted(this,param_1 + 0x293c);
  AthenaList_MergeSorted(this,param_1 + 0x1cf4);
  AthenaList_MergeSorted(this,param_1 + 0x10ac);
  return;
}

