# "ten-bonus glass dont break (glass race)"

**CT Entry ID:** 216

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+D016E:
db 90 90 90 90 90
Hamsterball.exe+D0182:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+D016E:
db 54 45 4E 42 4F
Hamsterball.exe+D0182:
db 54 45 4E 42 4F

```
