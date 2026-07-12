# "Camera Angle change"

**CT Entry ID:** 339

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+1A270:
  db C4 29

[DISABLE]
Hamsterball.exe+1A270:
  db BC 29

```
