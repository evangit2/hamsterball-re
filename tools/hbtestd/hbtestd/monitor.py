"""Address monitoring and value freezing for the live game.

Provides:
  - AddressMonitor: time-series sampling of memory addresses
  - FreezeManager: continuously pin a value at an address (Cheat Engine freeze)
"""
from __future__ import annotations

import struct
import threading
import time
from collections import deque
from typing import Any, Optional

from .memory import MemoryManager


class AddressMonitor:
    """Sample memory addresses at regular intervals and keep history."""

    def __init__(self, mm: MemoryManager, interval: float = 0.25, maxlen: int = 120):
        self.mm = mm
        self.interval = interval
        self.maxlen = maxlen
        self._watches: dict[str, dict[str, Any]] = {}
        self._history: dict[str, deque] = {}
        self._thread: Optional[threading.Thread] = None
        self._stop = threading.Event()

    def add_watch(self, name: str, address: int, data_type: str = "u32") -> None:
        """Add an address to monitor."""
        self._watches[name] = {
            "address": address,
            "type": data_type,
            "added_at": time.time(),
        }
        if name not in self._history:
            self._history[name] = deque(maxlen=self.maxlen)

    def remove_watch(self, name: str) -> bool:
        """Remove a watched address."""
        if name in self._watches:
            del self._watches[name]
            self._history.pop(name, None)
            return True
        return False

    def list_watches(self) -> list[dict[str, Any]]:
        """Return list of active watches."""
        return [
            {
                "name": name,
                "address": f"0x{w['address']:08X}",
                "type": w["type"],
                "samples": len(self._history.get(name, [])),
            }
            for name, w in self._watches.items()
        ]

    def get_history(self, name: str, count: int = 50) -> list[dict[str, Any]]:
        """Return time-series for a watched address."""
        if name not in self._history:
            return []
        items = list(self._history[name])
        if count:
            items = items[-count:]
        return items

    def sample_once(self) -> dict[str, dict[str, Any]]:
        """Take a single reading of all watched addresses."""
        snapshot = {}
        for name, watch in self._watches.items():
            try:
                value = self.mm.read_typed(watch["address"], watch["type"])
                entry = {"t": time.time(), "value": value, "ok": True}
                snapshot[name] = entry
                self._history[name].append(entry)
            except Exception as e:
                entry = {"t": time.time(), "error": str(e), "ok": False}
                snapshot[name] = entry
                self._history[name].append(entry)
        return snapshot

    def start(self) -> None:
        """Start background sampling thread."""
        if self._thread and self._thread.is_alive():
            return
        self._stop.clear()
        self._thread = threading.Thread(target=self._run, daemon=True)
        self._thread.start()

    def stop(self) -> None:
        """Stop background sampling."""
        self._stop.set()
        if self._thread:
            self._thread.join(timeout=2.0)
            self._thread = None

    def _run(self) -> None:
        while not self._stop.is_set():
            try:
                self.sample_once()
            except Exception:
                pass
            self._stop.wait(self.interval)

    def is_running(self) -> bool:
        return self._thread is not None and self._thread.is_alive()


class FreezeManager:
    """Continuously write a value to an address to 'freeze' it.

    This runs a background thread that re-writes the value every few ms,
    preventing the game from changing it.
    """

    def __init__(self, mm: MemoryManager, interval_ms: int = 20):
        self.mm = mm
        self.interval = interval_ms / 1000.0
        self._frozen: dict[str, dict[str, Any]] = {}
        self._thread: Optional[threading.Thread] = None
        self._stop = threading.Event()

    def freeze(self, name: str, address: int, value: Any, data_type: str = "u32") -> None:
        """Start freezing an address to a specific value."""
        self._frozen[name] = {
            "address": address,
            "value": value,
            "type": data_type,
            "frozen_at": time.time(),
            "write_count": 0,
        }
        self._ensure_running()

    def unfreeze(self, name: str) -> bool:
        """Stop freezing an address."""
        if name in self._frozen:
            del self._frozen[name]
            return True
        return False

    def unfreeze_all(self) -> None:
        """Unfreeze all addresses."""
        self._frozen.clear()

    def list_frozen(self) -> list[dict[str, Any]]:
        """Return list of frozen addresses."""
        return [
            {
                "name": name,
                "address": f"0x{f['address']:08X}",
                "value": f["value"],
                "type": f["type"],
                "frozen_at": f["frozen_at"],
                "write_count": f["write_count"],
            }
            for name, f in self._frozen.items()
        ]

    def is_frozen(self, name: str) -> bool:
        return name in self._frozen

    def _ensure_running(self) -> None:
        if self._thread and self._thread.is_alive():
            return
        self._stop.clear()
        self._thread = threading.Thread(target=self._run, daemon=True)
        self._thread.start()

    def _run(self) -> None:
        while not self._stop.is_set():
            if not self._frozen:
                self._stop.wait(0.5)
                continue
            for name, spec in list(self._frozen.items()):
                try:
                    self.mm.write_typed(spec["address"], spec["value"], spec["type"])
                    spec["write_count"] += 1
                except Exception:
                    pass
            self._stop.wait(self.interval)

    def stop(self) -> None:
        """Stop the freeze thread and unfreeze all."""
        self._stop.set()
        self._frozen.clear()
        if self._thread:
            self._thread.join(timeout=2.0)
            self._thread = None

    def is_running(self) -> bool:
        return self._thread is not None and self._thread.is_alive()
