# "Camera Angle change 3"

**CT Entry ID:** 341

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+1A27B:
  db 41 DB 03 00  // cos
Hamsterball.exe+1A290:
  db 2C DB 03 00  // cos
Hamsterball.exe+1A270:
  db BC 29

[DISABLE]
Hamsterball.exe+1A27B:
  db 41 DB 03 00
Hamsterball.exe+1A290:
  db 0C DB 03 00
Hamsterball.exe+1A270:
  db BC 29

```
