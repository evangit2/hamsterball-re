# "Upside Down Camera"

**CT Entry ID:** 3

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF3F0:
db 00 00 40 40 00 00 00 00 C3

[DISABLE]
Hamsterball.exe+CF3F0:
db 00 00 00 3F 00 00 00 00 C3

```
