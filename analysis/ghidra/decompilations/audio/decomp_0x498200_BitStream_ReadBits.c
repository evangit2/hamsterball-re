// Function: BitStream_ReadBits
// Address: 0x00498200
// Calling Convention: __cdecl
// Parameters: uint *param_1 (bitstream state), int param_2 (number of bits to read, 1-32)
// Returns: uint (value read, or 0xFFFFFFFF on stream end)
// Xrefs: Called from Vorbis_ProcessPacket, Vorbis_ReadIDHeader, Vorbis_ReadCommentHeader,
//        Vorbis_ReadSetupHeader (20+ call sites)
//
// PURPOSE: Bit-level reader for Ogg Vorbis compressed audio streams.
// Reads N bits from a bit-accurate position in a byte buffer, handling
// arbitrary bit alignment and multi-byte reads.
//
// BITSTREAM STATE (param_1, 12 bytes):
//   param_1[0] (offset +0x00): bit position within current byte (0-7)
//   param_1[1] (offset +0x04): current byte pointer
//   param_1[2] (offset +0x08): remaining bytes in stream
//
// ALGORITHM:
// 1. Compute new bit position (current + num_bits)
// 2. If within buffer: read bytes at current position, shift and OR to assemble value
//    - Handles up to 5 bytes for 32-bit reads crossing byte boundaries
//    - Uses lookup table at DAT_004fa740 for bit masks
//    - Advances byte pointer and bit offset
// 3. If past buffer end: check Huffman_StreamEnd for error, read remaining bits
//    from stream boundary, call Huffman_TreeAdvance2 to advance Vorbis decoder state
// 4. Returns value masked to param_2 bits, or -1 on error
//
// This is part of the statically-linked Ogg Vorbis decoder in the game binary.
// The game uses .mo3 container files with Vorbis-compressed audio.

uint __cdecl BitStream_ReadBits(uint *param_1, int param_2)
{
  // ... decompiled body ...
}
