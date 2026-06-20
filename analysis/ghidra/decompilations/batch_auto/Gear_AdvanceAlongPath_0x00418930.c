/*
 * Function: Gear_AdvanceAlongPath
 * Address: 0x00418930
 *
 * Description:
 *
Spline path follower for gears/rotators using 8-sample gradient descent.
Samples the spline path at +/- step/medium/far offsets, picks the lowest
collision cost direction, and advances the path parameter accordingly.
First frame: 100K iterations at 0.01 step (initialization), then 1 iteration
at 0.001 step per frame (normal movement). Uses Mesh_FindClosestCollision
for cost evaluation. Called 1x from Gear_Update. ~8879 chars.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
