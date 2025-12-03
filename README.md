# HeatMapOverlay 控件

此仓库提供一个可添加到 Qt 控件库的鼠标点击热力图控件 **HeatMapOverlay**，并附带 Qt Designer 插件与示例程序。

## 特性
- 透明蒙版：可覆盖在任意 QWidget 或背景图片上显示热点分布。
- 支持 Qt Designer：提供 `HeatMapOverlayPlugin`，可直接拖拽到界面。
- 丰富属性：热力点半径、颜色渐变、透明度、自动归一化、坐标归一化、辅助十字线等。
- 自适应缩放：使用归一化坐标时，可随背景缩放保持热点位置正确。
- Demo 应用：加载图片、录入点击、实时查看热力图效果。

## 主要属性
- `baseImage (QImage)`: 背景图片，可选；为空时仅显示热力图。
- `clickPoints (QVector<QPointF>)`: 点击坐标，默认使用 0~1 归一化坐标。
- `pointRadius (int)`: 热力点半径，控制模糊范围。
- `heatmapOpacity (qreal)`: 热力图整体透明度 (0~1)。
- `autoNormalize (bool)`: 自动归一化叠加强度，避免局部过曝。
- `normalizedCoordinates (bool)`: 是否启用归一化坐标。
- `coldColor/hotColor (QColor)`: 热力渐变的冷/热端颜色。
- `showCrosshair (bool)`: 是否显示调试用十字线。

## 构建
需要 Qt 5 或 Qt 6（Widgets/Gui/Designer 模块）。
```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt
cmake --build .
```
生成内容：
- `libheatmapoverlay.a`：控件静态库。
- `HeatMapOverlayPlugin`：Qt Designer 插件（输出在 `designer/`，安装到 `lib/designer`）。
- `heatmap_demo`：示例程序。

将 `designer/` 下的插件复制到 Qt Designer 的插件目录即可在设计器中使用控件。

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

QVector<QPointF> clicks = { {0.2, 0.3}, {0.5, 0.6}, {0.8, 0.25} };
overlay->setClickPoints(clicks);
```
3. 如需记录运行时点击，可在宿主控件的鼠标事件中调用 `addClick()`（传入归一化坐标更易于缩放显示）。

## 设计要点
- 热力叠加使用 `CompositionMode_Plus` 叠加径向渐变，实现平滑热点。
- 可选自动归一化，将最大透明度拉升到 255，保证热点对比度。
- 背景图片按 `KeepAspectRatioByExpanding` 缩放并居中裁剪，确保蒙版铺满控件。
- 插件使用 `QDesignerCustomWidgetInterface`，可在设计时调整公开的属性。
