# 性能检查与优化记录（2026-09-09）

检查范围：C++ 桌面版的启动、目录扫描、文件搜索/标签过滤、缩略图、切图、绘制与平移、鸟瞰图、亮度/对比度、保存、剪贴板及窗口交互。已编译 Release，并验证标注格式和交互回归。

已正常关闭旧进程并启动 `target/cpp-build/Release/labelImgCpp.exe`，恢复 `CAM-03/26/2d/18.jpg`。启动后窗口响应正常，工作集约 89.8 MiB；28.6 秒空闲观察期间 CPU 增量 0.031 秒，约占一个核心的 0.11%。记录见 `target/performance-app-start.json` 和 `target/performance-app-idle.json`。

## 测量方法与结果

环境：Windows 11、MSVC 2022、Qt 6.11.0、Release。对比使用同一基准函数、相同合成数据规则，在独立进程中运行；没有清空操作系统文件缓存。这是单次前后实测，不是多机统计或性能保证。

目录基准包含 6,000 张 96×64 JPEG，其中 300 张带 VOC 标注；创建测试文件的时间不计入操作耗时。绘制基准使用 4,000×3,000 图像，密集标注基准使用 1,000 个框。Qt 离屏后端测量事件处理与绘制耗时，不包含物理显示器刷新和输入设备延迟。

| 操作 | 优化前 | 优化后 |
| --- | ---: | ---: |
| 窗口构造 | 112 ms | 118 ms |
| 打开 6,000 张图片的目录 | 19,760 ms | 553 ms |
| 连续三次文件搜索 | 43,607 ms | 49 ms |
| 开启缩略图至首屏可用 | 33,024 ms | 91 ms |
| 前后切图共 12 次 | 288 ms | 118 ms |
| 保存当前标注 3 次 | 21.7 ms | 15.5 ms |
| 十字移动 200 次并处理绘制事件 | 318 ms | 203 ms |
| 获取鸟瞰底图 500 次 | 20.3 ms | 0.012 ms |
| 1,200 万像素图像调整亮度并复原 | 145 ms | 65 ms |
| 1,000 个框重绘 30 次 | 273 ms | 259 ms |
| 静止 300 ms 的画布重绘次数 | 0 | 0 |

缩略图优化前会同步完成整个目录；优化后首屏可用即返回，其他项随滚动后台生成，因此此行比较的是用户可交互时间，并不代表完整目录的解码吞吐量。

新增测量：十字移动的单次事件处理 P95 为 1.57 ms，最大 2.81 ms；拖动绘制 200 次共 212 ms，平移 100 次共 117 ms，连续缩放 12 次共 28.7 ms。窗口构造和密集框重绘没有明显改善，不将小幅波动认定为收益。

实际数据目录 `D:/tire-capture-data/CAM-06` 共识别 6,149 张支持格式的图片，打开耗时 559 ms；首张图像为 2,560×1,024，前后切图 8 次共 240 ms。实际目录仅执行读取和切图，未调用保存或创建标注操作。保存和绘制测试使用临时目录。

原始记录：

- `target/performance-before.json`：优化前基准。
- `target/performance-after.json`：第一轮优化结果。
- `target/performance-final.json`、`target/performance-final.log`：最终基准及实际目录测量。
- `target/perf-core-final.txt`、`target/perf-ui-final.txt`、`target/perf-native-final.txt`：验证记录。

## 瓶颈与修改

1. **目录索引**：原先每次搜索都重新打开所有图片、解析标注，再逐张检查 XML/TXT/JSON。现在每个目录只枚举一次标注文件，搜索和类别筛选复用索引。标签提取无需图片像素；LabelMe 索引和缩略图只解析元数据，正式打开标注仍完整校验嵌入图像。打开目录、变更保存目录以及应用内保存/删除会刷新相应索引。
2. **缩略图**：由同步解码整目录改为一个后台解码线程，仅处理当前可见行及邻近行；缓存最多 256 个 72×54 图标，滚出视区的列表项释放图标引用。过滤、换目录和保存时用代次号丢弃过期结果，损坏图片也缓存失败结果，避免重复解码循环。
3. **绘制热路径**：鸟瞰底图按原图和请求尺寸复用，换图即失效；关闭鸟瞰图后跳过刷新。剪贴板内容只在变化时解析，不再在每次刷新绘制动作状态时读取系统剪贴板。窗口标题和文件行样式仅在状态变化时更新。
4. **亮度与对比度**：用 256 项查找表替代每像素每通道的重复浮点计算，保留原有亮度先于对比度、取整、灰度均值及透明度规则。
5. **保存与交互**：保留此前修复的手势结束后自动保存、单行刷新、十字即时跟随、W 重复创建、空格确认、Esc 返回查看模式和原生无边框缩放行为，并纳入回归。

后台只操作 QImage 和数据，QPixmap 及控件在主线程更新，遵循 [Qt 线程与绘图要求](https://doc.qt.io/qt-6/threads-modules.html)。

## 验证

- 核心测试：114 通过，0 失败，0 跳过，覆盖格式读写、TIFF、配置和 AI 桥接。
- 完整界面测试：331 通过，0 失败。离屏运行跳过可选性能基准及 Windows 原生命中测试；两者另行执行通过。
- Windows 原生补测：150% 缩放下八个窗口缩放边/角、W 重复创建/取消及空格确认均通过。
- 新增回归验证可见缩略图按需加载、保存后更新框、滚动/过滤时异步结果安全、换图后鸟瞰底图失效，以及连续交互不重复读取剪贴板。
- `git diff --check` 通过。

基线测试环境原先缺少 Python 和参考示例。AI 桥接测试补入 Python PATH；参考示例来自 LabelMe `v5.7.0`，提交 `0667da095640ba856065362490ad0994cd3ed046`。修正了旧测试将 W 当成矩形的勾选动作，以及依赖本地示例版本/空描述的假设：版本测试验证源值完整往返，空描述测试显式构造 null 输入。正式数据加载校验没有因性能优化而放宽。

## 复现

构建 `labelImgCppUiTests` 和 `labelImgCppTests` 的 Release 目标。运行目录为仓库根目录，Qt DLL（包括新增的 Qt6Concurrent.dll）必须可用；AI 桥接测试要求 PATH 中存在 Python。参考示例可用以下方式取得：

```powershell
git clone --depth 1 --branch v5.7.0 --filter=blob:none --sparse https://github.com/wkentaro/labelme.git refs/labelme
git -C refs/labelme sparse-checkout set examples
```

性能基准为显式启用，普通回归不执行大目录生成：

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
$env:LABELIMG_RUN_PERF = '1'
$env:LABELIMG_PERF_OUTPUT = "$PWD/target/performance-repeat.json"
$env:QTEST_FUNCTION_TIMEOUT = '240000'
# 可选，只读测量实际图片目录：
$env:LABELIMG_PERF_REAL_DIR = 'D:/tire-capture-data/CAM-06'
& ./target/cpp-build/Release/labelImgCppUiTests.exe performanceAudit -o "$PWD/target/performance-repeat.log,txt"
Remove-Item Env:LABELIMG_RUN_PERF, Env:LABELIMG_PERF_REAL_DIR
```

## 边界

文件列表缓存反映最近打开目录或应用内更新后的状态；外部程序修改标注后，重新打开目录可重新建立索引。主图首次解码及严格 LabelMe 图像校验仍需完成；超大 TIFF、网络盘和真实 AI 模型推理未进行专项压力测试。后台解码已开始时不能强制中断，过期结果会被丢弃，退出会等待已有解码安全完成。

静止时低 FPS 是没有新画面的正常结果；本次验证静止画布没有无效重绘循环。
