# "remove the wavy floor (wobbly race)"

**CT Entry ID:** 188

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CFC68:
db 90 90 90 90 90
Hamsterball.exe+CFC82:
db 90 90 90 90


[DISABLE]
Hamsterball.exe+CFC68:
db 57 41 56 59 31
Hamsterball.exe+CFC82:
db 57 41 56 59

```
