#include "CardTable.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QWheelEvent>
#include <QBrush>
#include <QPen>
#include <QTimer>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// CardTable Implementation
// ============================================================================

CardTable::CardTable(QObject *parent)
    : QGraphicsScene(parent)
    , m_gen(m_rd())
{
    setBackgroundBrush(QBrush(QColor(45, 50, 60)));
    setSceneRect(-2000, -1500, 4000, 3000);
}

CardTable::CardTable(const QRectF &sceneRect, QObject *parent)
    : QGraphicsScene(sceneRect, parent)
    , m_gen(m_rd())
{
    setBackgroundBrush(QBrush(QColor(45, 50, 60)));
}

CardTable::CardTable(qreal x, qreal y, qreal w, qreal h, QObject *parent)
    : QGraphicsScene(x, y, w, h, parent)
    , m_gen(m_rd())
{
    setBackgroundBrush(QBrush(QColor(45, 50, 60)));
}

void CardTable::drawCards(int count)
{
    if (m_cardPool.empty() || count <= 0)
        return;

    count = std::min(count, static_cast<int>(m_cardPool.size()));

    // Create indices and shuffle
    std::vector<size_t> indices(m_cardPool.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), m_gen);

    QRectF viewRect = sceneRect();
    QPointF center = viewRect.center();

    // Draw cards in a nice arc/fan pattern
    for (int i = 0; i < count; ++i) {
        size_t idx = indices[i];
        const Card &card = m_cardPool[static_cast<int>(idx)];

        auto *widget = new CardWidget(card);
        addItem(widget);

        // Calculate position in fan pattern
        qreal angle = -45 + (90.0 * i / std::max(1, count - 1)); // -45 to 45 degrees
        angle = angle * M_PI / 180.0;  // Convert degrees to radians

        qreal radius = 300;
        qreal x = center.x() + radius * std::sin(angle);
        qreal y = center.y() + radius * std::cos(angle) - 100;

        // Start from random edge position
        qreal startX = (m_gen() % 2 == 0) ? -viewRect.width()/2 - 100 : viewRect.width()/2 + 100;
        qreal startY = (m_gen() % static_cast<int>(viewRect.height())) - viewRect.height()/2;
        widget->setPos(startX, startY);

        // Animate to target position with staggered delay (Qt5 compatible)
        QTimer::singleShot(i * 100, [widget, x, y]() {
            widget->animateTo(QPointF(x, y), 600);
        });

        // Connect edit request signal
        connect(widget, &CardWidget::editRequested, this, [this, widget]() {
            emit cardEditRequested(widget);
        });

        // Connect delete request signal
        connect(widget, &CardWidget::deleteRequested, this, [this, widget]() {
            emit cardDeleteRequested(widget);
        });

        m_drawnCards.append(widget);
        emit cardDrawn(widget);
    }
}

void CardTable::clearTable()
{
    qDeleteAll(m_drawnCards);
    m_drawnCards.clear();
}

void CardTable::addCardWidget(CardWidget *widget)
{
    if (!widget) return;

    addItem(widget);
    m_drawnCards.append(widget);

    // Connect edit request signal
    connect(widget, &CardWidget::editRequested, this, [this, widget]() {
        emit cardEditRequested(widget);
    });

    // Connect delete request signal
    connect(widget, &CardWidget::deleteRequested, this, [this, widget]() {
        emit cardDeleteRequested(widget);
    });

    emit cardDrawn(widget);
}

void CardTable::removeCardWidget(CardWidget *widget)
{
    if (!widget) return;

    m_drawnCards.removeAll(widget);
    removeItem(widget);
    delete widget;
}

// ============================================================================
// CardTableView Implementation
// ============================================================================

CardTableView::CardTableView(CardTable *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
{
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void CardTableView::wheelEvent(QWheelEvent *event)
{
    qreal scaleFactor = 1.15;
    if (event->angleDelta().y() > 0) {
        scale(scaleFactor, scaleFactor);
    } else {
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
}

void CardTableView::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter, rect);

    // Draw grid pattern
    painter->setPen(QPen(QColor(60, 65, 75, 50), 1, Qt::DotLine));
    const int gridSize = 50;

    qreal left = std::ceil(rect.left() / gridSize) * gridSize;
    qreal top = std::ceil(rect.top() / gridSize) * gridSize;

    for (qreal x = left; x < rect.right(); x += gridSize) {
        painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
    }
    for (qreal y = top; y < rect.bottom(); y += gridSize) {
        painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
    }
}

#include "CardTable.moc"
