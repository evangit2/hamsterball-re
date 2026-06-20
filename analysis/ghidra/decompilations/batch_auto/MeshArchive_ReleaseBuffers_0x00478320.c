
void MeshArchive_ReleaseBuffers(void)

{
  int unaff_ESI;
  
  Vorbis_FreeDecoder((undefined4 *)(unaff_ESI + 0x68));
  Vorbis_FreeDecodeState((undefined4 *)(unaff_ESI + 0xb8));
  *(undefined4 *)(unaff_ESI + 0x40) = 2;
  return;
}

