# "remove 8balls/badballs"

**CT Entry ID:** 155

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF74C:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CF74C:
db 42 41 44 42 41

```
