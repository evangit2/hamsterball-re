"""Structured error helpers for hbtestd tools.

All tools should return a JSON-serializable dict with at least:
  {"success": bool, ...}

This module provides consistent error wrappers so the MCP layer stays clean.
"""
from __future__ import annotations

import asyncio
import traceback
from functools import wraps
from typing import Any, Callable


def success(**kwargs: Any) -> dict[str, Any]:
    kwargs["success"] = True
    return kwargs


def failure(error: str, **kwargs: Any) -> dict[str, Any]:
    kwargs["success"] = False
    kwargs["error"] = error
    return kwargs


def tool_guard(fn: Callable[..., Any]) -> Callable[..., dict[str, Any]]:
    """Decorator that catches exceptions and returns structured failures.

    Works for both sync and async tool functions.
    """

    @wraps(fn)
    def sync_wrapper(*args: Any, **kwargs: Any) -> dict[str, Any]:
        try:
            result = fn(*args, **kwargs)
            return result
        except Exception as exc:
            return failure(
                f"{type(exc).__name__}: {exc}",
                traceback=traceback.format_exc(limit=3),
            )

    @wraps(fn)
    async def async_wrapper(*args: Any, **kwargs: Any) -> dict[str, Any]:
        try:
            result = await fn(*args, **kwargs)
            return result
        except Exception as exc:
            return failure(
                f"{type(exc).__name__}: {exc}",
                traceback=traceback.format_exc(limit=3),
            )

    if asyncio.iscoroutinefunction(fn):
        return async_wrapper
    return sync_wrapper
