/*
 * Function: RumbleBoard_ctor
 * Address: 0x004217b0
 *
 * Description:
 *
Constructor for RumbleBoard (arena mode board). Inherits from Board (Gadget+Scene).
Size ~0x47D4. Vtable 0x4D1358. Initializes AthenaList at +0x4394, RumbleTimer at +0x47C8,
time_limit at +0x47AC=6000 (100 seconds), name "RumbleBoard". Sets fields +0x47B0-0x47C5.
Called 15x — every arena init calls this. ~1384 chars.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
