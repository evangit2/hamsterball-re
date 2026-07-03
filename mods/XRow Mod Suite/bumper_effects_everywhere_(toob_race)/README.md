# "bumper effects everywhere (toob race)"

**CT Entry ID:** 289

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+10472:
db 90 90 90 90 90 90


[DISABLE]
Hamsterball.exe+10472:
db 0F 85 C9 01 00 00

```
