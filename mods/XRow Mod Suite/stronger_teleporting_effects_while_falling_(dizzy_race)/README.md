# "stronger teleporting effects while falling (dizzy race)"

**CT Entry ID:** 298

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+D572:
db 90 90 90 90 90 90


[DISABLE]
Hamsterball.exe+D572:
db 0F 85 F4 00 00 00

```
