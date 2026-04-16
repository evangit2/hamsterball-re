---
name: binary-format-string-scan
description: Parse unknown binary formats by scanning for length-prefixed strings first, then building structure around them. More reliable than sequential parsing when object sizes vary.
tags: [reverse-engineering, binary, parsing]
---

# Binary Format String-Scan Technique

## When to Use
When reverse-engineering a binary file format where:
- Objects have variable-length data that depends on type
- You can't determine object boundaries from structure alone
- The format uses length-prefixed strings ([uint32 length][string bytes])

## The Problem
Sequential parsing fails when you don't know the size of each object's data block.
Different object types have different data sizes, and there's no explicit size field.
Reading "the next N bytes as data" after a type string will misalign on the next object.

## The Solution: Two-Pass String Scan

### Pass 1: Collect all strings
```python
strings = []
pos = 0
while pos < len(data) - 4:
    slen = struct.unpack('<I', data[pos:pos+4])[0]
    if 2 <= slen <= 60 and pos + 4 + slen <= len(data):
        candidate = data[pos+4:pos+4+slen]
        s = candidate.split(b'\0')[0].decode('ascii', errors='replace')
        if is_recognized_type(s) or is_texture_filename(s):
            strings.append({
                'offset': pos,
                'str_len': slen,
                'content': s,
                'is_type': is_type_string(s),
                'is_texture': is_texture_filename(s)
            })
            pos += 4 + slen  # skip past string
            continue
    pos += 1
```

### Pass 2: Build objects from string adjacency
```python
objects = []
for i, s in enumerate(strings):
    if not s['is_type']:
        continue  # texture strings belong to previous object
    
    obj = create_object(type=s['content'])
    data_start = s['offset'] + 4 + s['str_len']
    
    # Read known-offset fields (position, rotation)
    obj.position = read_vec3(data, data_start)
    
    # If next string is a texture, it belongs to this object
    if i+1 < len(strings) and strings[i+1]['is_texture']:
        obj.texture = strings[i+1]['content']
        # Data between position and texture is transform/material data
        if data_start + 28 < strings[i+1]['offset']:
            obj.transform = read_transform(data, data_start + 28)
    
    objects.append(obj)
```

## Key Insights

1. **Length-prefixed strings are reliable anchors** — they create parseable boundaries in an otherwise opaque binary blob

2. **Don't try sequential parsing** — if you hit a string you can't classify, you'll lose alignment and every subsequent parse will be wrong

3. **Classify strings into categories** — "type strings" (START, FLAG, PLATFORM, etc.) create new objects; "texture strings" (.png, .bmp) are properties of the previous object

4. **Use inter-string gaps to infer structure** — the bytes between a type string and the next string contain position, transform, material, and index data. The size of this gap tells you how much data each type carries

5. **Verify with known test cases** — parse the smallest file first (Arena-SpawnPlatform = 1476 bytes) where you can manually verify every byte

6. **Handle null-padding** — type strings may be null-padded to their declared length. Always split on `\0` and trim trailing nulls

7. **Float vs int ambiguity** — the same 4 bytes can be read as float (1.0 = 0x3F800000) or uint32 (1). Look for 0x3F800000 patterns to identify identity transforms and float fields

## Verification Checklist
- [ ] Parser finds all objects in smallest test file
- [ ] Position values are reasonable (world-space coordinates)
- [ ] Texture filenames are correctly extracted
- [ ] Object counts per file match expected level complexity
- [ ] No crashes on edge cases (empty files, objects at end-of-file)
- [ ] All 86 level files parse without errors