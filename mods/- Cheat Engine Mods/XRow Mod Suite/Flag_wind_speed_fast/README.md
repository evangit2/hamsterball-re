# "Flag wind speed fast"

**CT Entry ID:** 233

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF55C:
db 00 00 20 42 D0


[DISABLE]
Hamsterball.exe+CF55C:
db 00 00 A0 40 D0

```
