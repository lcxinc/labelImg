import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

import numpy as np

BRIDGE = Path(__file__).resolve().parents[1] / 'cpp/tools/onnx_detection_bridge.py'
spec = importlib.util.spec_from_file_location('onnx_detection_bridge', BRIDGE)
bridge = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bridge)


class DetectionTests(unittest.TestCase):
    def decode(self, rows, layout='yolo8', names=None, **kwargs):
        output = np.array([rows], dtype=np.float32)
        if layout == 'yolo8':
            output = output.transpose(0, 2, 1)
        return bridge.decode(output, {'layout': layout, **kwargs}, names or ['tire', 'car'],
                             0.5, 0, 80, 1280, 960)

    def test_letterbox_coordinates_and_same_class_nms(self):
        shapes = self.decode([[100, 180, 100, 80, .9, .1],
                              [101, 181, 100, 80, .8, .1],
                              [100, 180, 100, 80, .1, .95]])
        self.assertEqual([s['label'] for s in shapes], ['car', 'tire'])
        self.assertEqual(shapes[1]['points'], [[100, 120], [300, 280]])

    def test_yolo5_uses_objectness_times_class_score(self):
        shapes = self.decode([[100, 180, 100, 80, .2, .9, .1],
                              [200, 280, 50, 50, .8, .1, .9]], layout='yolo5')
        self.assertEqual(len(shapes), 1)
        self.assertEqual(shapes[0]['label'], 'car')
        self.assertAlmostEqual(json.loads(shapes[0]['description'])['score'], .72, places=5)

    def test_end_to_end_clips_boxes_and_ignores_padding(self):
        shapes = self.decode([[-20, 50, 800, 1000, .9, 1], [0, 0, 0, 0, 0, 0]], layout='xyxy')
        self.assertEqual(shapes[0]['points'], [[0, 0], [1280, 960]])

    def test_empty_and_invalid_boxes(self):
        self.assertEqual(self.decode([[100, 180, -100, 80, .9, .1]]), [])
        self.assertEqual(self.decode([[100, 180, 100, 80, .01, .02]]), [])
        self.assertEqual(self.decode([[100, 180, 100, 80, float('nan'), .02]]), [])

    def test_wrong_layout_or_class_count_is_rejected(self):
        with self.assertRaisesRegex(ValueError, 'requires'):
            self.decode([[100, 180, 100, 80, .9, .1]], names=['only-one-class'])
        with self.assertRaisesRegex(ValueError, 'class ID'):
            self.decode([[0, 0, 100, 100, .9, 2]], layout='xyxy')
        with self.assertRaisesRegex(ValueError, 'class ID'):
            self.decode([[0, 0, 100, 100, .9, .5]], layout='xyxy')
        with self.assertRaisesRegex(ValueError, 'Thresholds'):
            self.decode([[100, 180, 100, 80, .9, .1]], score_threshold=float('nan'))

    def test_numeric_metadata_class_order_and_validation(self):
        self.assertEqual(bridge.class_names("{1: 'car', 0: 'tire'}"), ['tire', 'car'])
        for names in ([], ['a', ''], ['a', 'a']):
            with self.assertRaises(ValueError):
                bridge.class_names(names)

    def test_missing_model_server_returns_error_and_keeps_running(self):
        request = json.dumps({'operation': 'load', 'model_path': 'missing-model.onnx'})
        result = subprocess.run([sys.executable, str(BRIDGE), '--server'],
                                input=request + '\n' + request + '\n', text=True,
                                encoding='utf-8', capture_output=True, timeout=20)
        self.assertEqual(result.returncode, 0, result.stderr)
        responses = [json.loads(line) for line in result.stdout.splitlines()]
        self.assertEqual(len(responses), 2)
        self.assertTrue(all(not response['ok'] and response['error'] for response in responses))

    def test_cache_reuses_model_and_invalidates_replaced_file(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'model.onnx'
            path.write_bytes(b'first')
            request = {'operation': 'load', 'model_path': str(path)}
            detector = mock.Mock(names=['tire'])
            detector.input.shape = [1, 3, 640, 640]
            with mock.patch.object(bridge, 'Detector', return_value=detector) as create:
                cache = {}
                bridge.process(request, cache)
                bridge.process(request, cache)
                self.assertEqual(create.call_count, 1)
                path.write_bytes(b'replaced model')
                bridge.process(request, cache)
                self.assertEqual(create.call_count, 2)


class RuntimeIntegrationTests(unittest.TestCase):
    def test_real_runtime_load_infer_unicode_and_preprocessing(self):
        try:
            import onnx
            from onnx import TensorProto, helper
            from PIL import Image
            import onnxruntime  # noqa: F401
        except ImportError:
            self.skipTest('Integration test requires onnx, onnxruntime and Pillow')
        # The output score depends on the input's red-channel mean, so this
        # checks real execution, RGB/NCHW normalization, and image transport.
        input_info = helper.make_tensor_value_info('images', TensorProto.FLOAT, [1, 3, 64, 64])
        output_info = helper.make_tensor_value_info('output', TensorProto.FLOAT, [1, 5, 1])
        coords = helper.make_tensor('coords', TensorProto.FLOAT, [1, 4, 1], [32, 32, 32, 32])
        index = helper.make_tensor('index', TensorProto.INT64, [1], [0])
        shape = helper.make_tensor('shape', TensorProto.INT64, [3], [1, 1, 1])
        nodes = [helper.make_node('Gather', ['images', 'index'], ['red'], axis=1),
                 helper.make_node('ReduceMean', ['red'], ['score'], keepdims=1),
                 helper.make_node('Reshape', ['score', 'shape'], ['score3']),
                 helper.make_node('Concat', ['coords', 'score3'], ['output'], axis=1)]
        model = helper.make_model(helper.make_graph(nodes, 'test', [input_info], [output_info],
                                                   [coords, index, shape]),
                                  opset_imports=[helper.make_opsetid('', 13)], ir_version=8)
        helper.set_model_props(model, {'names': "{0: '轮胎'}", 'task': 'detect'})
        with tempfile.TemporaryDirectory(prefix='onnx-测试-') as directory:
            model_path = Path(directory) / '模型.onnx'
            image_path = Path(directory) / '图片.png'
            onnx.save(model, model_path)
            Image.new('RGB', (128, 128), (255, 0, 0)).save(image_path)
            load = {'operation': 'load', 'model_path': str(model_path)}
            infer = {**load, 'operation': 'infer', 'image_path': str(image_path), 'layout': 'yolo8'}
            result = subprocess.run([sys.executable, str(BRIDGE), '--server'],
                                    input='\n'.join(map(json.dumps, [load, infer, infer])) + '\n',
                                    encoding='utf-8', capture_output=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stderr)
            loaded, first, second = map(json.loads, result.stdout.splitlines())
            self.assertTrue(loaded['ok'], loaded)
            self.assertEqual(loaded['classes'], ['轮胎'])
            self.assertTrue(first['ok'], first)
            self.assertEqual(first, second)
            self.assertEqual(first['shapes'][0]['points'], [[32, 32], [96, 96]])
            self.assertEqual(json.loads(first['shapes'][0]['description'])['score'], 1.0)
            with self.assertRaisesRegex(ValueError, 'differ from model metadata'):
                bridge.Detector(model_path).infer({**infer, 'classes': ['wrong']})


if __name__ == '__main__':
    unittest.main()
