#ifndef CARDWIDGET_H
#define CARDWIDGET_H

#include "Card.h"
#include "LatexParser.h"
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSvgRenderer>
#include <QTextDocument>

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
    void setCard(const Card &card) { m_card = card; m_contentDocDirty = true; update(); }

    bool isExpanded() const { return m_expanded; }
    void setExpanded(bool expanded);

    void animateTo(const QPointF &targetPos, int duration = 500);

signals:
    void doubleClicked(CardWidget *widget);
    void editRequested(CardWidget *widget);
    void deleteRequested(CardWidget *widget);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    enum ResizeMode { None, Horizontal, Vertical, Both };
    ResizeMode hitResizeZone(const QPointF &pos) const;

    void updateAppearance();
    void rebuildLayout();
    void paintLatexContent(QPainter *painter, const QRectF &contentRect);

    struct RenderSegment {
        enum Type { Text, InlineMath, DisplayMath };
        Type type;
        QString text;       // Text: 文本内容
        QString svgPath;    // Math: SVG 文件路径
    };

    Card m_card;
    bool m_expanded{false};
    bool m_isDragging{false};
    bool m_isHovered{false};
    QPointF m_dragStartPos;

    // 动态尺寸
    qreal m_width{160};
    qreal m_height{220};

    // 缩放拖拽状态
    ResizeMode m_resizeMode{None};
    QPointF m_resizeStartPos;
    qreal m_resizeStartWidth{0};
    qreal m_resizeStartHeight{0};

    bool m_contentDocDirty{true};
    bool m_hasLatex{false};
    QList<RenderSegment> m_renderSegments;

    static constexpr qreal s_minWidth = 160;
    static constexpr qreal s_minHeight = 220;
    static constexpr qreal s_normalWidth = 160;
    static constexpr qreal s_normalHeight = 220;
    static constexpr qreal s_expandedWidth = 400;
    static constexpr qreal s_expandedHeight = 300;
    static constexpr qreal s_cornerRadius = 12;
    static constexpr qreal s_resizeZone = 8; // 缩放热区宽度
};

#endif // CARDWIDGET_H
