# "Trained mousetraps (x1.5 speed)"

**CT Entry ID:** 245

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF370:
db 00 00 0C 42 CD CC


[DISABLE]
Hamsterball.exe+CF370:
db 00 00 A0 41 CD

```
