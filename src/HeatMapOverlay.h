#pragma once

#include <QWidget>
#include <QImage>
#include <QColor>
#include <QVector>
#include <QPointF>
#include <QtUiPlugin/QDesignerExportWidget>

// 鼠标点击热力图覆盖控件：可以作为透明蒙版覆盖在任意图片或界面上，
// 通过绘制热力图展示用户点击的热点分布。
class QDESIGNER_WIDGET_EXPORT HeatMapOverlay : public QWidget
{
    Q_OBJECT
    // 背景图片，通常是任务场景或页面截屏
    Q_PROPERTY(QImage baseImage READ baseImage WRITE setBaseImage NOTIFY baseImageChanged)
    // 点击点列表，支持自定义权重
    Q_PROPERTY(QVector<QPointF> clickPoints READ clickPoints WRITE setClickPoints NOTIFY clickPointsChanged)
    // 热力点半径，控制模糊范围
    Q_PROPERTY(int pointRadius READ pointRadius WRITE setPointRadius NOTIFY pointRadiusChanged)
    // 热力图整体透明度，便于查看背景
    Q_PROPERTY(qreal heatmapOpacity READ heatmapOpacity WRITE setHeatmapOpacity NOTIFY heatmapOpacityChanged)
    // 是否自动归一化强度，避免过曝
    Q_PROPERTY(bool autoNormalize READ autoNormalize WRITE setAutoNormalize NOTIFY autoNormalizeChanged)
    // 是否使用归一化坐标（0~1），便于随背景缩放
    Q_PROPERTY(bool normalizedCoordinates READ normalizedCoordinates WRITE setNormalizedCoordinates NOTIFY normalizedCoordinatesChanged)
    // 颜色起点（冷色）
    Q_PROPERTY(QColor coldColor READ coldColor WRITE setColdColor NOTIFY colorRampChanged)
    // 颜色终点（热点）
    Q_PROPERTY(QColor hotColor READ hotColor WRITE setHotColor NOTIFY colorRampChanged)
    // 是否显示辅助十字线，用于调试定位
    Q_PROPERTY(bool showCrosshair READ showCrosshair WRITE setShowCrosshair NOTIFY showCrosshairChanged)

public:
    explicit HeatMapOverlay(QWidget *parent = nullptr);

    // 数据接口
    void addClick(const QPointF &pos, qreal weight = 1.0);
    void clearClicks();

    // 属性访问器
    QImage baseImage() const { return m_baseImage; }
    QVector<QPointF> clickPoints() const;
    int pointRadius() const { return m_pointRadius; }
    qreal heatmapOpacity() const { return m_heatmapOpacity; }
    bool autoNormalize() const { return m_autoNormalize; }
    bool normalizedCoordinates() const { return m_normalizedCoords; }
    QColor coldColor() const { return m_coldColor; }
    QColor hotColor() const { return m_hotColor; }
    bool showCrosshair() const { return m_showCrosshair; }

public slots:
    void setBaseImage(const QImage &image);
    void setClickPoints(const QVector<QPointF> &points);
    void setPointRadius(int radius);
    void setHeatmapOpacity(qreal value);
    void setAutoNormalize(bool on);
    void setNormalizedCoordinates(bool on);
    void setColdColor(const QColor &color);
    void setHotColor(const QColor &color);
    void setShowCrosshair(bool on);

signals:
    void baseImageChanged();
    void clickPointsChanged();
    void pointRadiusChanged();
    void heatmapOpacityChanged();
    void autoNormalizeChanged();
    void normalizedCoordinatesChanged();
    void colorRampChanged();
    void showCrosshairChanged();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void regenerateCache();
    QPointF mapToDisplay(const QPointF &pos) const;
    void colorizeHeatmap(QImage &heatmap) const;

    QImage m_baseImage;
    struct HeatPoint
    {
        QPointF pos;
        qreal weight = 1.0;
    };

    QVector<HeatPoint> m_points; // 存储点击坐标，若为归一化则范围 0~1

    int m_pointRadius = 25;
    qreal m_heatmapOpacity = 0.65;
    bool m_autoNormalize = true;
    bool m_normalizedCoords = true;
    QColor m_coldColor = QColor(0, 120, 255);
    QColor m_hotColor = QColor(255, 0, 0);
    bool m_showCrosshair = false;

    QImage m_cachedHeatmap;
    bool m_dirty = true;
};
