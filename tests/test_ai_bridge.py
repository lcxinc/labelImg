import importlib.util
from pathlib import Path
import sys
from types import SimpleNamespace
import unittest
from unittest import mock

import numpy as np


def load_bridge_module():
    path = Path(__file__).resolve().parents[1] / "cpp" / "tools" / "labelme_ai_bridge.py"
    spec = importlib.util.spec_from_file_location("labelme_ai_bridge_under_test", path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Unable to load {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class TestLabelMeAiBridge(unittest.TestCase):
    def test_bridge_session_reuses_image_embedding(self):
        bridge = load_bridge_module()
        model = mock.Mock()
        model.encode_image.side_effect = lambda image: object()
        with mock.patch.object(bridge, "_load_model", return_value=model):
            session = bridge._BridgeSession("sam2:latest")
            image = np.zeros((4, 4, 3), dtype=np.uint8)
            first = session.image_embedding(image, "image-a")
            second = session.image_embedding(image, "image-a")

        self.assertIs(first, second)
        model.encode_image.assert_called_once_with(image=image)

    def test_load_model_pulls_uncached_model_once(self):
        bridge = load_bridge_module()
        pulled = []

        class FakeModelType:
            def __new__(cls):
                return "prepared-model"

            @classmethod
            def get_size(cls):
                return None

            @classmethod
            def pull(cls):
                pulled.append(True)

        fake_osam = SimpleNamespace(
            apis=SimpleNamespace(
                get_model_type_by_name=lambda name: FakeModelType,
            )
        )
        with mock.patch.dict(sys.modules, {"osam": fake_osam}):
            self.assertEqual(bridge._load_model("sam2:latest"), "prepared-model")
        self.assertEqual(pulled, [True])

    def test_load_model_skips_pull_for_cached_model(self):
        bridge = load_bridge_module()
        pulled = []

        class FakeModelType:
            def __new__(cls):
                return "cached-model"

            @classmethod
            def get_size(cls):
                return 1024

            @classmethod
            def pull(cls):
                pulled.append(True)

        fake_osam = SimpleNamespace(
            apis=SimpleNamespace(
                get_model_type_by_name=lambda name: FakeModelType,
            )
        )
        with mock.patch.dict(sys.modules, {"osam": fake_osam}):
            self.assertEqual(bridge._load_model("sam2:latest"), "cached-model")
        self.assertEqual(pulled, [])

    def test_load_model_reports_download_progress(self):
        bridge = load_bridge_module()
        progress_events = []

        class FakeModelType:
            _blobs = {
                "weights": SimpleNamespace(attachments=[SimpleNamespace()]),
            }

            def __new__(cls):
                return "prepared-model"

            @classmethod
            def get_size(cls):
                return None

            @classmethod
            def pull(cls, progress):
                progress("weights.bin", 128, 512)

        fake_osam = SimpleNamespace(
            apis=SimpleNamespace(
                get_model_type_by_name=lambda name: FakeModelType,
            )
        )
        with mock.patch.dict(sys.modules, {"osam": fake_osam}):
            self.assertEqual(
                bridge._load_model("sam2:latest", progress_events.append),
                "prepared-model",
            )

        self.assertEqual(
            progress_events,
            [
                {
                    "event": "progress",
                    "model": "sam2:latest",
                    "file_index": 0,
                    "file_count": 2,
                    "filename": "weights.bin",
                    "bytes_done": 128,
                    "bytes_total": 512,
                }
            ],
        )

    def test_point_annotations_are_sorted_by_score(self):
        bridge = load_bridge_module()
        annotations = [
            SimpleNamespace(score=0.2),
            SimpleNamespace(score=0.95),
            SimpleNamespace(score=None),
        ]

        ordered = bridge._sort_annotations_by_score(annotations)

        self.assertEqual([item.score for item in ordered], [0.95, 0.2, None])

    def test_point_annotations_are_suppressed_after_score_sorting(self):
        bridge = load_bridge_module()
        annotations = [
            SimpleNamespace(
                score=0.4,
                text=None,
                bounding_box=SimpleNamespace(xmin=0, ymin=0, xmax=10, ymax=10),
                mask=None,
            ),
            SimpleNamespace(
                score=0.9,
                text=None,
                bounding_box=SimpleNamespace(xmin=1, ymin=1, xmax=9, ymax=9),
                mask=None,
            ),
            SimpleNamespace(
                score=0.8,
                text=None,
                bounding_box=SimpleNamespace(xmin=30, ymin=30, xmax=40, ymax=40),
                mask=None,
            ),
        ]

        kept = bridge._suppress_annotations(
            bridge._sort_annotations_by_score(annotations),
            iou_threshold=0.5,
        )

        self.assertEqual([item.score for item in kept], [0.9, 0.8])

    def test_annotation_suppression_rejects_mismatched_mask_extent(self):
        bridge = load_bridge_module()
        annotations = [
            SimpleNamespace(
                score=0.9,
                text=None,
                bounding_box=SimpleNamespace(xmin=0, ymin=0, xmax=4, ymax=4),
                mask=np.ones((2, 2), dtype=bool),
            )
        ]

        with self.assertRaisesRegex(ValueError, "mask shape"):
            bridge._suppress_annotations(annotations, iou_threshold=0.5)

    def test_circle_uses_inscribed_bbox_when_mask_is_missing(self):
        bridge = load_bridge_module()
        annotation = SimpleNamespace(
            bounding_box=SimpleNamespace(xmin=10, ymin=20, xmax=30, ymax=50),
            mask=None,
        )

        shape = bridge._shape_from_annotation(annotation, "circle")

        self.assertIsNotNone(shape)
        self.assertEqual(shape["points"], [[20.0, 35.0], [30.0, 35.0]])

    def test_oriented_rectangle_uses_minimum_area_mask_box(self):
        bridge = load_bridge_module()
        annotation = SimpleNamespace(
            bounding_box=SimpleNamespace(xmin=100, ymin=200, xmax=120, ymax=210),
            mask=np.ones((11, 21), dtype=bool),
        )

        shape = bridge._shape_from_annotation(annotation, "oriented_rectangle")

        self.assertIsNotNone(shape)
        self.assertEqual(
            shape["points"],
            [[100.0, 200.0], [120.0, 200.0], [120.0, 210.0], [100.0, 210.0]],
        )

    def test_oriented_rectangle_uses_mask_orientation_instead_of_bbox(self):
        bridge = load_bridge_module()
        y, x = np.indices((15, 15))
        mask = np.abs(x - 7) + np.abs(y - 7) <= 5
        annotation = SimpleNamespace(
            bounding_box=SimpleNamespace(xmin=100, ymin=200, xmax=112, ymax=212),
            mask=mask,
        )

        shape = bridge._shape_from_annotation(annotation, "oriented_rectangle")

        self.assertIsNotNone(shape)
        self.assertEqual(
            shape["points"],
            [[102.0, 207.0], [107.0, 202.0], [112.0, 207.0], [107.0, 212.0]],
        )


if __name__ == "__main__":
    unittest.main()
