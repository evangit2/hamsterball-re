// Scene_CheckPath @ 0x00457EC0
// Ring topology pathfinder on 360-cell circular grid
// Used in Ball_Update for angular collision and track snapping
//
// Parameters:
//   param_1: start position (0-358, 0x167-1)
//   param_2: target position (0-358)
//
// Returns:
//   1  = target reachable going forward (clockwise)
//   -1 = target reachable going backward (counter-clockwise)  
//   0  = unreachable (shouldn't happen on a ring)
//
// The grid size 0x167 = 359 cells. The ring wraps around,
// so position 0 is adjacent to position 358.
// Two walkers start from start pos: one goes forward (+1),
// one goes backward (-1). Whichever reaches target first
// determines the direction.

int Scene_CheckPath(int start, int target)
{
  int forward = start;
  int backward = start;
  int steps = 0;
  
  while (true) {
    if (forward == target) return 1;   // Forward reachable
    if (backward == target) return -1;  // Backward reachable
    
    // Advance forward walker (wrap at 359->0)
    forward = (forward + 1) % 360;
    // Retreat backward walker (wrap at 0->359)
    backward = (backward - 1 + 360) % 360;
    
    steps++;
    if (steps > 360) return 0;  // Unreachable (safety)
  }
}