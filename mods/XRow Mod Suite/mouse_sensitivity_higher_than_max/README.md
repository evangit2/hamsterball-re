# "mouse sensitivity higher than max"

**CT Entry ID:** 318

**Script Type:** Code cave / complex

## Script

```
[ENABLE]
"Hamsterball.exe"+FDECC:
db 00 00 A0 40 01 01 01

[DISABLE]
"Hamsterball.exe"+FDECC:
db 00 00 80 3F 01 01

```
