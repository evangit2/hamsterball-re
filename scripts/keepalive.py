#!/usr/bin/env python3
"""Send a 'continue' message to the Hamsterball RE Telegram chat to keep Hermes session alive."""
import os
import time
import requests
from urllib.parse import quote

# Telegram bot token from Hermes .env
BOT_TOKEN = os.environ.get("TELEGRAM_BOT_TOKEN", "")
if not BOT_TOKEN:
    # Try loading from .env
    env_path = os.path.expanduser("~/.hermes/.env")
    if os.path.exists(env_path):
        for line in open(env_path):
            if line.startswith("TELEGRAM_BOT_TOKEN="):
                BOT_TOKEN = line.strip().split("=", 1)[1]
                break

# Jack's Telegram user ID (from session key: telegram:dm:7512373356)
CHAT_ID = "7512373356"

def send_continue():
    """Send 'continue' message to the chat via Telegram Bot API."""
    url = f"https://api.telegram.org/bot{BOT_TOKEN}/sendMessage"
    payload = {
        "chat_id": CHAT_ID,
        "text": "continue"
    }
    try:
        resp = requests.post(url, json=payload, timeout=10)
        if resp.status_code == 200:
            print(f"[{time.strftime('%H:%M:%S')}] Sent 'continue' to chat {CHAT_ID}")
        else:
            print(f"[{time.strftime('%H:%M:%S')}] Failed: {resp.status_code} {resp.text[:100]}")
    except Exception as e:
        print(f"[{time.strftime('%H:%M:%S')}] Error: {e}")

if __name__ == "__main__":
    send_continue()