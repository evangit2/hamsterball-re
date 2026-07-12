# "remove the trapdoor (tower race)"

**CT Entry ID:** 201

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF970:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CF970:
db 54 52 41 50 44

```
