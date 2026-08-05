import json
import sys
import time


for line in sys.stdin:
    if not line.strip():
        continue
    print(json.dumps({
        "event": "progress",
        "model": "yoloworld:latest",
        "file_index": 0,
        "file_count": 1,
        "filename": "model.bin",
        "bytes_done": 128,
        "bytes_total": 512,
    }), flush=True)
    time.sleep(1.0)
    print(json.dumps({"ok": True, "shapes": []}), flush=True)
