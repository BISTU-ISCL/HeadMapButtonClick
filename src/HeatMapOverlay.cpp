#include "HeatMapOverlay.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QtMath>
#include <cmath>

HeatMapOverlay::HeatMapOverlay(QWidget *parent)
    : QWidget(parent)
{
    // 让控件背景透明，方便覆盖其他控件或图片
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setAttribute(Qt::WA_NoSystemBackground, true);
}

QVector<QPointF> HeatMapOverlay::clickPoints() const
{
    QVector<QPointF> result;
    result.reserve(m_points.size());
    for (const HeatPoint &p : m_points)
        result.append(p.pos);
    return result;
}

void HeatMapOverlay::setBaseImage(const QImage &image)
{
    m_baseImage = image;
    m_dirty = true;
    emit baseImageChanged();
    update();
}

void HeatMapOverlay::setClickPoints(const QVector<QPointF> &points)
{
    m_points.clear();
    for (const QPointF &p : points) {
        m_points.append({p, 1.0});
    }
    m_dirty = true;
    emit clickPointsChanged();
    update();
}

void HeatMapOverlay::setPointRadius(int radius)
{
    if (radius == m_pointRadius)
        return;
    m_pointRadius = qMax(1, radius);
    m_dirty = true;
    emit pointRadiusChanged();
    update();
}

void HeatMapOverlay::setHeatmapOpacity(qreal value)
{
    m_heatmapOpacity = qBound<qreal>(0.0, value, 1.0);
    m_dirty = true;
    emit heatmapOpacityChanged();
    update();
}

void HeatMapOverlay::setAutoNormalize(bool on)
{
    if (on == m_autoNormalize)
        return;
    m_autoNormalize = on;
    m_dirty = true;
    emit autoNormalizeChanged();
    update();
}

void HeatMapOverlay::setNormalizedCoordinates(bool on)
{
    if (on == m_normalizedCoords)
        return;
    m_normalizedCoords = on;
    m_dirty = true;
    emit normalizedCoordinatesChanged();
    update();
}

void HeatMapOverlay::setColdColor(const QColor &color)
{
    m_coldColor = color;
    m_dirty = true;
    emit colorRampChanged();
    update();
}

void HeatMapOverlay::setHotColor(const QColor &color)
{
    m_hotColor = color;
    m_dirty = true;
    emit colorRampChanged();
    update();
}

void HeatMapOverlay::setShowCrosshair(bool on)
{
    if (on == m_showCrosshair)
        return;
    m_showCrosshair = on;
    emit showCrosshairChanged();
    update();
}

void HeatMapOverlay::addClick(const QPointF &pos, qreal weight)
{
    m_points.append({pos, qMax<qreal>(0.01, weight)});
    m_dirty = true;
    update();
}

void HeatMapOverlay::clearClicks()
{
    m_points.clear();
    m_dirty = true;
    update();
}

void HeatMapOverlay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 绘制背景图片，可根据控件大小自适应缩放
    if (!m_baseImage.isNull()) {
        QImage scaled = m_baseImage.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        painter.drawImage(rect(), scaled, scaled.rect());
    }

    // 生成热力图缓存
    if (m_dirty)
        regenerateCache();

    if (!m_cachedHeatmap.isNull()) {
        painter.setOpacity(m_heatmapOpacity);
        painter.drawImage(QPoint(0, 0), m_cachedHeatmap);
        painter.setOpacity(1.0);
    }

    if (m_showCrosshair) {
        painter.setPen(QPen(Qt::yellow, 1, Qt::DashLine));
        painter.drawLine(width() / 2, 0, width() / 2, height());
        painter.drawLine(0, height() / 2, width(), height() / 2);
    }
}

void HeatMapOverlay::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // 大小变更后需要重算热力图
    m_dirty = true;
}

QPointF HeatMapOverlay::mapToDisplay(const QPointF &pos) const
{
    if (!m_normalizedCoords)
        return pos;

    if (m_baseImage.isNull())
        return QPointF(pos.x() * width(), pos.y() * height());

    // 将 0~1 的归一化坐标映射到当前显示区域
    QSizeF scaledSize = m_baseImage.size();
    scaledSize.scale(size(), Qt::KeepAspectRatioByExpanding);

    // 计算图片在控件中的偏移（居中裁剪）
    qreal offsetX = (scaledSize.width() - width()) / 2.0;
    qreal offsetY = (scaledSize.height() - height()) / 2.0;

    QPointF mapped(pos.x() * scaledSize.width() - offsetX,
                   pos.y() * scaledSize.height() - offsetY);
    return mapped;
}

void HeatMapOverlay::colorizeHeatmap(QImage &heatmap) const
{
    // 将灰度 alpha 转换为渐变色
    if (heatmap.format() != QImage::Format_ARGB32_Premultiplied)
        heatmap = heatmap.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    uchar *bits = heatmap.bits();
    const int pixelCount = heatmap.width() * heatmap.height();

    for (int i = 0; i < pixelCount; ++i) {
        int index = i * 4;
        const uchar alpha = bits[index + 3];
        if (alpha == 0) {
            bits[index + 0] = bits[index + 1] = bits[index + 2] = 0;
            continue;
        }

        // 根据 alpha 比例插值颜色
        qreal t = alpha / 255.0;
        QColor color(
            static_cast<int>(m_coldColor.red() + (m_hotColor.red() - m_coldColor.red()) * t),
            static_cast<int>(m_coldColor.green() + (m_hotColor.green() - m_coldColor.green()) * t),
            static_cast<int>(m_coldColor.blue() + (m_hotColor.blue() - m_coldColor.blue()) * t),
            alpha);

        bits[index + 0] = static_cast<uchar>(color.blue());
        bits[index + 1] = static_cast<uchar>(color.green());
        bits[index + 2] = static_cast<uchar>(color.red());
    }
}

void HeatMapOverlay::regenerateCache()
{
    if (width() <= 0 || height() <= 0) {
        m_cachedHeatmap = QImage();
        return;
    }

    QImage heatmap(size(), QImage::Format_ARGB32_Premultiplied);
    heatmap.fill(Qt::transparent);

    QPainter heatPainter(&heatmap);
    heatPainter.setRenderHint(QPainter::Antialiasing, true);
    heatPainter.setCompositionMode(QPainter::CompositionMode_Plus);

    // 遍历点击点，使用径向渐变叠加 alpha
    for (const HeatPoint &heatPoint : m_points) {
        QPointF mapped = mapToDisplay(heatPoint.pos);

        QRadialGradient g(mapped, m_pointRadius);
        QColor centerColor = QColor(255, 255, 255, static_cast<int>(180 * heatPoint.weight));
        QColor edgeColor = QColor(255, 255, 255, 0);
        g.setColorAt(0.0, centerColor);
        g.setColorAt(1.0, edgeColor);

        heatPainter.setBrush(g);
        heatPainter.setPen(Qt::NoPen);
        heatPainter.drawEllipse(mapped, m_pointRadius, m_pointRadius);
    }

    heatPainter.end();

    if (m_autoNormalize) {
        // 寻找最大 alpha 用于归一化，避免局部过曝
        uchar maxAlpha = 0;
        const uchar *bits = heatmap.constBits();
        const int pixelCount = heatmap.width() * heatmap.height();
        for (int i = 0; i < pixelCount; ++i) {
            maxAlpha = qMax(maxAlpha, bits[i * 4 + 3]);
        }
        if (maxAlpha > 0 && maxAlpha < 255) {
            QImage normalized = heatmap;
            uchar *mutableBits = normalized.bits();
            for (int i = 0; i < pixelCount; ++i) {
                int idx = i * 4 + 3;
                mutableBits[idx] = static_cast<uchar>((mutableBits[idx] / static_cast<float>(maxAlpha)) * 255);
            }
            heatmap = normalized;
        }
    }

    colorizeHeatmap(heatmap);
    m_cachedHeatmap = heatmap;
    m_dirty = false;
}
