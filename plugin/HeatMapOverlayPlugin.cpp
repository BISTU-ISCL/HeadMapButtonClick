#include "HeatMapOverlayPlugin.h"
#include "HeatMapOverlay.h"

HeatMapOverlayPlugin::HeatMapOverlayPlugin(QObject *parent)
    : QObject(parent)
{
}

QWidget *HeatMapOverlayPlugin::createWidget(QWidget *parent)
{
    return new HeatMapOverlay(parent);
}

QString HeatMapOverlayPlugin::name() const
{
    return QStringLiteral("HeatMapOverlay");
}

QString HeatMapOverlayPlugin::group() const
{
    return QStringLiteral("Heat Map Widgets");
}

QIcon HeatMapOverlayPlugin::icon() const
{
    return QIcon();
}

QString HeatMapOverlayPlugin::toolTip() const
{
    return QStringLiteral("鼠标点击热力图蒙版控件");
}

QString HeatMapOverlayPlugin::whatsThis() const
{
    return QStringLiteral("在背景图片上展示点击热力图，支持透明蒙版、缩放和归一化。");
}

bool HeatMapOverlayPlugin::isContainer() const
{
    return false;
}

QString HeatMapOverlayPlugin::includeFile() const
{
    // 与安装路径保持一致，便于 Qt Designer 自动包含头文件
    return QStringLiteral("heatmapoverlay/HeatMapOverlay.h");
}

QString HeatMapOverlayPlugin::domXml() const
{
    return R"(<ui language=\"c++\">
 <widget class=\"HeatMapOverlay\" name=\"heatMapOverlay\">
  <property name=\"toolTip\" >
   <string>鼠标点击热力图蒙版控件</string>
  </property>
 </widget>
</ui>)";
}

bool HeatMapOverlayPlugin::isInitialized() const
{
    return m_initialized;
}

void HeatMapOverlayPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);
    if (m_initialized)
        return;
    m_initialized = true;
}
