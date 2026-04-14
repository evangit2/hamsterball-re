/*
 * Hamsterball App Resource Loader (originally named TimerDisplay at 0x42A8C0)
 * 
 * This function is the master asset loader called during app initialization.
 * Creates LoadingScreenGadget (0x3628 bytes) and loads ALL game resources
 * through vtable callbacks on the gadget.
 *
 * LoadingScreenGadget vtable method indices:
 *   +0x48  LoadTexture(dest, filename, alpha_flag)  - texture with optional alpha
 *   +0x4C  LoadMesh(dest, filename)                 - mesh file
 *   +0x50  LoadLevel(dest, filename)                 - level geometry
 *   +0x54  LoadCollision(dest, level_src)            - collision from level
 *   +0x58  LoadSprite(dest, filename)                 - sprite/texture
 *   +0x5C  LoadFont(dest, font_path)                 - font resource
 *   +0x60  LoadSound(dest, sound_path, max_channels)  - sound with channel pool
 *
 * === FONT ASSETS ===
 *   +0x318  "fonts\\showcardgothic28"         (main title)
 *   +0x324  "fonts\\arialnarrow12bold"        (UI detail)
 *   +0x31C  "fonts\\showcardgothic14"         (small label)
 *   +0x328  "fonts\\showcardgothic72"         (race timer digits)
 *   +800    "fonts\\showcardgothic16"          (info text)
 *
 * === TEXTURE ASSETS (sprites) ===
 *   Title:  +0x348 titletext-left.png, +0x34C titletext-right.png
 *   Hammy:  +0x330 hammy1.png, +0x334 hammy2.png, +0x338 hammy3.png
 *   HUD:    +0x390 timerblot.png, +0x33C ballborder.png, +0x340 ballburner.png
 *           +0x344 sweat.png, +0x350 raptisoftlogo.png
 *   Effects:+0x3A0 star.png, +0x3A4 dust.png, +0x3A8 safecircle.png
 *   Chrome: +0x368 chrome.png, +0x36C chromeshadow.png
 *   Lock:   +0x3AC lock.png, +0x3B0 lock.png
 *   Medals: +0x370 bronze-small.png, +0x374 silver-small.png, +0x378 gold-small.png
 *           +0x37C goldenweasel.png
 *   Icons:  +0x380 bronze-icon.png, +900 silver-icon.png, +0x388 gold-icon.png
 *           +0x38C goldenweasel-icon.png
 *   UI bg:  +0x354 blueblot.png, +0x358 blueblot2.png, +0x35C bluecircle.png
 *   Signs:  +0x27C sign-bewarethetar.png, +0x280 sign-bewarethetar.png(alpha)
 *           +0x284 sign-bewarethetar-mirrored.png
 *   Goals:  +0x294 goal.png, +0x298 goal.png(alpha), +0x29C goal-lit.png(alpha)
 *           +0x2A0 goal-mirrored.png(alpha), +0x2A4 goal-lit-mirrored.png(alpha)
 *           +0x2B4 goal-round.png, +0x2B8 goal-round.png(alpha)
 *           +700 goal-round-lit.png(alpha), +0x2C0 goal-round-mirrored.png(alpha)
 *           +0x2C4 goal-round-lit-mirrored.png(alpha)
 *   Arrows: +0x288 arrow1.png, +0x28C arrow1.png(alpha), +0x290 arrow1-mirrored.png
 *   Locks:  +0x2A8 locktile.png, +0x2AC locktile.png(alpha), +0x2B0 locktile2.png(alpha)
 *
 * === LEVEL CHECKER/BRICK TEXTURES (all non-alpha) ===
 *   +0x2C8 pinkchecker.bmp,     +0x2D0 bluebrick.png
 *   +0x2CC bluechecker.bmp,     +0x2D4 greenchecker.bmp
 *   +0x2D8 greenbrick.png,      +0x2DC yelllowchecker.png
 *   +0x2E0 greyoutlinechecker.png
 *   +0x2E4 redcheckers.bmp,    +0x2E8 redbrick.png
 *   +0x2EC orangechecker.bmp,  +0x2F0 orangebrick.png
 *   +0x2F4 brightgreenchecker.bmp, +0x2F8 brightgreenbrick.png
 *   +0x2FC toobchecker.png,    +0x300 toobbrick.png
 *   +0x304 skychecker.png,     +0x308 purplechecker.bmp
 *   +0x30C purplebrick.png,    +0x310 brownbrick.png
 *   +0x314 blackchecker.png
 *
 * === MESH ASSETS ===
 *   Ball:    +0x244 Sphere, +0x248 SphereBreak1, +0x24C SphereBreak2
 *            +0x260 RBGlare, +0x264 Sphere+Tar
 *   Hamster: +0x250 Hamster-Waiting, +0x254 Hamster-trot1
 *            +600 Hamster-trot2, +0x25C Hamster-trot3
 *   Balls:   +0x268 8Ball, +0x26C FunBall, +0x270 Bell, +0x274 Dizzy
 *   Hazards: +0x5A4 tarbubble, +0x5A8 fanblades, +0x5AC fanbody
 *            +0x5B0 sawblade, +0x5B4 sawface, +0x5B8 sawface2
 *            +0x5BC dawgshoe, +0x5C0 dawgshoe2, +0x5C4 dawgshadow
 *            +0x578 mousetrapshadow
 *   Bonus:   +0x584 GlassBonus, +0x588 GlassBonus-Smashed
 *
 * === LEVEL ASSETS ===
 *   +0x570 Levels\MouseTrap,    +0x57C Levels\Secret
 *   +0x580 Levels\Secret-Unlock
 *   +0x574 collision from MouseTrap
 *   +0x594 Levels\Level4-Trapdoor1, +0x598 Levels\Level4-Trapdoor2
 *   +0x59C/0x5A0 collision from trapdoors
 *   +0x58C Levels\PopupSign, +0x590 collision from PopupSign
 *   +0x5C8 Levels\Level6-Lifter
 *
 * === TOURNAMENT ICONS ===
 *   +0x3B4 tourney-beginner.png, +0x3B8 tourney-cascade.png
 *   +0x3BC tourney-intermediate.png, +0x3C0 tourney-dizzy.png
 *   +0x3C4 tourney-Tower.png, +0x3C8 tourney-Up.png
 *   +0x3CC tourney-Neon.png, +0x3D0 tourney-Expert.png
 *   +0x3D4 tourney-Odd.png, +0x3D8 tourney-Toob.png
 *   +0x3DC tourney-Wobbly.png, +0x3E0 tourney-Glass.png
 *   +0x3E4 tourney-Sky.png, +1000 tourney-Master.png
 *   +0x3EC tourney-Impossible.png
 *   +0x3F4 tournament.png, +0x3F8 timetrials.png
 *   +0x430 partyrace.png, +0x434 rodentrumble.png
 *   +0x3FC mirror.png
 *
 * === GAME MODE UI ===
 *   +0x400 Burst.png, +0x414 Lost.png
 *   +0x404 Winner.png, +0x408 Winner2p.png
 *   +0x418 title1.png, +0x41C title2.png, +0x420 title3.png, +0x424 title4.png
 *   +0x42C unlock.png, +0x438 scoreball.png
 *   +0x32C weaselbox.png, +0x530 scrollmore.png
 *   +0x398 settext.png, +0x39C gotext.png
 *
 * === SOUND ASSETS (with max channel counts) ===
 *   Ball physics:  collide(10), roll(10), bumper(10), ballbreak(5), ballbreaksmall(5)
 *   Environment:   whistle(1), thwomp(2), snap(2), popup(2), crack(2), crumble(2)
 *   Level events:  dropin(2), dropinshort(2), popout(2), pipebump1-3(10ea)
 *                  gearclank(20), bridgeslam(2), platformtick(5), creakyplatform(20)
 *                  trapdoor(2), catapult(2), fwing(2), unlock(1), breakbridge(1)
 *   Hazards:       gluestuck(5), bubble1-2(5ea), wheelcreak(2), chomp(1)
 *                  sawstartup(2), sawcut(2), saw(2), sawspeedy(2)
 *                  fan-start(10), fan-blow(10), sizzle(2), explode(3)
 *                  vac-o-sux(3), speedcylinder(2), dawgstep1-2(10ea), dawgsmash(10)
 *   Bonus/UI:      clink(3), whoosh(3), minipop(5), bell(3), zip(2), ting(20)
 *                  shrink(3), grow(3), tweet(3), wubba(5), bonuspop(5), buzzbonus(1)
 *                  NeonRide(1), NeonFlicker(50), ZoopDown(2), LightsOff(2)
 *                  GlassBonus(2)
 *
 * App+0x22C = LoadingScreenGadget pointer
 * App+0x200 = registered flag (set to 1 multiple times as load milestones)
 * Final: Menu_MergeAllLists, Scene_AddObject
 */

/*
 * CreatePlatformOrStands (0x413490) - Secondary factory
 *
 * PLATFORM (prefix match, 8 chars):
 *   - Platform_ctor(0x10FC bytes) with pos from param_4
 *   - Uses mesh reference from Scene+0x438C
 *   - Registered: Scene+0x2578, Scene+0x4394
 *
 * STANDS (prefix match, 6 chars):
 *   - Stands_ctor(0x10D0 bytes) with mesh from Scene+0x4390
 *   - Position via Timer-based animation setup
 *   - Timer_Init -> set pos from param_4 -> Timer animate -> Timer_Cleanup
 */