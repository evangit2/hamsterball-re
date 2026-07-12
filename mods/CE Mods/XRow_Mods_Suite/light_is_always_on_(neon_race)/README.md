# "light is always on (neon race)"

**CT Entry ID:** 178

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+D010E:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+D010E:
db 4C 49 47 48 54

```
