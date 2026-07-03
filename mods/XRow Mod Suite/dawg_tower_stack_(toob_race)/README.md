# "dawg tower stack (toob race)"

**CT Entry ID:** 286

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+FF3E:
db 90 90 90 90 90 90


[DISABLE]
Hamsterball.exe+FF3E:
db 0F 85 A8 00 00 00

```
