# "draw-bridge doesn't open (tower race)"

**CT Entry ID:** 169

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF9D6:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CF9D6:
db 4F 50 45 4E 53

```
