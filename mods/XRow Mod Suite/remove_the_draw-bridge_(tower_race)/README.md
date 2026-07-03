# "remove the draw-bridge (tower race)"

**CT Entry ID:** 202

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF988:
db 90 90 90 90 90
Hamsterball.exe+CF9D6:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CF988:
db 44 52 41 57 42
Hamsterball.exe+CF9D6:
db 4F 50 45 4E 53

```
