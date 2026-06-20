/*
 * Function: CreateMechanicalObjects
 * Address: 0x00417fe0
 *
 * Description:
 *
Factory dispatcher for mechanical objects. Matches multiple name prefixes:
  - "LOOPER" (6) → Looper_ctor (0x1500 bytes), appends to rotator list
  - "BIGGEAR" (7) → BigGear_ctor (0x1514 bytes)
  - "BONKBASH" (8) → BonkBash_ctor
  - Other mechanical objects via CreatePlatformOrStands fallback
Each creates objects with the scene's spatial tree pointer (this+0x436C or 0x47E0).
1 vtable ref. ~5353 chars.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
