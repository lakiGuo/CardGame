#include "CardWidget.h"
#include <QPainter>
#include <QCursor>
#include <QPainterPath>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QDebug>

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

QRectF CardWidget::boundingRect() const
{
    qreal w = m_expanded ? s_expandedWidth : s_normalWidth;
    qreal h = m_expanded ? s_expandedHeight : s_normalHeight;
    return QRectF(-w/2, -h/2, w, h);
}

void CardWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    qreal w = m_expanded ? s_expandedWidth : s_normalWidth;
    qreal h = m_expanded ? s_expandedHeight : s_normalHeight;

    painter->setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(QRectF(-w/2, -h/2, w, h), s_cornerRadius, s_cornerRadius);

    // Background gradient
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

    // Title text
    painter->setPen(QColor(255, 255, 255));
    QFont titleFont = painter->font();
    titleFont.setPixelSize(m_expanded ? 20 : 14);
    titleFont.setBold(true);
    painter->setFont(titleFont);

    QRectF titleRect(-w/2 + 15, -h/2, w - 30, 50);
    painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter,
                      m_card.title().left(m_expanded ? 30 : 18));

    // ID badge
    QString idText = QString("#%1").arg(m_card.id());
    QFont idFont = painter->font();
    idFont.setPixelSize(10);
    painter->setFont(idFont);
    painter->setPen(QColor(200, 220, 255));
    QRectF idRect(w/2 - 45, -h/2, 40, 50);
    painter->drawText(idRect, Qt::AlignRight | Qt::AlignVCenter, idText);

    // Content
    painter->setPen(QColor(60, 60, 60));
    QFont contentFont = painter->font();
    contentFont.setPixelSize(m_expanded ? 14 : 11);
    contentFont.setBold(false);
    painter->setFont(contentFont);

    QRectF contentRect(-w/2 + 15, -h/2 + 60, w - 30, h - 75);
    int flags = Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap;
    QString displayContent = m_card.content();
    if (!m_expanded) {
        displayContent = displayContent.left(80);
        if (displayContent.length() < m_card.content().length()) {
            displayContent += "...";
        }
    }
    painter->drawText(contentRect, flags, displayContent);

    // Timestamp
    painter->setPen(QColor(150, 140, 130));
    QFont timeFont = painter->font();
    timeFont.setPixelSize(9);
    painter->setFont(timeFont);
    QString timeText = m_card.createdAt().toString("yyyy-MM-dd hh:mm");
    QRectF timeRect(-w/2 + 15, h/2 - 25, w - 30, 20);
    painter->drawText(timeRect, Qt::AlignLeft | Qt::AlignVCenter, timeText);
}

void CardWidget::setExpanded(bool expanded)
{
    if (m_expanded == expanded)
        return;

    m_expanded = expanded;
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

void CardWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragStartPos = event->pos();
    }
    QGraphicsItem::mousePressEvent(event);
}

void CardWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isDragging) {
        QGraphicsItem::mouseMoveEvent(event);
    }
}

void CardWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
    }
    QGraphicsItem::mouseReleaseEvent(event);
}

void CardWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    emit editRequested(this);
}

void CardWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    m_isHovered = true;
    update();
    setCursor(QCursor(Qt::OpenHandCursor));
}

void CardWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    m_isHovered = false;
    update();
    unsetCursor();
}

QVariant CardWidget::itemChange(GraphicsItemChange change, const QVariant &value)
{
    return QGraphicsItem::itemChange(change, value);
}

#include "CardWidget.moc"
