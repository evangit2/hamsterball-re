#!/usr/bin/env bash
# Run the Hamsterball testing daemon.
cd "$(dirname "$0")"
source .venv/bin/activate
python -m hbtestd.server
