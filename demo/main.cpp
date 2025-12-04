#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QSlider>
#include <QLabel>
#include <QSpinBox>
#include <QFrame>
#include <QCheckBox>
#include <QComboBox>
#include <QMouseEvent>
#include <QEvent>
#include "../src/HeatMapOverlay.h"

// Demo 程序：载入背景图片，录入点击并实时展示热力图
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle(QStringLiteral("HeatMapOverlay Demo"));
    window.resize(960, 640);

    auto *overlay = new HeatMapOverlay();
    overlay->setNormalizedCoordinates(true);

    QVBoxLayout *rootLayout = new QVBoxLayout();

    QHBoxLayout *controlLayout = new QHBoxLayout();

    QPushButton *loadBtn = new QPushButton(QStringLiteral("选择背景图片"));
    controlLayout->addWidget(loadBtn);

    QLabel *radiusLabel = new QLabel(QStringLiteral("半径"));
    QSpinBox *radiusSpin = new QSpinBox();
    radiusSpin->setRange(5, 200);
    radiusSpin->setValue(25);
    controlLayout->addWidget(radiusLabel);
    controlLayout->addWidget(radiusSpin);

    QLabel *scaleLabel = new QLabel(QStringLiteral("缩放模式"));
    QComboBox *scaleCombo = new QComboBox();
    scaleCombo->addItem(QStringLiteral("铺满裁剪"), HeatMapOverlay::CoverWidget);
    scaleCombo->addItem(QStringLiteral("完整适配"), HeatMapOverlay::FitInside);
    controlLayout->addWidget(scaleLabel);
    controlLayout->addWidget(scaleCombo);

    QLabel *opacityLabel = new QLabel(QStringLiteral("透明度"));
    QSlider *opacitySlider = new QSlider(Qt::Horizontal);
    opacitySlider->setRange(0, 100);
    opacitySlider->setValue(65);
    controlLayout->addWidget(opacityLabel);
    controlLayout->addWidget(opacitySlider);

    QCheckBox *normalizeBox = new QCheckBox(QStringLiteral("自动归一化"));
    normalizeBox->setChecked(true);
    controlLayout->addWidget(normalizeBox);

    QCheckBox *adaptiveRadiusBox = new QCheckBox(QStringLiteral("半径随缩放"));
    adaptiveRadiusBox->setChecked(true);
    controlLayout->addWidget(adaptiveRadiusBox);

    QCheckBox *crosshairBox = new QCheckBox(QStringLiteral("显示十字线"));
    controlLayout->addWidget(crosshairBox);

    QPushButton *clearBtn = new QPushButton(QStringLiteral("清除点击记录"));
    controlLayout->addWidget(clearBtn);

    controlLayout->addStretch();

    rootLayout->addLayout(controlLayout);

    // 使用 QFrame 包裹 overlay，支持鼠标点击录入
    QFrame *frame = new QFrame();
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setStyleSheet("background: #202020;");
    QVBoxLayout *frameLayout = new QVBoxLayout(frame);
    frameLayout->setContentsMargins(0, 0, 0, 0);
    frameLayout->addWidget(overlay);
    rootLayout->addWidget(frame, 1);

    window.setLayout(rootLayout);

    QObject::connect(loadBtn, &QPushButton::clicked, [&]() {
        QString path = QFileDialog::getOpenFileName(&window, QStringLiteral("选择背景图片"), QString(), "Images (*.png *.jpg *.bmp)");
        if (!path.isEmpty()) {
            QImage img(path);
            if (!img.isNull()) {
                overlay->setBaseImage(img);
            }
        }
    });

    QObject::connect(radiusSpin, QOverload<int>::of(&QSpinBox::valueChanged), overlay, &HeatMapOverlay::setPointRadius);
    QObject::connect(scaleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index){
        overlay->setScaleMode(static_cast<HeatMapOverlay::ScaleMode>(scaleCombo->itemData(index).toInt()));
    });
    QObject::connect(opacitySlider, &QSlider::valueChanged, [&](int value){
        overlay->setHeatmapOpacity(value / 100.0);
    });
    QObject::connect(normalizeBox, &QCheckBox::toggled, overlay, &HeatMapOverlay::setAutoNormalize);
    QObject::connect(adaptiveRadiusBox, &QCheckBox::toggled, overlay, &HeatMapOverlay::setAdaptivePointRadius);
    QObject::connect(crosshairBox, &QCheckBox::toggled, overlay, &HeatMapOverlay::setShowCrosshair);
    QObject::connect(clearBtn, &QPushButton::clicked, overlay, &HeatMapOverlay::clearClicks);

    // 自定义事件过滤器，记录点击点
    class ClickFilter : public QObject {
    public:
        ClickFilter(HeatMapOverlay *overlay, QWidget *target, QObject *parent = nullptr)
            : QObject(parent), m_overlay(overlay), m_target(target) {}
    protected:
        bool eventFilter(QObject *watched, QEvent *event) override {
            if (watched == m_target && event->type() == QEvent::MouseButtonPress) {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
                QPointF pos = mouseEvent->position();
                QPointF overlayPos = m_overlay->mapFrom(m_target, pos.toPoint());
                QRectF rect = m_overlay->displayRect();
                if (rect.contains(overlayPos) && rect.width() > 0 && rect.height() > 0) {
                    // 将坐标归一化到实际背景区域，保证更换图片或缩放后依然对应正确位置
                    QPointF normalized((overlayPos.x() - rect.left()) / rect.width(),
                                       (overlayPos.y() - rect.top()) / rect.height());
                    m_overlay->addClick(normalized);
                }
            }
            return QObject::eventFilter(watched, event);
        }
    private:
        HeatMapOverlay *m_overlay;
        QWidget *m_target;
    };

    ClickFilter *filter = new ClickFilter(overlay, frame, &window);
    frame->installEventFilter(filter);

    window.show();
    return app.exec();
}
