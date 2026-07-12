# "Torrential mousetraps (x10 speed)"

**CT Entry ID:** 236

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF370:
db 00 00 48 43 CD


[DISABLE]
Hamsterball.exe+CF370:
db 00 00 A0 41 CD

```
