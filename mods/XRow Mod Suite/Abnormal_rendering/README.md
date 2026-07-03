# "Abnormal rendering"

**CT Entry ID:** 342

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF39C:
  db 00 00 80 3E

[DISABLE]
Hamsterball.exe+CF39C:
  db 26 B4 17 3D  //

```
