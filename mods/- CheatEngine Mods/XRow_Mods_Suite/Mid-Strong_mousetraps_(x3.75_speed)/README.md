# "Mid-Strong mousetraps (x3.75 speed)"

**CT Entry ID:** 292

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF370:
db 00 00 8C 42 CD


[DISABLE]
Hamsterball.exe+CF370:
db 00 00 A0 41 CD

```
