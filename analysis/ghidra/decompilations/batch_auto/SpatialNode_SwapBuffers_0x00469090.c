
void __thiscall SpatialNode_SwapBuffers(void *this,char param_1)

{
  Path_ComputeSegmentLengths((int)this);
  if (param_1 != '\0') {
    Path_ComputeSegDeltas((int)this,(void *)((int)this + 0x30));
    Path_ComputeSegDeltas((int)this + 0x10,(void *)((int)this + 0x40));
    Path_ComputeSegmentLengths((int)this);
    return;
  }
  Path_BuildVertexStrips((int)this,(void *)((int)this + 0x30));
  Path_BuildVertexStrips((int)this + 0x10,(void *)((int)this + 0x40));
  Path_ComputeSegmentLengths((int)this);
  return;
}

