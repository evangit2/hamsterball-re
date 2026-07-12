# "disable "GOAL""

**CT Entry ID:** 207

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF806:
db 90 90 90 90


[DISABLE]
Hamsterball.exe+CF806:
db 47 4F 41 4C

```
