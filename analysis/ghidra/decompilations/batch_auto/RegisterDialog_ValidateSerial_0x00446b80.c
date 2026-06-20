/*
 * Function: RegisterDialog_ValidateSerial
 * Address: 0x00446B80
 * Signature: void __fastcall RegisterDialog_ValidateSerial(int *param_1)
 * Parameters:
 *   param_1: RegisterDialog* this (as int*) — dialog containing serial input
 *
 * Description:
 * Validates the user-entered serial key for game registration. Steps:
 *   1. Generates 128 random bytes via RNG_Rand into two buffers (local_10c, local_20c)
 *   2. Copies serial key from dialog (+0x449 or *+0x449 if len >= 0x10, SSO)
 *   3. Copies affiliate key from dialog (+0x450 or *+0x450 if len >= 0x10, SSO)
 *   4. Calls ESellerate_Init with App (param_1[0x21E])
 *   5. Calls LicenseKey_Validate(affiliate_key, serial_key, NULL, "54138")
 *   6. Checks strnicmp("HAMSTER", affiliate_key, 7) == 0 AND validation succeeded
 *   7. If valid: creates OkayDialog "THANKS FOR BUYING HAMSTERBALL!" (0x8A8 bytes)
 *   8. If invalid: creates error dialog
 *
 * Uses SSO (Small String Optimization) pattern: strings <= 15 chars stored inline.
 *
 * Struct offsets:
 *   param_1[0x21E] (offset 0x878): App pointer
 *   param_1[0x449] (offset 0x1124): serial key (SSO string, capacity at +0x44E)
 *   param_1[0x450] (offset 0x1140): affiliate key (SSO string, capacity at +0x455)
 *
 * Cross-references:
 *   Referenced from vtables at 0x4D6834, 0x4D6964, 0x4D69BC, 0x4D6A14 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
