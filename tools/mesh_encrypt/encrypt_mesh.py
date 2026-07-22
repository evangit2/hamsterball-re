#!/usr/bin/env python3
"""
XOR encryption tool for Hamsterball .MESH and .MESHWORLD files.

Usage:
    python3 encrypt_mesh.py <directory>

Encrypts ALL .mesh and .meshworld files in the given directory (recursive)
using single-byte XOR with key=119. Files are encrypted IN PLACE.

To decrypt, run the same tool again (XOR is symmetric).
"""
import os
import sys

KEY = 119  # 0x77

def xor_file(path):
    """XOR every byte of file with KEY. Symmetric — run again to reverse."""
    with open(path, 'rb') as f:
        data = bytearray(f.read())
    
    for i in range(len(data)):
        data[i] ^= KEY
    
    with open(path, 'wb') as f:
        f.write(data)
    
    return len(data)

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <directory>")
        print(f"XOR-encrypts all .mesh and .meshworld files with key={KEY}")
        sys.exit(1)
    
    directory = sys.argv[1]
    if not os.path.isdir(directory):
        print(f"Error: {directory} is not a directory")
        sys.exit(1)
    
    count = 0
    total_bytes = 0
    
    for root, dirs, files in os.walk(directory):
        for fname in files:
            lower = fname.lower()
            if lower.endswith('.mesh') or lower.endswith('.meshworld'):
                # Skip .cached and .bak files
                if lower.endswith('.cached') or lower.endswith('.bak'):
                    continue
                fpath = os.path.join(root, fname)
                nbytes = xor_file(fpath)
                count += 1
                total_bytes += nbytes
                print(f"  encrypted: {fname} ({nbytes:,} bytes)")
    
    print(f"\nDone: {count} files, {total_bytes:,} bytes total")
    print(f"XOR key: {KEY} (0x{KEY:02X})")

if __name__ == '__main__':
    main()
