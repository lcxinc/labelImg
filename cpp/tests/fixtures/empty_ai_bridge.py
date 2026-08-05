import json
import sys


def main() -> int:
    for line in sys.stdin:
        if line.strip():
            json.loads(line)
            print(json.dumps({"ok": True, "shapes": []}), flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
