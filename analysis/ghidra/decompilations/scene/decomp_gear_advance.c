/* Gear_AdvanceAlongPath - Spline path following with collision avoidance
 * Address: 0x418930
 * Called from: Scene_Update when gear_has_active_path is true
 * 
 * GEAR STRUCT OFFSETS:
 *   +0x00 = path_ptr (Path* - spline to follow)
 *   +0x04 = path_param (float, current position on path [0..N])
 *   +0x08 = initialized (bool, first frame flag)
 *   +0x0C = max_iterations (int, set to 10 on first frame)
 *
 * PARAMETERS:
 *   param_1, param_2, param_3 = target position (camera/ball XYZ)
 *
 * ALGORITHM (gradient descent on path distance):
 *   1. On first frame: set step_size=0.01, max_iter=100000
 *      Subsequent frames: step_size=0.001, max_iter=1
 *   2. Compute 8 gradient sample points by polling path at offsets:
 *      - ±step_size (near forward/backward)
 *      - ±step_size*_DAT_004cf9f8 (medium range)
 *      - ±step_size*_DAT_004cf3ec (far range)  
 *      - ±step_size*_DAT_004cf454 (very far range)
 *   3. For each sample: Path_GetPosition(path, pos, param + offset)
 *   4. Evaluate collision cost at each sample via Collision_GradientEval_Stub
 *   5. Pick the sample with LOWEST cost (gradient descent direction)
 *   6. Move path_param in the direction of the best sample:
 *      - Sample 0: +step (forward)
 *      - Sample 1: -step (backward)
 *      - Sample 2: +step * multiplier_0
 *      - Sample 3: +medium_step
 *      - Sample 4: -medium_step
 *      - Sample 5: +step * multiplier_1
 *      - Sample 6: +far_step
 *      - Sample 7: -far_step
 *   7. Repeat until min cost reached (samples 0 or 1 = at end)
 *      OR max iterations reached
 *      OR position close enough to target (distance < _DAT_004cf55c)
 *
 * PATH CLAMPING:
 *   - path_param clamped to [0.0, path_length-1]
 *   - All sample offsets clamped similarly before evaluation
 *
 * COLLISION AVOIDANCE:
 *   - Collision_GradientEval_Stub samples collision at each candidate position
 *   - This prevents the gear/camera from moving into collision geometry
 *   - The gradient descent naturally avoids local minima by checking
 *     8 different step sizes simultaneously
 *
 * This is essentially a camera/gear rail system where the camera
 * follows a predefined spline path but is steered away from collisions
 * using gradient descent. The 8-sample approach gives directional
 * information without computing actual derivatives.
 */
void __thiscall Gear_AdvanceAlongPath(void *this, float param_1, float param_2, float param_3)
{
    // Full decompilation saved - see comments above for documented offsets
}