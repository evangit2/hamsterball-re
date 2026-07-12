# "hammer doesnt chase you (expert and master race)"

**CT Entry ID:** 151

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CFABA:
db 90 90


[DISABLE]
Hamsterball.exe+CFABA:
db 48 41

```
