# "remove glubies"

**CT Entry ID:** 152

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF680:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CF680:
db 47 4C 55 45 42

```
