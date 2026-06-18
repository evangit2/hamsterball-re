"""Read/write/search live game process memory on Linux.

This module uses Linux-specific interfaces:
  - process_vm_readv/process_vm_writev for bulk transfers
  - ptrace(PTRACE_POKEDATA) for small writes when process_vm_writev fails
  - /proc/<pid>/maps to resolve module bases and readable/writable regions

Windows support can be added later with ReadProcessMemory/WriteProcessMemory.
"""
from __future__ import annotations

import ctypes
import os
import re
import struct
import time
from typing import Any, Optional

libc = ctypes.CDLL("libc.so.6", use_errno=True)

# iovec for process_vm_readv/writev
class iovec(ctypes.Structure):
    _fields_ = [
        ("iov_base", ctypes.c_void_p),
        ("iov_len", ctypes.c_size_t),
    ]


libc.process_vm_readv.restype = ctypes.c_ssize_t
libc.process_vm_readv.argtypes = [
    ctypes.c_int,
    ctypes.POINTER(iovec),
    ctypes.c_ulong,
    ctypes.POINTER(iovec),
    ctypes.c_ulong,
    ctypes.c_ulong,
]

libc.process_vm_writev.restype = ctypes.c_ssize_t
libc.process_vm_writev.argtypes = [
    ctypes.c_int,
    ctypes.POINTER(iovec),
    ctypes.c_ulong,
    ctypes.POINTER(iovec),
    ctypes.c_ulong,
    ctypes.c_ulong,
]

PTRACE_POKEDATA = 5
PTRACE_ATTACH = 16
PTRACE_DETACH = 17

libc.ptrace.restype = ctypes.c_long
libc.ptrace.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_void_p, ctypes.c_void_p]
libc.waitpid.restype = ctypes.c_int
libc.waitpid.argtypes = [ctypes.c_int, ctypes.POINTER(ctypes.c_int), ctypes.c_int]

# Type format chars for struct module
_TYPE_FORMATS = {
    "u8":   ("B", 1),
    "i8":   ("b", 1),
    "u16":  ("H", 2),
    "i16":  ("h", 2),
    "u32":  ("I", 4),
    "i32":  ("i", 4),
    "u64":  ("Q", 8),
    "i64":  ("q", 8),
    "float": ("f", 4),
    "double": ("d", 8),
}


class MemoryManager:
    def __init__(self, pid: int):
        self.pid = pid
        self._module_cache: dict[str, int] = {}

    # ------------------------------------------------------------------
    # Low-level read/write primitives
    # ------------------------------------------------------------------
    def read_bytes(self, address: int, size: int) -> bytes:
        """Read raw bytes from the target process."""
        if size <= 0:
            return b""
        buf = ctypes.create_string_buffer(size)
        local = iovec(ctypes.cast(buf, ctypes.c_void_p).value, size)
        remote = iovec(address, size)
        n = libc.process_vm_readv(self.pid, ctypes.byref(local), 1, ctypes.byref(remote), 1, 0)
        if n < 0:
            errno = ctypes.get_errno()
            raise OSError(errno, os.strerror(errno))
        return buf.raw[:n]

    def write_bytes(self, address: int, data: bytes) -> int:
        """Write raw bytes into the target process.

        Tries process_vm_writev first, falls back to ptrace POKEDATA for
        small aligned writes.
        """
        size = len(data)
        if size == 0:
            return 0

        buf = ctypes.create_string_buffer(data)
        local = iovec(ctypes.cast(buf, ctypes.c_void_p).value, size)
        remote = iovec(address, size)
        n = libc.process_vm_writev(self.pid, ctypes.byref(local), 1, ctypes.byref(remote), 1, 0)
        if n >= 0:
            return n

        # Fallback to ptrace for small writes (common for 4-byte ints/floats)
        if size <= 4 and address % 4 == 0:
            return self._write_bytes_ptrace(address, data)

        errno = ctypes.get_errno()
        raise OSError(errno, os.strerror(errno))

    def _write_bytes_ptrace(self, address: int, data: bytes) -> int:
        """Use ptrace POKEDATA to write up to 4 bytes at an aligned address."""
        if len(data) > 4:
            raise ValueError("ptrace fallback only supports up to 4 bytes")
        if address % 4 != 0:
            raise ValueError("ptrace fallback requires 4-byte aligned address")

        # Combine new bytes with existing word for partial writes
        existing = self.read_bytes(address, 4)
        word = bytearray(existing)
        for i, b in enumerate(data):
            word[i] = b
        value = struct.unpack("<I", bytes(word))[0]

        attached = False
        status = ctypes.c_int(0)
        if libc.ptrace(PTRACE_ATTACH, self.pid, None, None) == 0:
            attached = True
            libc.waitpid(self.pid, ctypes.byref(status), 0)
        try:
            if libc.ptrace(PTRACE_POKEDATA, self.pid, ctypes.c_void_p(address), ctypes.c_void_p(value)) != 0:
                errno = ctypes.get_errno()
                raise OSError(errno, os.strerror(errno))
        finally:
            if attached:
                libc.ptrace(PTRACE_DETACH, self.pid, None, None)
        return len(data)

    # ------------------------------------------------------------------
    # Typed read helpers
    # ------------------------------------------------------------------
    def read_u32(self, address: int) -> int:
        return struct.unpack("<I", self.read_bytes(address, 4))[0]

    def read_u16(self, address: int) -> int:
        return struct.unpack("<H", self.read_bytes(address, 2))[0]

    def read_u8(self, address: int) -> int:
        return self.read_bytes(address, 1)[0]

    def read_i32(self, address: int) -> int:
        return struct.unpack("<i", self.read_bytes(address, 4))[0]

    def read_i64(self, address: int) -> int:
        return struct.unpack("<q", self.read_bytes(address, 8))[0]

    def read_u64(self, address: int) -> int:
        return struct.unpack("<Q", self.read_bytes(address, 8))[0]

    def read_float(self, address: int) -> float:
        return struct.unpack("<f", self.read_bytes(address, 4))[0]

    def read_double(self, address: int) -> float:
        return struct.unpack("<d", self.read_bytes(address, 8))[0]

    def read_bool(self, address: int) -> bool:
        return self.read_bytes(address, 1)[0] != 0

    def read_string(self, address: int, max_length: int = 256, encoding: str = "utf-8") -> str:
        """Read a null-terminated string from memory."""
        raw = self.read_bytes(address, max_length)
        null_idx = raw.find(b"\x00")
        if null_idx >= 0:
            raw = raw[:null_idx]
        return raw.decode(encoding, errors="replace")

    def read_vec3(self, address: int) -> tuple[float, float, float]:
        """Read 3 consecutive floats (12 bytes) — a Vec3 position/velocity."""
        raw = self.read_bytes(address, 12)
        return struct.unpack("<fff", raw)

    def read_struct(self, address: int, offsets: list[tuple[str, int, str]]) -> dict[str, Any]:
        """Read multiple fields from a struct in one go.

        Args:
            address: base address of the struct
            offsets: list of (name, offset, type) tuples
        Returns:
            dict mapping name -> value
        """
        result = {}
        for name, offset, dtype in offsets:
            result[name] = self.read_typed(address + offset, dtype)
        return result

    # ------------------------------------------------------------------
    # Typed write helpers
    # ------------------------------------------------------------------
    def write_u32(self, address: int, value: int) -> int:
        return self.write_bytes(address, struct.pack("<I", value & 0xFFFFFFFF))

    def write_u16(self, address: int, value: int) -> int:
        return self.write_bytes(address, struct.pack("<H", value & 0xFFFF))

    def write_u8(self, address: int, value: int) -> int:
        return self.write_bytes(address, struct.pack("<B", value & 0xFF))

    def write_i32(self, address: int, value: int) -> int:
        return self.write_bytes(address, struct.pack("<i", value))

    def write_i64(self, address: int, value: int) -> int:
        return self.write_bytes(address, struct.pack("<q", value))

    def write_u64(self, address: int, value: int) -> int:
        return self.write_bytes(address, struct.pack("<Q", value))

    def write_float(self, address: int, value: float) -> int:
        return self.write_bytes(address, struct.pack("<f", value))

    def write_double(self, address: int, value: float) -> int:
        return self.write_bytes(address, struct.pack("<d", value))

    def write_bool(self, address: int, value: bool) -> int:
        return self.write_bytes(address, b"\x01" if value else b"\x00")

    def write_vec3(self, address: int, x: float, y: float, z: float) -> int:
        return self.write_bytes(address, struct.pack("<fff", x, y, z))

    def write_string(self, address: int, text: str, max_length: int = 256) -> int:
        """Write a null-terminated string, padding with zeros."""
        data = text.encode("utf-8")[: max_length - 1] + b"\x00"
        if len(data) < max_length:
            data += b"\x00" * (max_length - len(data))
        return self.write_bytes(address, data[:max_length])

    # ------------------------------------------------------------------
    # Generic typed read/write dispatcher
    # ------------------------------------------------------------------
    def read_typed(self, address: int, data_type: str) -> Any:
        """Read a typed value from memory by type name."""
        if data_type == "bytes":
            raise ValueError("use read_bytes for raw bytes")
        if data_type == "string":
            return self.read_string(address)
        if data_type == "bool":
            return self.read_bool(address)
        if data_type == "vec3":
            return self.read_vec3(address)
        if data_type == "cstr":
            return self.read_string(address)
        if data_type not in _TYPE_FORMATS:
            raise ValueError(f"unsupported type: {data_type}")
        fmt, size = _TYPE_FORMATS[data_type]
        return struct.unpack("<" + fmt, self.read_bytes(address, size))[0]

    def write_typed(self, address: int, value: Any, data_type: str) -> int:
        """Write a typed value to memory by type name."""
        if data_type == "bytes":
            data = bytes.fromhex(value) if isinstance(value, str) else bytes(value)
            return self.write_bytes(address, data)
        if data_type == "string":
            return self.write_string(address, str(value))
        if data_type == "bool":
            return self.write_bool(address, bool(value))
        if data_type == "vec3":
            if isinstance(value, (list, tuple)) and len(value) == 3:
                return self.write_vec3(address, float(value[0]), float(value[1]), float(value[2]))
            if isinstance(value, dict):
                return self.write_vec3(address, value["x"], value["y"], value["z"])
            raise ValueError("vec3 requires [x, y, z] or {x, y, z}")
        if data_type == "float" and isinstance(value, str):
            value = float(value)
        if data_type not in _TYPE_FORMATS:
            raise ValueError(f"unsupported type: {data_type}")
        fmt, size = _TYPE_FORMATS[data_type]
        if data_type.startswith("u") or data_type.startswith("i"):
            value = int(value)
        elif data_type in ("float", "double"):
            value = float(value)
        packed = struct.pack("<" + fmt, value)
        return self.write_bytes(address, packed)

    # ------------------------------------------------------------------
    # Pointer chain resolution (multi-level dereferencing)
    # ------------------------------------------------------------------
    def resolve_pointer_chain(
        self, base: int, offsets: list[int]
    ) -> int:
        """Follow a pointer chain: read base -> [base] -> [base + o1] -> ...

        This is the classic Cheat Engine style pointer chain:
            base_address -> [+offset] -> [+offset] -> ... -> final

        Args:
            base: starting absolute address
            offsets: list of offsets applied at each level
        Returns:
            final resolved address
        """
        addr = base
        for i, offset in enumerate(offsets):
            ptr = self.read_u32(addr)
            if ptr == 0:
                raise ValueError(f"null pointer at chain level {i} (base+{''.join(f'+0x{o:X}' for o in offsets[:i])})")
            addr = ptr + offset
        return addr

    def read_pointer(self, address: int) -> int:
        """Read a 32-bit pointer value."""
        return self.read_u32(address)

    def dereference(self, address: int, offset: int = 0) -> int:
        """Read a pointer and add an offset."""
        return self.read_pointer(address) + offset

    # ------------------------------------------------------------------
    # Batch operations
    # ------------------------------------------------------------------
    def read_batch(self, reads: list[dict]) -> list[dict]:
        """Read multiple addresses in one call.

        Each item: {"label": "ball_x", "address": "0x...", "type": "float"}
        Returns: [{"label": ..., "address": ..., "value": ..., "type": ...}, ...]
        """
        results = []
        for item in reads:
            label = item.get("label", "")
            addr = item["address"]
            dtype = item.get("type", "u32")
            try:
                addr_int = addr if isinstance(addr, int) else _parse_addr(addr)
                value = self.read_typed(addr_int, dtype)
                results.append({
                    "label": label,
                    "address": f"0x{addr_int:08X}",
                    "value": value,
                    "type": dtype,
                    "ok": True,
                })
            except Exception as e:
                results.append({
                    "label": label,
                    "address": str(addr),
                    "type": dtype,
                    "ok": False,
                    "error": str(e),
                })
        return results

    def write_batch(self, writes: list[dict]) -> list[dict]:
        """Write multiple addresses in one call.

        Each item: {"address": "0x...", "value": 42, "type": "u32"}
        Returns: status for each write
        """
        results = []
        for item in writes:
            addr = item["address"]
            value = item["value"]
            dtype = item.get("type", "u32")
            try:
                addr_int = addr if isinstance(addr, int) else _parse_addr(addr)
                n = self.write_typed(addr_int, value, dtype)
                results.append({
                    "address": f"0x{addr_int:08X}",
                    "bytes_written": n,
                    "value": value,
                    "type": dtype,
                    "ok": True,
                })
            except Exception as e:
                results.append({
                    "address": str(addr),
                    "value": value,
                    "type": dtype,
                    "ok": False,
                    "error": str(e),
                })
        return results

    # ------------------------------------------------------------------
    # Value scanning (Cheat Engine style)
    # ------------------------------------------------------------------
    def scan_value(
        self,
        value: Any,
        data_type: str = "u32",
        module_name: Optional[str] = None,
        start: Optional[int] = None,
        end: Optional[int] = None,
        max_results: int = 200,
    ) -> list[dict]:
        """Scan memory for all addresses containing a specific value.

        Args:
            value: the value to search for
            data_type: type to search as (u32, float, u16, u8, etc.)
            module_name: restrict to a specific module
            start/end: address range
            max_results: stop after this many hits
        Returns:
            list of {"address": int, "value": value} dicts
        """
        fmt, size = _TYPE_FORMATS.get(data_type, (None, None))
        if fmt is None:
            raise ValueError(f"unsupported type for scanning: {data_type}")
        if data_type in ("float", "double"):
            value = float(value)
        else:
            value = int(value)
        needle = struct.pack("<" + fmt, value)

        results = []
        regions = self.list_regions()
        if module_name:
            regions = [r for r in regions if r["path"].endswith(module_name)]
        if start is not None:
            regions = [r for r in regions if r["end"] > start]
        if end is not None:
            regions = [r for r in regions if r["start"] < end]

        for region in regions:
            rs = max(region["start"], start or 0)
            re_ = min(region["end"], end or region["end"])
            rsize = re_ - rs
            if rsize <= 0:
                continue
            # Read in chunks to avoid huge allocations
            chunk_size = min(rsize, 4 * 1024 * 1024)  # 4MB chunks
            for chunk_off in range(0, rsize, chunk_size):
                this_size = min(chunk_size, rsize - chunk_off)
                try:
                    data = self.read_bytes(rs + chunk_off, this_size)
                except OSError:
                    continue
                idx = 0
                while True:
                    idx = data.find(needle, idx)
                    if idx == -1:
                        break
                    results.append({
                        "address": rs + chunk_off + idx,
                        "value": value,
                    })
                    idx += size
                    if len(results) >= max_results:
                        return results
        return results

    def scan_float_range(
        self,
        low: float,
        high: float,
        module_name: Optional[str] = None,
        start: Optional[int] = None,
        end: Optional[int] = None,
        max_results: int = 200,
    ) -> list[dict]:
        """Scan for floats within a range [low, high]."""
        results = []
        regions = self.list_regions()
        if module_name:
            regions = [r for r in regions if r["path"].endswith(module_name)]
        if start is not None:
            regions = [r for r in regions if r["end"] > start]
        if end is not None:
            regions = [r for r in regions if r["start"] < end]

        for region in regions:
            rs = max(region["start"], start or 0)
            re_ = min(region["end"], end or region["end"])
            rsize = re_ - rs
            if rsize < 4:
                continue
            chunk_size = min(rsize, 4 * 1024 * 1024)
            for chunk_off in range(0, rsize - 3, chunk_size - 3):
                this_size = min(chunk_size, rsize - chunk_off)
                try:
                    data = self.read_bytes(rs + chunk_off, this_size)
                except OSError:
                    continue
                for i in range(0, len(data) - 3, 4):
                    val = struct.unpack("<f", data[i : i + 4])[0]
                    if low <= val <= high:
                        results.append({"address": rs + chunk_off + i, "value": val})
                        if len(results) >= max_results:
                            return results
        return results

    def scan_next(
        self,
        value: Any,
        data_type: str = "u32",
        prev_results: list[int] | None = None,
        max_results: int = 200,
    ) -> list[dict]:
        """Narrow down previous scan results by checking which still match.

        Args:
            value: new value to check
            data_type: type to check
            prev_results: list of addresses from previous scan
        Returns:
            list of addresses that now contain this value
        """
        fmt, size = _TYPE_FORMATS.get(data_type, (None, None))
        if fmt is None:
            raise ValueError(f"unsupported type: {data_type}")
        if data_type in ("float", "double"):
            value = float(value)
        else:
            value = int(value)
        needle = struct.pack("<" + fmt, value)

        results = []
        if prev_results is None:
            return self.scan_value(value, data_type, max_results=max_results)

        for addr in prev_results:
            try:
                data = self.read_bytes(addr, size if size else 4)
                if data == needle:
                    results.append({"address": addr, "value": value})
                    if len(results) >= max_results:
                        break
            except OSError:
                continue
        return results

    # ------------------------------------------------------------------
    # Disassembly hint (basic — shows raw bytes at an address)
    # ------------------------------------------------------------------
    def dump_hex(self, address: int, size: int = 64) -> dict[str, Any]:
        """Return a hex dump of memory at the given address."""
        raw = self.read_bytes(address, size)
        lines = []
        for i in range(0, len(raw), 16):
            chunk = raw[i : i + 16]
            hex_part = " ".join(f"{b:02X}" for b in chunk)
            ascii_part = "".join(chr(b) if 32 <= b < 127 else "." for b in chunk)
            lines.append(f"0x{address + i:08X}  {hex_part:<48s}  |{ascii_part}|")
        return {
            "address": f"0x{address:08X}",
            "size": len(raw),
            "hex": "\n".join(lines),
            "raw_hex": raw.hex(),
        }

    # ------------------------------------------------------------------
    # Module/region helpers
    # ------------------------------------------------------------------
    def list_regions(self) -> list[dict]:
        """Return readable memory regions from /proc/<pid>/maps."""
        path = f"/proc/{self.pid}/maps"
        regions = []
        pattern = re.compile(
            r"^([0-9a-f]+)-([0-9a-f]+)\s+([rwxsp-]{4})\s+([0-9a-f]+)\s+(\d+):(\d+)\s+(\d+)\s*(.*)$"
        )
        try:
            with open(path, "r") as f:
                for line in f:
                    m = pattern.match(line)
                    if not m:
                        continue
                    start = int(m.group(1), 16)
                    end = int(m.group(2), 16)
                    perms = m.group(3)
                    if "r" not in perms:
                        continue
                    regions.append(
                        {
                            "start": start,
                            "end": end,
                            "size": end - start,
                            "perms": perms,
                            "offset": int(m.group(4), 16),
                            "path": m.group(8).strip(),
                        }
                    )
        except FileNotFoundError:
            pass
        return regions

    def get_module_base(self, module_name: Optional[str] = None) -> Optional[int]:
        """Find the base address of the named module (default main executable)."""
        name = module_name or "Hamsterball.exe"
        if name in self._module_cache:
            return self._module_cache[name]
        for region in self.list_regions():
            path = region["path"]
            if path.endswith(name) and region["offset"] == 0:
                self._module_cache[name] = region["start"]
                return region["start"]
        return None

    def get_module_info(self, module_name: Optional[str] = None) -> Optional[dict]:
        """Return full module info (base, end, size, path)."""
        name = module_name or "Hamsterball.exe"
        regions = self.list_regions()
        for region in regions:
            if region["path"].endswith(name) and region["offset"] == 0:
                # Find the highest adjacent region for the same module
                end = region["end"]
                for r2 in regions:
                    if r2["path"] == region["path"] and r2["start"] >= region["start"]:
                        end = max(end, r2["end"])
                return {
                    "name": name,
                    "base": region["start"],
                    "end": end,
                    "size": end - region["start"],
                    "path": region["path"],
                    "regions": [r for r in regions if r["path"].endswith(name)],
                }
        return None

    def resolve_rva(self, rva: int, module_name: Optional[str] = None) -> Optional[int]:
        base = self.get_module_base(module_name)
        if base is None:
            return None
        return base + rva

    def resolve_address(self, spec: str) -> Optional[int]:
        """Resolve an address specification.

        Supports:
          - "0x005341E0" -> absolute address
          - "RVA:0x1341E0" -> base + RVA (Hamsterball.exe base)
          - "module:Ntdll.dll:0x5000" -> module base + offset
          - "g_App" -> known address from addresses.py (if available)
          - "g_App+0x16C" -> known address + offset
          - "ptr:0x005341E0" -> dereference pointer at address
          - "chain:0x005341E0,0x10,0x20" -> pointer chain
        """
        spec = spec.strip()

        # Pointer chain
        if spec.startswith("chain:"):
            parts = spec[6:].split(",")
            base = _parse_addr(parts[0].strip())
            offsets = [int(x.strip(), 0) for x in parts[1:]]
            return self.resolve_pointer_chain(base, offsets)

        # Dereference
        if spec.startswith("ptr:"):
            addr = _parse_addr(spec[4:].strip())
            return self.read_pointer(addr)

        # RVA relative to main module
        if spec.startswith("RVA:") or spec.startswith("rva:"):
            rva = int(spec[4:].strip(), 0)
            return self.resolve_rva(rva)

        # Module-specific
        if spec.startswith("module:"):
            parts = spec[7:].split(":")
            mod_name = parts[0]
            offset = int(parts[1], 0) if len(parts) > 1 else 0
            base = self.get_module_base(mod_name)
            return base + offset if base is not None else None

        # Known symbol (from addresses.py)
        if not spec.startswith(("0x", "0X")):
            try:
                from .addresses import resolve_symbol
                result = resolve_symbol(self, spec)
                if result is not None:
                    return result
            except (ImportError, Exception):
                pass

        # Absolute hex or decimal
        return _parse_addr(spec)

    # ------------------------------------------------------------------
    # Search
    # ------------------------------------------------------------------
    def find_pattern(
        self,
        pattern: bytes,
        module_name: Optional[str] = None,
        start: Optional[int] = None,
        end: Optional[int] = None,
        max_results: int = 100,
    ) -> list[int]:
        """Search for a byte pattern in readable regions.

        Pattern may contain '?' wildcards, e.g. b'\\x89\\x??\\x05'.
        """
        use_wildcard = b"?" in pattern
        results = []

        regions = self.list_regions()
        if module_name:
            regions = [r for r in regions if r["path"].endswith(module_name)]
        if start is not None:
            regions = [r for r in regions if r["end"] > start]
        if end is not None:
            regions = [r for r in regions if r["start"] < end]

        for region in regions:
            region_start = max(region["start"], start or 0)
            region_end = min(region["end"], end or region["end"])
            size = region_end - region_start
            if size <= 0:
                continue
            try:
                data = self.read_bytes(region_start, size)
            except OSError:
                continue

            if use_wildcard:
                results.extend(self._find_pattern_wildcard(data, pattern, region_start, max_results - len(results)))
            else:
                idx = 0
                while True:
                    idx = data.find(pattern, idx)
                    if idx == -1:
                        break
                    results.append(region_start + idx)
                    idx += 1
                    if len(results) >= max_results:
                        return results
        return results

    def _find_pattern_wildcard(
        self, data: bytes, pattern: bytes, base_address: int, max_results: int
    ) -> list[int]:
        results = []
        if not pattern:
            return results
        first_byte = pattern[0]
        is_wild_first = first_byte == ord("?")
        idx = 0
        while True:
            if is_wild_first:
                idx += 1
                if idx >= len(data):
                    break
            else:
                idx = data.find(bytes([first_byte]), idx)
                if idx == -1:
                    break
            if idx + len(pattern) > len(data):
                break
            if self._match_pattern(data, idx, pattern):
                results.append(base_address + idx)
                if len(results) >= max_results:
                    return results
            idx += 1
        return results

    def _match_pattern(self, data: bytes, offset: int, pattern: bytes) -> bool:
        for i, b in enumerate(pattern):
            if b == ord("?"):
                continue
            if data[offset + i] != b:
                return False
        return True


def _parse_addr(s: str) -> int:
    """Parse an address string (hex with 0x prefix or decimal)."""
    s = s.strip().replace("`", "")
    if s.lower().startswith("0x"):
        return int(s, 16)
    try:
        return int(s)
    except ValueError:
        return int(s, 16)
