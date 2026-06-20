// Function: ComputeScanlineZBuffer
// Address: 0x004912E4
// Calling Convention: __cdecl
// Parameters: int param_1 (wrap mode: 0=clamp, non-zero=wrap)
// Returns: uint* (allocated Z-buffer array, or NULL on allocation failure)
// Xrefs: Called from D3DX_TransformTex_Bilinear (×2) and DDSurface_Blt3PointWBuffer (×3)
//
// PURPOSE: Pre-computes a scanline interpolation Z-buffer for perspective-correct
// texture mapping. Part of the D3DX8 software rasterization pipeline used for
// bilinear texture filtering and 3-point perspective correction.
//
// INPUTS (from registers, not params):
//   unaff_EBX → scanline width (pixel count, used for allocation: width × 16 bytes)
//   unaff_EDI → source texture height (for wrap/clamp boundary)
//   unaff_ESI → FPU rounding mode (for FPU_RoundDouble)
//
// OUTPUT ARRAY (width × 4 uint32 entries, i.e., width × 16 bytes):
// For each pixel x in the scanline:
//   buf[x*4+0] → integer V coordinate (clamped/wrapped to [0, height-1])
//   buf[x*4+1] → fractional weight for pixel x (1.0 - frac)
//   buf[x*4+2] → integer V coordinate for pixel x+1 (adjacent texel)
//   buf[x*4+3] → fractional weight for pixel x+1 (frac itself)
//
// ALGORITHM:
// 1. Allocate width × 16 bytes via operator_new
// 2. For each pixel in the scanline:
//    a. Compute interpolation factor: t = (float)x / (float)width
//    b. Compute source V coordinate: v = t * (height/width) - 0.5 (half-texel offset)
//    c. Round V to nearest integer using FPU_RoundDouble
//    d. Compute fractional part: frac = 1.0 - (v - rounded)
//    e. Clamp/wrap V to [0, height-1] based on param_1 (wrap mode)
//    f. Also compute V+1 (adjacent texel for bilinear filter), clamp/wrap that too
//    g. Store: integer V, fractional weight, V+1, (1 - frac) into 4-slot entry
//
// This is the core of D3DX8's software texture sampling — it pre-computes the
// vertical interpolation weights so the inner rasterizer loop only needs to
// multiply-add. The function is used during texture transformation and surface
// blitting operations that require perspective-correct or bilinear-filtered
// texture reads.

uint * __cdecl ComputeScanlineZBuffer(int param_1)
{
  // ... decompiled body ...
}
