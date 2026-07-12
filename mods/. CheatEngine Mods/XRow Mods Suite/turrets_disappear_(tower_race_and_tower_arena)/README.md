# "turrets disappear (tower race and tower arena)"

**CT Entry ID:** 247

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF960:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CF960:
db 54 55 52 52 45

```
