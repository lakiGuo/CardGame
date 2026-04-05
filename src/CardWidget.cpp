#include "CardWidget.h"
#include "LatexRenderer.h"
#include "LatexParser.h"
#include <QPainter>
#include <QCursor>
#include <QPainterPath>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QDebug>
#include <QMenu>

CardWidget::CardWidget(const Card &card, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , m_card(card)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 100));
    shadow->setOffset(QPointF(0, 4));
    setGraphicsEffect(shadow);

    setScale(0);
    QPropertyAnimation *scaleAnim = new QPropertyAnimation(this, "scale", this);
    scaleAnim->setStartValue(0.0);
    scaleAnim->setEndValue(1.0);
    scaleAnim->setDuration(300);
    scaleAnim->setEasingCurve(QEasingCurve::OutBack);
    scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

// ============================================================================
// Resize zone detection
// ============================================================================

CardWidget::ResizeMode CardWidget::hitResizeZone(const QPointF &pos) const
{
    qreal halfW = m_width / 2;
    qreal halfH = m_height / 2;
    qreal z = s_resizeZone;

    bool atRight   = (pos.x() >= halfW - z && pos.x() <= halfW);
    bool atBottom  = (pos.y() >= halfH - z && pos.y() <= halfH);

    if (atRight && atBottom)  return Both;
    if (atRight)              return Horizontal;
    if (atBottom)             return Vertical;
    return None;
}

// ============================================================================
// Geometry
// ============================================================================

QRectF CardWidget::boundingRect() const
{
    return QRectF(-m_width / 2, -m_height / 2, m_width, m_height);
}

// ============================================================================
// Paint
// ============================================================================

void CardWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    qreal w = m_width;
    qreal h = m_height;

    painter->setRenderHint(QPainter::Antialiasing);

    // Card body
    QPainterPath path;
    path.addRoundedRect(QRectF(-w/2, -h/2, w, h), s_cornerRadius, s_cornerRadius);

    QLinearGradient grad(QPointF(-w/2, -h/2), QPointF(-w/2, h/2));
    if (m_isHovered || isSelected()) {
        grad.setColorAt(0.0, QColor(255, 250, 240));
        grad.setColorAt(1.0, QColor(255, 240, 220));
    } else {
        grad.setColorAt(0.0, QColor(250, 248, 245));
        grad.setColorAt(1.0, QColor(240, 235, 230));
    }
    painter->fillPath(path, grad);

    // Border
    QPen borderPen;
    if (isSelected()) {
        borderPen.setColor(QColor(100, 150, 255));
        borderPen.setWidth(3);
    } else if (m_isHovered) {
        borderPen.setColor(QColor(180, 160, 140));
        borderPen.setWidth(2);
    } else {
        borderPen.setColor(QColor(200, 190, 180));
        borderPen.setWidth(1);
    }
    painter->setPen(borderPen);
    painter->drawPath(path);

    // Title bar
    QPainterPath titleBar;
    titleBar.addRoundedRect(QRectF(-w/2, -h/2, w, 50), s_cornerRadius, s_cornerRadius);
    painter->setClipPath(titleBar);

    QLinearGradient titleGrad(QPointF(-w/2, -h/2), QPointF(-w/2, -h/2 + 50));
    titleGrad.setColorAt(0.0, QColor(100, 120, 180));
    titleGrad.setColorAt(1.0, QColor(70, 90, 150));
    painter->fillPath(titleBar, titleGrad);
    painter->setClipPath(QPainterPath(), Qt::NoClip);

    // Title text — 完整显示
    painter->setPen(QColor(255, 255, 255));
    QFont titleFont = painter->font();
    titleFont.setPixelSize(w > 200 ? 20 : 14);
    titleFont.setBold(true);
    painter->setFont(titleFont);

    QRectF titleRect(-w/2 + 15, -h/2, w - 30, 50);
    painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, m_card.title());

    // ID badge
    QString idText = QString("#%1").arg(m_card.id());
    QFont idFont = painter->font();
    idFont.setPixelSize(10);
    painter->setFont(idFont);
    painter->setPen(QColor(200, 220, 255));
    QRectF idRect(w/2 - 45, -h/2, 40, 50);
    painter->drawText(idRect, Qt::AlignRight | Qt::AlignVCenter, idText);

    // Content — 完整显示，不再截断
    QFont contentFont = painter->font();
    contentFont.setPixelSize(w > 200 ? 14 : 11);
    contentFont.setBold(false);
    painter->setFont(contentFont);

    QRectF contentRect(-w/2 + 15, -h/2 + 60, w - 30, h - 75);

    if (m_contentDocDirty) {
        rebuildLayout();
        m_contentDocDirty = false;
    }

    if (m_hasLatex && LatexRenderer::instance()->isAvailable()) {
        painter->setClipRect(contentRect);
        paintLatexContent(painter, contentRect);
        painter->setClipRect(QRectF(), Qt::NoClip);
    } else {
        painter->setPen(QColor(60, 60, 60));
        int flags = Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap;
        painter->drawText(contentRect, flags, m_card.content());
    }

    // Timestamp
    painter->setPen(QColor(150, 140, 130));
    QFont timeFont = painter->font();
    timeFont.setPixelSize(9);
    painter->setFont(timeFont);
    QString timeText = m_card.createdAt().toString("yyyy-MM-dd hh:mm");
    QRectF timeRect(-w/2 + 15, h/2 - 25, w - 30, 20);
    painter->drawText(timeRect, Qt::AlignLeft | Qt::AlignVCenter, timeText);

    // Resize handles (visual indicator)
    if (m_isHovered) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(100, 150, 255, 80));

        // Right edge handle
        painter->drawRect(QRectF(w/2 - 3, -h/4, 3, h/2));
        // Bottom edge handle
        painter->drawRect(QRectF(-w/4, h/2 - 3, w/2, 3));
        // Corner handle
        painter->drawRect(QRectF(w/2 - 6, h/2 - 6, 6, 6));
    }
}

// ============================================================================
// Expanded preset
// ============================================================================

void CardWidget::setExpanded(bool expanded)
{
    if (m_expanded == expanded)
        return;

    m_expanded = expanded;
    m_width  = expanded ? s_expandedWidth  : s_normalWidth;
    m_height = expanded ? s_expandedHeight : s_normalHeight;
    m_contentDocDirty = true;
    prepareGeometryChange();
    update();
}

void CardWidget::animateTo(const QPointF &targetPos, int duration)
{
    QPropertyAnimation *anim = new QPropertyAnimation(this, "pos", this);
    anim->setStartValue(pos());
    anim->setEndValue(targetPos);
    anim->setDuration(duration);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// ============================================================================
// Mouse events — resize + drag
// ============================================================================

void CardWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        ResizeMode mode = hitResizeZone(event->pos());
        if (mode != None) {
            m_resizeMode = mode;
            m_resizeStartPos = event->pos();
            m_resizeStartWidth = m_width;
            m_resizeStartHeight = m_height;
            event->accept();
            return;
        }
        m_isDragging = true;
        m_dragStartPos = event->pos();
    }
    QGraphicsItem::mousePressEvent(event);
}

void CardWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizeMode != None) {
        QPointF delta = event->pos() - m_resizeStartPos;
        prepareGeometryChange();

        if (m_resizeMode == Horizontal || m_resizeMode == Both) {
            m_width = qMax(s_minWidth, m_resizeStartWidth + delta.x());
        }
        if (m_resizeMode == Vertical || m_resizeMode == Both) {
            m_height = qMax(s_minHeight, m_resizeStartHeight + delta.y());
        }

        m_contentDocDirty = true;
        update();
        event->accept();
        return;
    }

    if (m_isDragging) {
        QGraphicsItem::mouseMoveEvent(event);
    }
}

void CardWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_resizeMode != None) {
            m_resizeMode = None;
            event->accept();
            return;
        }
        m_isDragging = false;
    }
    QGraphicsItem::mouseReleaseEvent(event);
}

void CardWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    emit editRequested(this);
}

void CardWidget::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;

    QAction *editAction = menu.addAction("Edit Card");
    menu.addSeparator();
    QAction *deleteAction = menu.addAction("Delete Card");

    QAction *selected = menu.exec(event->screenPos());
    if (selected == editAction) {
        emit editRequested(this);
    } else if (selected == deleteAction) {
        emit deleteRequested(this);
    }
}

// ============================================================================
// Hover — cursor for resize zones
// ============================================================================

void CardWidget::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    ResizeMode mode = hitResizeZone(event->pos());
    if (mode == Horizontal)     setCursor(QCursor(Qt::SizeHorCursor));
    else if (mode == Vertical) setCursor(QCursor(Qt::SizeVerCursor));
    else if (mode == Both)     setCursor(QCursor(Qt::SizeFDiagCursor));
    else                       setCursor(QCursor(Qt::OpenHandCursor));
}

void CardWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    m_isHovered = true;
    update();
}

void CardWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    m_isHovered = false;
    m_resizeMode = None;
    update();
    unsetCursor();
}

QVariant CardWidget::itemChange(GraphicsItemChange change, const QVariant &value)
{
    return QGraphicsItem::itemChange(change, value);
}

// ============================================================================
// Content layout
// ============================================================================

void CardWidget::rebuildLayout()
{
    m_renderSegments.clear();

    // 始终显示完整内容（用户可以自由调整卡片大小）
    QString displayContent = m_card.content();

    m_hasLatex = LatexParser::containsLatex(displayContent);

    if (m_hasLatex) {
        QList<ContentSegment> segments = LatexParser::parse(displayContent);
        for (const auto &seg : segments) {
            RenderSegment rs;
            if (seg.type == ContentSegment::Text) {
                rs.type = RenderSegment::Text;
                rs.text = seg.text;
            } else if (seg.type == ContentSegment::InlineMath) {
                rs.type = RenderSegment::InlineMath;
                rs.svgPath = LatexRenderer::instance()->getSvgPath(seg.text, false);
                rs.text = seg.text;
            } else {
                rs.type = RenderSegment::DisplayMath;
                rs.svgPath = LatexRenderer::instance()->getSvgPath(seg.text, true);
                rs.text = seg.text;
            }
            m_renderSegments.append(rs);
        }
    }
}

void CardWidget::paintLatexContent(QPainter *painter, const QRectF &contentRect)
{
    QFont font = painter->font();
    font.setPixelSize(m_width > 200 ? 14 : 11);
    font.setBold(false);
    painter->setFont(font);
    painter->setPen(QColor(60, 60, 60));

    QFontMetrics fm(font);
    int lineHeight = fm.height();
    qreal x = contentRect.left();
    qreal y = contentRect.top();
    qreal baseline = y + fm.ascent();
    qreal rightEdge = contentRect.right();
    qreal bottomEdge = contentRect.bottom();

    for (const auto &seg : m_renderSegments) {
        if (seg.type == RenderSegment::Text) {
            QString remaining = seg.text;
            while (!remaining.isEmpty()) {
                int spaceIdx = remaining.indexOf(' ');
                QString word;
                if (spaceIdx >= 0) {
                    word = remaining.left(spaceIdx + 1);
                    remaining = remaining.mid(spaceIdx + 1);
                } else {
                    word = remaining;
                    remaining.clear();
                }

                int wordWidth = fm.horizontalAdvance(word);
                if (x + wordWidth > rightEdge && x > contentRect.left()) {
                    x = contentRect.left();
                    y += lineHeight;
                    baseline = y + fm.ascent();
                    if (y > bottomEdge) return;
                }

                painter->drawText(QPointF(x, baseline), word);
                x += wordWidth;
            }
        } else if (seg.type == RenderSegment::InlineMath) {
            if (seg.svgPath.isEmpty()) {
                painter->drawText(QPointF(x, baseline), "$" + seg.text + "$");
                x += fm.horizontalAdvance("$" + seg.text + "$");
                continue;
            }

            QSvgRenderer svgRenderer(seg.svgPath);
            if (!svgRenderer.isValid()) continue;

            QSizeF svgSize = svgRenderer.defaultSize();
            if (svgSize.height() <= 0) continue;
            qreal aspect = svgSize.width() / svgSize.height();

            int mathH = lineHeight;
            int mathW = qMax(static_cast<int>(mathH * aspect), 1);

            if (x + mathW > rightEdge && x > contentRect.left()) {
                x = contentRect.left();
                y += lineHeight;
                baseline = y + fm.ascent();
                if (y > bottomEdge) return;
            }

            svgRenderer.render(painter, QRectF(x, y, mathW, mathH));
            x += mathW + 2;
        } else if (seg.type == RenderSegment::DisplayMath) {
            if (x > contentRect.left()) {
                x = contentRect.left();
                y += lineHeight;
                baseline = y + fm.ascent();
            }

            if (seg.svgPath.isEmpty()) continue;

            QSvgRenderer svgRenderer(seg.svgPath);
            if (!svgRenderer.isValid()) continue;

            QSizeF svgSize = svgRenderer.defaultSize();
            if (svgSize.height() <= 0) continue;
            qreal aspect = svgSize.width() / svgSize.height();

            int availW = static_cast<int>(contentRect.width());
            int displayH = qMax(static_cast<int>(availW / aspect), 1);
            int displayW = static_cast<int>(displayH * aspect);

            if (y + displayH > bottomEdge) return;

            qreal centerX = contentRect.left() + (contentRect.width() - displayW) / 2.0;
            svgRenderer.render(painter, QRectF(centerX, y, displayW, displayH));

            y += displayH + 4;
            baseline = y + fm.ascent();
            x = contentRect.left();
        }
    }
}

void CardWidget::updateAppearance()
{
    // Reserved for future use
}

#include "CardWidget.moc"
