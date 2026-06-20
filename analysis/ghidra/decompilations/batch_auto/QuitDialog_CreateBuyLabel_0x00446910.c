/*
 * Function: QuitDialog_CreateBuyLabel
 * Address: 0x00446910
 * Signature: void __fastcall QuitDialog_CreateBuyLabel(int param_1)
 * Parameters:
 *   param_1: QuitDialog* this — the dialog creating the buy label
 *
 * Description:
 * Creates a "Buy Hamsterball" label widget for unregistered game copies.
 * Steps:
 *   1. Calls eSellerate_ReadAffiliateKey("PUB022254855","SKU5794335848", buf, 0x200)
 *   2. If key read fails (returns 0), copies fallback URL
 *      "http://www.raptisoft.com/buyhamsterball" into buffer (10 DWORDs = 40 chars)
 *   3. Allocates Gadget_Label (0x880 bytes) via operator_new
 *   4. Calls Gadget_LabelCtor with App (param_1+0x878), URL buffer, and duration=1000
 *   5. Adds label to scene via Scene_AddObject(App→+0x184)
 *
 * This function only appears in trial/unregistered builds, creating a buy link.
 *
 * Struct offsets:
 *   +0x878: App pointer (→+0x184: Scene object list)
 *
 * Cross-references:
 *   Called from 0x448870 — UNCONDITIONAL_CALL (QuitToDesktopDialog flow)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
