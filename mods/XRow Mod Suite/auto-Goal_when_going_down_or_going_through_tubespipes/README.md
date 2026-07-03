# "auto-Goal when going down or going through tubes/pipes"

**CT Entry ID:** 274

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CEB6:
db 90 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CEB6:
db 0F 85 B2 01 00 00

```
