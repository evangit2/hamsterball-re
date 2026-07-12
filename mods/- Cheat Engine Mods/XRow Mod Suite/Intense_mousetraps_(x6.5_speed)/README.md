# "Intense mousetraps (x6.5 speed)"

**CT Entry ID:** 149

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF370:
db 00 00 02 43 CD


[DISABLE]
Hamsterball.exe+CF370:
db 00 00 A0 41 CD

```
