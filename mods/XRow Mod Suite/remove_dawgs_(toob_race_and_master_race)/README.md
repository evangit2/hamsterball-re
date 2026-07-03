# "remove dawgs (toob race and master race)"

**CT Entry ID:** 184

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CFCC0:
db 90 90 90 90 90 90 90
Hamsterball.exe+CFCD8:
db 90 90 90 90 90 90 90
Hamsterball.exe+CFCF0:
db 90 90 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CFCC0:
db 42 4C 4F 43 4B 44 41
Hamsterball.exe+CFCD8:
db 42 4C 4F 43 4B 44 41
Hamsterball.exe+CFCF0:
db 42 4C 4F 43 4B 44 41

```
