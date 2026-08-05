import json
import sys


def main() -> int:
    for line in sys.stdin:
        if not line.strip():
            continue
        request = json.loads(line)
        threshold = float(request.get("score_threshold", 0.0))
        requested = request.get("texts", [])
        output_format = request.get("output_format", "rectangle")

        detections = [
            ("person", 0.85, [[20.0, 15.0], [70.0, 80.0]]),
            ("sofa", 0.25, [[80.0, 20.0], [145.0, 70.0]]),
        ]
        shapes = []
        for label, score, points in detections:
            if label not in requested or score < threshold:
                continue
            shapes.append(
                {
                    "label": label,
                    "description": "score={:.2f};model={}".format(score, request.get("model", "")),
                    "shape_type": output_format,
                    "points": points,
                    "flags": {"fixture": True},
                }
            )

        print(json.dumps({"ok": True, "shapes": shapes}, ensure_ascii=False), flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
