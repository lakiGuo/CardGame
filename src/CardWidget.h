#ifndef CARDWIDGET_H
#define CARDWIDGET_H

#include "Card.h"
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

class CardWidget : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos DESIGNABLE false)
    Q_PROPERTY(qreal scale READ scale WRITE setScale DESIGNABLE false)

public:
    explicit CardWidget(const Card &card, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    Card card() const { return m_card; }
    void setCard(const Card &card) { m_card = card; update(); }

    bool isExpanded() const { return m_expanded; }
    void setExpanded(bool expanded);

    void addConnection(CardWidget *other);
    void removeConnection(CardWidget *other);
    void clearConnections();
    QList<CardWidget*> connections() const { return m_connections; }

    void animateTo(const QPointF &targetPos, int duration = 500);

signals:
    void doubleClicked(CardWidget *widget);
    void editRequested(CardWidget *widget);
    void connectionRequested(CardWidget *from, CardWidget *to);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    void updateAppearance();

    Card m_card;
    bool m_expanded{false};
    bool m_isDragging{false};
    bool m_isHovered{false};
    QPointF m_dragStartPos;

    static constexpr qreal s_normalWidth = 160;
    static constexpr qreal s_normalHeight = 220;
    static constexpr qreal s_expandedWidth = 400;
    static constexpr qreal s_expandedHeight = 300;
    static constexpr qreal s_cornerRadius = 12;

    QList<CardWidget*> m_connections;
    QList<QGraphicsLineItem*> m_connectionLines;
};

#endif // CARDWIDGET_H
