# HeatMapOverlay 控件

此仓库提供一个可添加到 Qt 控件库的鼠标点击热力图控件 **HeatMapOverlay**，并附带 Qt Designer 插件与示例程序，可在任务录屏或静态截图上叠加鼠标点击热图（类似眼动热图的可视化效果）。

## 特性
- 透明蒙版：可覆盖在任意 QWidget 或背景图片上显示热点分布。
- 支持 Qt Designer：提供 `HeatMapOverlayPlugin`，可直接拖拽到界面。
- 丰富属性：热力点半径、颜色渐变、透明度、自动归一化、坐标归一化、辅助十字线等。
- 自适应缩放：支持“完整适配”/“铺满裁剪”两种缩放模式，热区半径可随背景缩放无失真，方便替换背景图片后直接扣上蒙版展示。
- Demo 应用：加载图片、录入点击、实时查看热力图效果。

## 主要属性
- `scaleMode (ScaleMode)`: 背景缩放方式，`FitInside`（完整呈现有留边）或 `CoverWidget`（铺满裁剪）。
- `baseImage (QImage)`: 背景图片，可选；为空时仅显示热力图。
- `clickPoints (QVector<QPointF>)`: 点击坐标，默认使用 0~1 归一化坐标。
- `pointRadius (int)`: 热力点半径，控制模糊范围。
- `adaptivePointRadius (bool)`: 是否随背景缩放自动放大/缩小半径，保证热点大小不失真。
- `heatmapOpacity (qreal)`: 热力图整体透明度 (0~1)。
- `autoNormalize (bool)`: 自动归一化叠加强度，避免局部过曝。
- `normalizedCoordinates (bool)`: 是否启用归一化坐标。
- `coldColor/hotColor (QColor)`: 热力渐变的冷/热端颜色。
- `showCrosshair (bool)`: 是否显示调试用十字线。
- `displayRect()`: 返回热图实际绘制区域（考虑 letterbox），便于外部坐标映射。

## 构建
仅支持 Qt 5.15.2（Widgets/Gui/Designer 模块）。插件必须由 **Qt 5.15.2 工具链** 编译并加载，方可在同版本 Qt Designer/Qt Creator（使用 Qt 5.15.2 套件）中显示；Qt 6 或其他版本不在支持范围内。
```bash
mkdir build && cd build
# 给 Qt 5.15.2 Designer/Qt Creator (Qt 5.15.2 套件) 构建插件
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt/5.15.2/
cmake --build .
```
生成内容：
- `libheatmapoverlay.a`：控件静态库。
- `HeatMapOverlayPlugin`：Qt Designer 插件（输出在 `designer/`，安装到 `lib/designer`）。
- `heatmap_demo`：示例程序。

将 `designer/` 下的插件复制到 Qt 5.15.2 对应的插件目录即可在设计器中使用控件：
- Qt Creator（需选择 Qt 5.15.2 Kit）：`<QtCreator安装目录>/lib/Qt/plugins/designer/`
- 独立 Qt Designer 5.15.2：`<Qt安装目录>/5.15.2/msvcXXXX/plugins/designer/`

> 若 Qt 安装包同时包含 Qt Creator 6.x（基于 Qt6），请在 Qt Creator 内选择基于 **Qt 5.15.2** 的 Kit 编译/运行，否则插件不会被加载。

> 如果开发环境暂时无法安装 Qt 依赖，可运行 `scripts/generate_sample_heatmap.py` 生成 `artifacts/sample_heatmap.bmp`，快速预览热力图效果。

## 示例运行
```bash
./build/heatmap_demo
```
操作步骤：
1. 点击“选择背景图片”载入任务截图。
2. 在画布上点击即可记录坐标并显示热力图。
3. 使用半径/透明度/归一化/十字线等控件实时调整效果。
4. “清除点击记录”按钮可清空热点。

## 配置示例
`config/heatmap_config.json` 给出常用属性的默认值，可作为自定义配置参考。

## 集成到项目
1. 将 `src/HeatMapOverlay.*` 加入项目并链接 Qt Widgets/Gui，或使用 `cmake --install` 后包含安装位置的 `include/heatmapoverlay`。
2. 在界面中实例化控件，设置 `baseImage` 与点击数据：
```cpp
HeatMapOverlay *overlay = new HeatMapOverlay(parent);
overlay->setBaseImage(QImage("background.png"));
overlay->setPointRadius(40);
overlay->setHeatmapOpacity(0.6);
overlay->setColdColor(QColor("#00bcd4"));
overlay->setHotColor(QColor("#ff5722"));
overlay->setScaleMode(HeatMapOverlay::FitInside); // 按需选择 FitInside / CoverWidget

QVector<QPointF> clicks = { {0.2, 0.3}, {0.5, 0.6}, {0.8, 0.25} };
overlay->setClickPoints(clicks);
```
3. 如需记录运行时点击，可在宿主控件的鼠标事件中调用 `addClick()`（传入归一化坐标更易于缩放显示；可使用 `displayRect()` 将窗口坐标转换为背景坐标再归一化）。

## 设计要点
- 热力叠加使用 `CompositionMode_Plus` 叠加径向渐变，实现平滑热点。
- 可选自动归一化，将最大透明度拉升到 255，保证热点对比度。
- 背景缩放模式可切换：`FitInside` 保持全图、`CoverWidget` 铺满裁剪；热力图与背景共享同一映射，替换图片或窗口缩放均保持热点位置一致。
- 插件使用 `QDesignerCustomWidgetInterface`，可在设计时调整公开的属性。
