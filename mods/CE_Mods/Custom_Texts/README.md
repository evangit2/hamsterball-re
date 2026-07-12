# "Custom Texts"

**CT Entry ID:** 329

**Script Type:** Code cave / complex

## Script

```
{ Game   : Hamsterball.exe
  Version:
  Date   : 2026-05-30
  Author : XRow

  Custom Pause Button Text

  Use HEX values
}

[ENABLE]

// RESUME
"Hamsterball.exe"+D410C:
db 4B 61 6B 61 79 20 4B 61 6B 61 79 20 42 61 6B 61 00

// OPTIONS
"Hamsterball.exe"+D3F00:
db 4B 61 6B 61 79 20 42 61 6B 61 00

// QUIT THIS FIGHT
"Hamsterball.exe"+D4810:
db 4B 61 6B 61 79 20 42 61 6B 61 00

[DISABLE]

"Hamsterball.exe"+D410C:
db 52 45 53 55 4D 45 00  // "RESUME"

"Hamsterball.exe"+D3F00:
db 4F 50 54 49 4F 4E 53 00  // "OPTIONS"

"Hamsterball.exe"+D4810:
db 51 55 49 54 20 54 48 49 53 20 46 49 47 48 54 00  // "QUIT THIS FIGHT"

```
