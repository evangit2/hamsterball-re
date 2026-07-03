# "despawn hammer (expert race and master race)"

**CT Entry ID:** 218

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CFABA:
db 90 90
Hamsterball.exe+CFAC8:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CFAC8:
db 45 3A 43 41 4C
Hamsterball.exe+CFABA:
db 48 41

```
