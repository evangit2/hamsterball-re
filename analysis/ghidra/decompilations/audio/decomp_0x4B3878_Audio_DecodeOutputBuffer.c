// Function: Audio_DecodeOutputBuffer
// Address: 0x004B3878
// Calling Convention: __cdecl
// Parameters: int param_1 (audio decoder state), param_2-param_5 (decode buffer params),
//             int *param_6 (output sample count accumulator)
// Xrefs: Referenced from data section 0x004B39D6 (vtable entry — virtual decode method)
//
// PURPOSE: Decodes compressed audio data into an output PCM buffer. Part of the
// game's audio codec pipeline (likely the MO3 container decoder, which wraps
// MP3/Vorbis streams).
//
// DECODER STATE OBJECT (param_1):
//   param_1+0x04 → codec vtable (has decode methods at +0x1C, +0x04)
//   param_1+0x188 → stream state struct:
//     state+0x08 → source data pointer
//     state+0x0C → decoded frame pointer
//     state+0x10 → frame size (bytes per decoded frame)
//     state+0x14 → source position/offset
//     state+0x18 → consumed samples counter
//   param_1+0x19C → output converter vtable (method at +0x04)
//   param_1+0x1A4 → post-process vtable (method at +0x04)
//
// ALGORITHM:
// 1. Check if current frame is fully consumed (state+0x18 == 0)
// 2. If so, decode a new frame: call codec->vtable[+0x1C](decoder, source, position, frame_size, 1)
//    → stores decoded frame at state+0x0C
// 3. Save consumed count (uVar3 = state+0x18 before reset)
// 4. Convert decoded samples to output format: call output_converter->vtable[+0x04](
//      decoder, param_2, param_3, param_4, decoded_frame, consumed_ptr, frame_size)
// 5. If consumed < total in frame: call post_processor->vtable[+0x04]() on remaining samples
// 6. Advance source position (state+0x14 += state+0x10)
// 7. Reset consumed counter if frame fully used (state+0x18 = 0)
// 8. Accumulate decoded sample count into *param_6

void __cdecl Audio_DecodeOutputBuffer(int param_1, undefined4 param_2, undefined4 param_3,
                                       undefined4 param_4, undefined4 param_5, int *param_6)
{
  // ... decompiled body ...
}
