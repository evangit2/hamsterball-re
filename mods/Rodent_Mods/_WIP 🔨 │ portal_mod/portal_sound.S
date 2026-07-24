/*
 * portal_sound.S - Sound_Play3D wrapper for MinGW (AT&T syntax)
 * 
 * void __cdecl portal_play_zip(DWORD soundChannel, float x, float y, float z)
 * 
 * Sound_Play3D at 0x459860 is __thiscall(ECX=soundChannel, x, y, z) with RET 0x10
 * We push 4 DWORDs (16 bytes) to match the RET 0x10 cleanup.
 */
.global _portal_play_zip

_portal_play_zip:
    /* Stack: [ESP+0]=return addr, [ESP+4]=soundChannel, [ESP+8]=x, [ESP+0xC]=y, [ESP+0x10]=z */
    /* Load soundChannel into ECX */
    movl 4(%esp), %ecx
    /* Push args in reverse order: z, y, x, padding */
    pushl 16(%esp)     /* z (was at ESP+0x10, now shifted by push) */
    pushl 16(%esp)     /* y (was at ESP+0xC, now at 0x10 after 1 push) */
    pushl 16(%esp)     /* x (was at ESP+8, now at 0x10 after 2 pushes) */
    pushl $0           /* padding (4th DWORD to match RET 0x10) */
    /* ECX already = soundChannel */
    movl $0x00459860, %eax    /* Sound_Play3D */
    call *%eax
    /* callee cleaned 16 bytes via RET 0x10 */
    ret
