/*
 * Hamsterball Menu/UI System
 * 
 * Menu hierarchy: MainMenu -> SimpleMenu -> UIList (items/spacers)
 * Each menu is a Gadget subclass with rendering and input handling.
 *
 * Class hierarchy:
 *   Gadget -> SimpleMenu -> MainMenu / OptionsMenu / HighScoreMenu / CreditsMenu
 *   Gadget -> LoadingScreenGadget (inter-level loading screen)
 *
 * UIList at offset 0x88c holds UIListItem pointers (0x444 bytes each)
 * AthenaList at offset 0x44c holds AthenaString references
 *
 * Key offsets in SimpleMenu (inherits Gadget at ~0x864+):
 *   +0x868  title (char*)
 *   +0x88c  UIList items (AthenaList of UIListItem*)
 *   +0x44c  UIList strings (AthenaList of AthenaString)
 *   +0xCAC  min_width (int)
 *   +0xCB0  max_width (int)
 *   +0xCB4  total_height (int)
 *   +0xCBC  needs_layout (bool)
 *
 * UIListItem struct (0x444 bytes):
 *   +0x00   display_text (char*) - e.g. "LET'S PLAY!"
 *   +0x04   id_code (char*) - e.g. "PLAY"
 *   +0x0C   param1 (undefined4)
 *   +0x10   param2 (undefined4)
 *   +0x14   param3 (undefined4)
 *   +0x18   param4 (undefined4)
 *   +0x1C   label (AthenaString*)
 *   +0x20   parent (int) - linked list parent
 *   +0x24   height (int) - calculated from font + parent
 *
 * MainMenu items (0x42DE50 - MainMenu_ctor):
 *   "LET'S PLAY!"       -> "PLAY"   (scaled 0.75/1.0/0.75 - large)
 *   spacer(10)
 *   "HIGH SCORES"       -> "HS"     (1.0/1.0/1.0 - normal)
 *   "OPTIONS"           -> "OP"     (normal)
 *   "CREDITS"           -> "CR"     (normal)
 *   "REGISTER GAME"     -> "RG"     (only if !registered, normal + y-scale 0)
 *   spacer(10)
 *   [multiplayer item]  -> "MG"     (if registered + has MP data, 40x316)
 *   spacer(10)
 *   "EXIT TO DESKTOP"   -> "EXIT"   (0.75/1.0/1.0 - smaller)
 *
 * vtable: PTR_MPMenu_DeletingDtor_004d3f30
 *
 * UIList_AddItem (0x4497F0):
 *   - Allocates UIListItem (0x444 bytes)
 *   - Copies display_text and id_code strings
 *   - Creates AthenaString for label
 *   - Appends to both AthenaList at +0x44c and +0x88c
 *   - Calculates height from Font_MeasureText + parent offset
 *   - Accumulates total_height at +0xCB4
 *   - Updates max_width at +0xCB0 from text width
 *   - Sets needs_layout flag at +0xCBC
 *   - Resets transform matrix to identity
 *
 * OptionsMenu_RenderControls (0x42E910):
 *   - Iterates 4 control bindings at App+0xB28..0xB38
 *   - Values 99/100 = keyboard/mouse (rendered with x=0.5 scale)
 *   - Duplicate bindings shown in red (scale 0,1,0,1)
 *   - Uses UIList_SetColorsByName per "CONTROL%d"
 *
 * LoadingScreenGadget_Render (0x42DA60):
 *   - Full-screen loading screen between levels
 *   - Draws background sprite at (100,0,600,600)
 *   - If registered: draws 2 preview sprites at (130,40,256) and (386,43,256)
 *   - Progress bar: Gfx_DrawQuadRandomColor with height scaled by progress
 *   - When loaded: "CLICK HERE TO PLAY!" or "CLICK HERE TO EXIT" (demo)
 *   - Before loaded: LoadingScreen_DrawSwirl animation at (400,450)
 *   - Demo nag: "LOADING TOO SLOWLY?" / "THE MORE YOU PLAY THE DEMO..." 
 *     (shown after 20+ seconds, every 5 frames)
 *   - Free plays counter: "You have %d free plays remaining!"
 *   - Tip text carousel at bottom of screen
 *   - Sprite-based decorative elements drawn via AthenaList iteration
 */