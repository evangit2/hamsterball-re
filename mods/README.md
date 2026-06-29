# Hamsterball Mods Catalog

| Mod | Description | Files |
|-----|-------------|-------|
| universal-ref-loader | v3: Loads any ref type into any level via vtable[33] hook. JIT mesh injection from disk, clone-on-return for static-mesh objects, difficulty gate bypass, board slot safety checks | [universal-ref-loader/](universal-ref-loader/) |
| global-lifters | Spawns Up Race lifters (Rotators) on any level with a hotkey | [global-lifters/](global-lifters/) |
| global-drawbridge | Spawns Tower Race drawbridges on any level with a hotkey | [global-drawbridge/](global-drawbridge/) |
| global-trapdoor | Spawns Tower Race trapdoors on any level with a hotkey | [global-trapdoor/](global-trapdoor/) |
| global-neon | Spawns all 6 Neon Race objects on any level + neon lighting mode (dark scene with ball-emitted point light). Toggle via NeonLighting/NeonAmbient symbols | [global-neon/](global-neon/) |
| global-expert | Spawns all 6 Expert Race objects (Bonk/Hammer, Fan, Sawblade, Bridge, Judge, Bell) on any level with a hotkey | [global-expert/](global-expert/) |
| global-bonk | Spawns Bonk the Hammer from Expert Race on any level — standalone, constructor loads its own mesh internally | [global-bonk/](global-bonk/) |
| wall-bumpers | All walls act as pinball bumpers — velocity-reversal detection amplifies wall bounces. F8 toggle, F9 force cycle | [wall_bumpers/](wall_bumpers/) |
| ball-break | Press X to shatter your ball and respawn at nearest checkpoint. Calls the game's own Ball_Shatter function | [ball_break/](ball_break/) |
| half-size-all | Shrinks player ball to half size by inlining Ball_Shrink's physics fields (radius=13.0, physics_scale=2.5). No sound, player index 0 only | [half_size_all/](half_size_all/) |
| raptisoft-live-log | Passive logger — taps Raptisoft's hidden in-memory status tracking system (App+0x208/0x20C/0x210) and writes live updates to live_status.txt. No gameplay changes | [raptisoft_live_log/](raptisoft_live_log/) |
| level-colors | Changes per-level base colors (timer oval, timer text, race selection menu text) from a colors.txt config file. Edit at runtime | [level_colors/](level_colors/) |
| universal-safespots | Adds SAFESPOT(*) universal checkpoint — always accepted as respawn candidate regardless of active SAFESWITCH filter. Competes on distance with lettered SAFESPOTs | [universal-safespots/](universal-safespots/) |
| 8ball-spawn | Press B to spawn an 8-ball in front of the player's hamster ball. Uses mesh index 9 (8Ball), spawns as physics debris (player_index=-1) | [8ball_spawn/](8ball_spawn/) |
| 8ball-goal-fix | Prevents crash when 8-ball (BadBall) touches N:GOAL or E:ACTION(SCORE) triggers. Adds player_index<0 guard (same pattern Raptisoft used for E:LIMIT) to 3 patch points in DispatchCollisionEvents | [8ball_goal_fix/](8ball_goal_fix/) |
| ball-tint | v2: Tints player 1's ball by writing RGBA floats to the board's player color table (board+0x3AB0), the same entries Board_ctor initializes via Vec3_Init. Reads hex color from ball_tint.txt at runtime | [ball_tint/](ball_tint/) |
| global-objects-mkn | Spawns SWIRL (Dizzy Race) on any level via global object spawning hook. JIT mesh loading + collision registration | [global_objects_mkn/](global_objects_mkn/) |
