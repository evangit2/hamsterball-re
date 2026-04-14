/* Waypoint & Arrow System - Deep Documentation
 *
 * Waypoint system manages race checkpoints that guide the player
 * through levels. Displayed as directional arrows on HUD.
 *
 * ═══════════════════════════════════════════════════════════════
 * WaypointList Structure (size ~0x424+ bytes)
 * ═══════════════════════════════════════════════════════════════
 *
 * +0x000: vtable (0x4D262C = WaypointList_DeletingDtor)
 * +0x004: AthenaList waypoints (embedded, 0x418 bytes)
 * +0x41C: current_waypoint_index (int)
 * +0x420: last_waypoint_Graphics_ptr (copied from ball+0x154)
 * +0x424: distance_max (default 9999999, from ctor)
 *
 * ═══════════════════════════════════════════════════════════════
 * Waypoint Entry Structure (size 0x28 = 40 bytes)
 * ═══════════════════════════════════════════════════════════════
 *
 * Allocated via operator_new(0x28):
 *   +0x00: float X (position)
 *   +0x04: float Y (position)
 *   +0x08: float Z (position)
 *   +0x0C: float value_1  (Ball+0x190 range/scale)
 *   +0x10: float value_2  (Ball+0x194 speed?)
 *   +0x14: float value_3  (Ball+0x150 gravity?)
 *   +0x18: byte  collision_flag (Ball+0x748 collision mesh)
 *   +0x1C: float value_4  (Ball+0x74C)
 *   +0x20: float value_5  (Ball+0x750)
 *   +0x24: float radius  (Ball+0x284 = 35.0)
 *
 * ═══════════════════════════════════════════════════════════════
 * WaypointList_ctor (0x427660)
 * ═══════════════════════════════════════════════════════════════
 *
 * Init: set vtable 0x4D262C, AthenaList_Init for embedded list,
 * set distance_max at +0x424 to 9999999 (sentinel value).
 *
 * ═══════════════════════════════════════════════════════════════
 * WaypointList_SetNextWaypoint (0x427690)
 * ═══════════════════════════════════════════════════════════════
 *
 * 1. Read current waypoint index from this+0x41C
 * 2. Get waypoint entry from AthenaList array at this+0x410
 * 3. Two Graphics_SetPosition calls (SceneObject vtable[0x34]):
 *    - First: ball's current position (ball+0x164/168/16C), flag=1
 *    - Second: waypoint position (entry+0/4/8), flag=0
 * 4. Copy 10 fields from waypoint to ball:
 *    ball+0x164 ← entry[0] (X)
 *    ball+0x168 ← entry[1] (Y)
 *    ball+0x16C ← entry[2] (Z)
 *    ball+0x190 ← entry[3] (range)
 *    ball+0x194 ← entry[4] (speed)
 *    ball+0x150 ← entry[5] (gravity flag)
 *    ball+0x748 ← entry[6] (collision flag, as byte)
 *    ball+0x74C ← entry[7]
 *    ball+0x750 ← entry[8]
 *    ball+0x284 ← entry[9] (radius)
 * 5. Copy Graphics* from this+0x420 to ball+0x154
 *
 * ═══════════════════════════════════════════════════════════════
 * WaypointList_AppendCurrent (0x427810)
 * ═══════════════════════════════════════════════════════════════
 *
 * Creates new 0x28-byte waypoint entry from current ball state:
 *   entry[0] = ball+0x164 (X)
 *   entry[1] = ball+0x168 (Y)
 *   entry[2] = ball+0x16C (Z)
 *   entry[3] = ball+0x190 (range)
 *   entry[4] = ball+0x194 (speed)
 *   entry[5] = ball+0x150 (gravity)
 *   entry[6] = ball+0x748 (collision, byte)
 *   entry[7] = ball+0x74C
 *   entry[8] = ball+0x750
 *   entry[9] = ball+0x284 (radius)
 * Then copies ball+0x154 (Graphics*) to this+0x420
 * Appends entry to embedded AthenaList at this+0x04
 *
 * ═══════════════════════════════════════════════════════════════
 * WaypointList_dtor (0x427760)
 * ═══════════════════════════════════════════════════════════════
 *
 * 1. Set vtable to deleting dtor (0x4D262C)
 * 2. Iterate AthenaList at this+0x04, free each waypoint entry
 * 3. AthenaList_Free on the embedded list
 * 4. Vec3List_Free on the embedded list (same memory)
 *
 * ═══════════════════════════════════════════════════════════════
 * Catapult_Launch (0x434290)
 * ═══════════════════════════════════════════════════════════════
 *
 * Simple trigger: set catapult+0x10F0 = 1 (activate)
 *                  set catapult+0x10F4 = 50 (0x32 = timer/duration)
 *
 * Called when ball hits E:CATAPULTBOTTOM collision event.
 * The catapult then applies trajectory force to the ball during
 * its update cycle.
 */