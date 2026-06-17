#!/usr/bin/env python3
"""Quick PE summary for any Windows binary."""
import sys, pefile

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <file.exe>")
        return 1
    path = sys.argv[1]
    pe = pefile.PE(path)
    print(f"Image base: 0x{pe.OPTIONAL_HEADER.ImageBase:08X}")
    print(f"Entry point RVA: 0x{pe.OPTIONAL_HEADER.AddressOfEntryPoint:08X}")
    print(f"Sections: {len(pe.sections)}")
    for s in pe.sections:
        print(f"  {s.Name.decode(errors='replace').strip(chr(0))}: 0x{s.VirtualAddress:08X} size 0x{s.Misc_VirtualSize:08X}")
    if hasattr(pe, "DIRECTORY_ENTRY_IMPORT"):
        print("Imports:")
        for entry in pe.DIRECTORY_ENTRY_IMPORT:
            print(f"  {entry.dll.decode()}: {len(entry.imports)} imports")
    return 0

if __name__ == "__main__":
    raise SystemExit(main()))
