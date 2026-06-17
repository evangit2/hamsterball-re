# 02 - Binary Reconnaissance

## Initial Commands (any binary)

```bash
file target.exe
strings target.exe | head -50
strings target.exe | grep -iE '\.(dll|so|dylib)'
strings target.exe | grep -E '^[A-Za-z_][A-Za-z0-9_]*$' | sort | uniq -c | sort -rn | head
python3 - <<'PY'
import pefile, hashlib
with open('target.exe','rb') as f: data = f.read()
print('md5', hashlib.md5(data).hexdigest())
print('sha256', hashlib.sha256(data).hexdigest())
pe = pefile.PE('target.exe')
print('imagebase', hex(pe.OPTIONAL_HEADER.ImageBase))
print('entrypoint', hex(pe.OPTIONAL_HEADER.AddressOfEntryPoint))
print('imports')
for entry in pe.DIRECTORY_ENTRY_IMPORT:
    print(' ', entry.dll.decode(), len(entry.imports))
PY
```

## What to Look For

- **Import table:** reveals graphics/audio/input/network libraries.
- **String table:** filenames, URLs, error messages, debug paths, class names.
- **Sections:** large `.text`, `.rdata`, `.data`; unusual names may indicate packing.
- **Entropy:** packed/encrypted sections have entropy near 8.
- **Exports:** DLL interfaces, COM classes, plugin APIs.

## Red Flags

| Sign | Meaning |
|------|---------|
| High entropy `.text` | Packed or encrypted code |
| Missing normal imports | Custom loader or syscall usage |
| TLS callback | Anti-debug / early execution |
| Resources with high entropy | Embedded payload |
