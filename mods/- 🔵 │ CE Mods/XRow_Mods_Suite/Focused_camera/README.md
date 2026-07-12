# "Focused camera"

**CT Entry ID:** 336

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+1A1F6:
  db 90 90 90 90

[DISABLE]
Hamsterball.exe+1A1F6:
  db 85 C0 74 5A

```
