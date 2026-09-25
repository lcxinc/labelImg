# labelImgCpp

## 本地 ONNX 目标检测

在“文件 → 打开更多 → ONNX 目标检测”中选择本地 `.onnx` 文件，点击“加载模型”，再点击“检测当前图片”。检测框添加到当前图片，保留已有标注，可调整、撤销并按当前格式保存；已有自动保存设置继续生效。无需打开图片也可检查模型是否能够加载。

运行程序的 `PATH` 中需有 Python 3.10+。安装可选依赖：

```powershell
python -m pip install -r cpp/tools/requirements-onnx.txt
```

便携版/安装版在程序目录执行 `python -m pip install -r requirements-onnx.txt`。安装包包含桥接脚本和依赖清单，未内置 Python 或模型。推理通过 ONNX Runtime 的 CPU 后端运行，不下载模型。

支持单图 RGB NCHW、float32/float16 的 YOLO 检测模型。输出格式需选择与导出一致的选项：YOLOv8/YOLO11 `[1,4+C,N]`、YOLOv5 `[1,N,5+C]`，或 end-to-end/内置 NMS `[1,N,6]`（`x1,y1,x2,y2,score,class_id`）。坐标为输入图像像素坐标；不支持分类、分割、姿态、OBB、多输入、多输出或自定义预处理模型。预处理为 RGB、等比例缩放、114 居中填充和除以 255；固定尺寸来自模型，动态尺寸使用窗口设置。每次最多生成 300 个框。

类别优先读取模型的 `names` 元数据；缺少时在窗口按 ID 从 0 开始每行填写一个类别。YOLO 数据集模式自动使用 YAML 类别，且与模型元数据严格核对名称和顺序。界面中的普通标签历史不作为模型类别编号。置信度过滤和同类别 NMS 后会还原到原图坐标；重复检测还会抑制与现有同类框重叠的结果。

模型加载和推理在独立进程中执行，窗口显示进度，取消或关闭会终止进程。单次操作超过 180 秒会超时报错；错误不会修改标注。模型在窗口打开期间复用，模型路径和成功推理的参数保存在设置中。

实现参考：[ONNX Runtime Python API](https://onnxruntime.ai/docs/api/python/api_summary)、[Ultralytics detection export layouts](https://docs.ultralytics.com/guides/end2end-detection)。

Qt 6 C++ / Qt Widgets port of the Python labelImg application.

See [docs/feature-parity-design.md](docs/feature-parity-design.md) for the parity matrix, implementation boundaries, and regression checklist.

## Windows MSVC Build

Install:

- Visual Studio Build Tools with MSVC C++ workload
- Qt 6 MSVC package
- CMake 3.24+

Configure and build:

```powershell
bin\build.bat -QtPrefix "D:\Qt\6.11.0\msvc2022_64" -RunTests
```

Deploy manually when needed:

```powershell
windeployqt target\cpp-build\Release\labelImgCpp.exe
```

Build the Windows deployment directory, portable zip, and per-user installer:

```powershell
powershell -ExecutionPolicy Bypass -File cpp\packaging\build_windows_installer.ps1
```

Build artifacts are written under `target/`; the repository `bin/` directory contains only developer entry scripts.

The installer writes to `%LOCALAPPDATA%\Programs\labelImgCpp` and creates Start Menu/Desktop shortcuts, so it does not require administrator rights.

The optional `labelme_ai_bridge.py` is included beside the executable in both the portable zip and installer payload, so AI point/box modes keep the same runtime lookup path after installation.

## 拷贝到其他 Windows 电脑

请使用 `target/dist/labelImgCpp-<版本>-win64.zip`，完整解压后运行里面的 `labelImgCpp.exe`。请勿只复制 EXE；旁边的 Qt DLL、`platforms`、`iconengines` 和 `imageformats` 等目录也需要保留。

图标和语言资源现已编译进 EXE，不依赖开发电脑的源码路径或系统图标字体。新电脑首次启动默认简体中文；已有语言选择继续保留。若之前已保存为英文，可在 `View → Language → 简体中文` 切换。`data/predefined_classes.txt` 仍可外部修改，缺失时使用内置默认类别。ONNX 推理仍需目标电脑安装 Python 和可选依赖。

只生成便携包：

```powershell
powershell -ExecutionPolicy Bypass -File cpp/packaging/build_windows_installer.ps1 -BuildDir target/cpp-portable-build -QtBin C:/Qt/6.11.0/msvc2022_64/bin -CMakeExe C:/Qt/Tools/CMake_64/bin/cmake.exe -Version 0.1.1 -PortableOnly
```

## YOLO 数据集标注调整

队列中的图片被外部删除时，切换到该图片会显示打开失败提醒；关闭提醒后自动重新扫描目录并更新队列。当前画面、缩放和未保存标注保留，再次切换使用新队列。若当前图片也已被删除，下一次从新队列的第一张继续；目录已空时提示没有下一张。YOLO 数据集只重新扫描 `images`，保留类别与标签路径映射。

按 Q/E 切换上一处/下一处标注时，目标不完整位于画布可视区域内会自动滚动到目标位置，并保持当前缩放比例。目标已完整可见时不移动画面；目标大于视口时显示其中心区域。标注列表继续同步选中项，单个标注的选中/取消行为保持不变。

在“文件 → 打开更多 → YOLO数据库”中选择包含 `data.yaml`（或 `data.yml`）和 `images` 的数据集根目录，例如 `D:\tire-training\dataset-v3-cam01-cam06`。也可以通过启动参数传入根目录或 YAML 文件。

程序递归列出 `images` 中的全部图片（包括未列入训练抽样清单的图片），读取 YAML 中的 `names` 类别，并自动保持 YOLO 检测框格式。`images/train/cam01/a.jpg` 的标注读取和保存到 `labels/train/cam01/a.txt`；子目录结构和类别编号保持不变。缺少的标签在保存时创建，空文件表示无目标。

使用矩形框工具调整标注，按 Ctrl+S 保存；已有的自动保存设置继续生效。YAML 和训练清单不被修改，也不会生成额外的 `classes.txt`。此模式使用 YAML 中已有的类别；分割、姿态等非五列检测标签会报告读取错误并阻止覆盖。重启时恢复数据集模式。

普通“打开目录”、最近目录和启动参数中的目录会自动检查 YOLO 数据集结构。识别成功后弹窗显示类型、目录和类别数量；确认使用 YOLO 规则，取消、Esc 或关闭弹窗则继续按默认规则打开同一目录。普通目录或无效数据集配置不弹窗。“打开更多 → YOLO数据库”以及已保存的数据集会话恢复仍直接使用指定模式。
