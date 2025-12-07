#pragma once

#include <QtUiPlugin/QDesignerCustomWidgetInterface>
#include <QObject>

class HeatMapOverlayPlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface")
#endif

public:
    explicit HeatMapOverlayPlugin(QObject *parent = nullptr);

    QWidget *createWidget(QWidget *parent) override;
    QString name() const override;
    QString group() const override;
    QIcon icon() const override;
    QString toolTip() const override;
    QString whatsThis() const override;
    bool isContainer() const override;
    QString includeFile() const override;
    QString domXml() const override;
    bool isInitialized() const override;
    void initialize(QDesignerFormEditorInterface *core) override;

private:
    bool m_initialized = false;
};
