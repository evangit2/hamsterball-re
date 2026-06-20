// Function: D3DXMesh_ComputeLightingFromNormals
// Address: 0x0049675E
// Calling Convention: __fastcall (param_1 = mesh object)
// Returns: 0
// Xrefs: Called from D3DXMesh_ComputeLighting565 (0x004969CC) and
//        D3DXMesh_ComputeLightingThenAssemble (0x0049767C)
//
// PURPOSE: Computes per-vertex lighting from vertex normals and writes packed
// 16-bit (565 format) color values. Part of D3DX8's software mesh processing pipeline.
//
// MESH OBJECT LAYOUT (relevant offsets):
//   param_1+0x18    → vertex data base pointer
//   param_1+0x1050   → stride component 1 (vertex size factor)
//   param_1+0x1054   → stride component 2
//   param_1+0x106C   → vertex normals array (float*, 8 floats per vertex: pos3+norm3+uv2)
//   param_1+0x1070   → start vertex index
//   param_1+0x1074   → offset to 16-bit color output
//   param_1+0x1078   → vertex count
//   param_1+0x1080   → stride multiplier
//   param_1+0x108C   → dirty flag (1 = needs recompute, cleared after)
//   param_1+0x1090   → enabled flag (must be non-zero)
//   param_1+0x1094   → red/green bit shift
//   param_1+0x1098   → blue bit shift
//
// LIGHTING MODEL:
// Uses a fixed light direction stored in globals:
//   DAT_004dc0fc..0x4dc118 → light direction matrix entries (3x3)
//   DAT_004cf3f0 → bias/ambient term
// For each vertex normal (nx, ny, nz):
//   channel1 = clamp(nx*M00 + ny*M01 + nz*M02 + bias, 0, 255)
//   channel2 = clamp(nx*M10 + ny*M11 + nz*M12 + bias, 0, 255)
//   channel3 = clamp(nx*M20 + ny*M21 + nz*M22 + bias, 0, 80) → 0x80 range
//   channel4 = clamp(nx*M30 + ny*M31 + nz*M32 + bias, 0, 80)
// Packed as: word0 = (channel1 << shift1) | (channel3 << shift2)
//            word1 = (channel2 << shift1) | (channel4 << shift2)
// Written to the mesh's 16-bit color stream (565 format).

undefined4 __fastcall D3DXMesh_ComputeLightingFromNormals(int param_1)
{
  // ... decompiled body ...
}
