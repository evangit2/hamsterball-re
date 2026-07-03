# "remove mousetraps"

**CT Entry ID:** 157

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF754:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CF754:
db 4D 4F 55 53 45

```
