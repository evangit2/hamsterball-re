# "Strong mousetraps (x2.5 speed)"

**CT Entry ID:** 234

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF370:
db 00 00 48 42 CD


[DISABLE]
Hamsterball.exe+CF370:
db 00 00 A0 41 CD

```
