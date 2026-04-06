#include "CardTable.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QWheelEvent>
#include <QBrush>
#include <QPen>
#include <QTimer>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
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
    m_focusedCard = nullptr;
    qDeleteAll(m_drawnCards);
    m_drawnCards.clear();
}

void CardTable::setMode(TableMode mode)
{
    if (m_mode == mode) return;

    // Exiting browse mode - unfocus if needed
    if (m_mode == TableMode::Browse && mode != TableMode::Browse) {
        if (m_focusedCard) {
            m_focusedCard->setFocused(false);
            m_focusedCard->setScale(1.0);
            m_focusedCard->setZValue(0);
            m_focusedCard->setFlag(QGraphicsItem::ItemIsMovable, true);
            m_focusedCard = nullptr;
        }
    }

    m_mode = mode;
    emit modeChanged(mode);
}

void CardTable::layoutCardsInGrid(const std::vector<Card> &cards)
{
    if (cards.empty()) return;

    static constexpr qreal CARD_WIDTH = 160;
    static constexpr qreal CARD_HEIGHT = 220;
    static constexpr qreal H_GAP = 20;
    static constexpr qreal V_GAP = 20;

    int n = static_cast<int>(cards.size());
    int cols = std::max(1, static_cast<int>(std::floor(std::sqrt(n * (CARD_WIDTH + H_GAP) / (CARD_HEIGHT + V_GAP)))));
    int rows = static_cast<int>(std::ceil(n / static_cast<double>(cols)));

    QRectF viewRect = sceneRect();
    QPointF center = viewRect.center();

    qreal gridW = cols * (CARD_WIDTH + H_GAP) - H_GAP;
    qreal gridH = rows * (CARD_HEIGHT + V_GAP) - V_GAP;
    qreal startX = center.x() - gridW / 2 + CARD_WIDTH / 2;
    qreal startY = center.y() - gridH / 2 + CARD_HEIGHT / 2;

    for (int i = 0; i < n; ++i) {
        const Card &card = cards[i];
        auto *widget = new CardWidget(card);
        addItem(widget);

        int col = i % cols;
        int row = i / cols;
        qreal targetX = startX + col * (CARD_WIDTH + H_GAP);
        qreal targetY = startY + row * (CARD_HEIGHT + V_GAP);

        // Start from off-screen
        widget->setPos(2000, 0);
        widget->setFlag(QGraphicsItem::ItemIsMovable, false);

        int delay = i * 30;
        QTimer::singleShot(delay, [widget, targetX, targetY]() {
            widget->animateTo(QPointF(targetX, targetY), 400);
        });

        // Connect signals
        connect(widget, &CardWidget::editRequested, this, [this, widget]() {
            emit cardEditRequested(widget);
        });
        connect(widget, &CardWidget::deleteRequested, this, [this, widget]() {
            emit cardDeleteRequested(widget);
        });
        connect(widget, &CardWidget::returnRequested, this, [this]() {
            unfocusCard();
        });

        m_drawnCards.append(widget);
        emit cardDrawn(widget);
    }
}

void CardTable::focusCard(CardWidget *widget)
{
    if (!widget) return;

    // If clicking a different card, instantly return the current one
    if (m_focusedCard && m_focusedCard != widget) {
        m_focusedCard->setFocused(false);
        m_focusedCard->setSize(m_focusedCardOriginalWidth, m_focusedCardOriginalHeight);
        m_focusedCard->setPos(m_focusedCardOriginalPos);
        m_focusedCard->setScale(m_focusedCardOriginalScale);
        m_focusedCard->setZValue(0);
        m_focusedCard->setFlag(QGraphicsItem::ItemIsMovable, false);
        m_focusedCard = nullptr;
    }

    // If already focused on this card, do nothing
    if (m_focusedCard == widget) {
        return;
    }

    // Save original state (position, scale, dimensions)
    m_focusedCard = widget;
    m_focusedCardOriginalPos = widget->pos();
    m_focusedCardOriginalScale = widget->scale();
    m_focusedCardOriginalWidth = widget->cardWidth();
    m_focusedCardOriginalHeight = widget->cardHeight();

    // Animate to center with zoom
    QPointF center = sceneRect().center();

    auto *posAnim = new QPropertyAnimation(widget, "pos");
    posAnim->setDuration(300);
    posAnim->setEndValue(center);
    posAnim->setEasingCurve(QEasingCurve::OutCubic);

    auto *scaleAnim = new QPropertyAnimation(widget, "scale");
    scaleAnim->setDuration(300);
    scaleAnim->setStartValue(widget->scale());
    scaleAnim->setEndValue(1.8);
    scaleAnim->setEasingCurve(QEasingCurve::OutCubic);

    auto *group = new QParallelAnimationGroup(this);
    group->addAnimation(posAnim);
    group->addAnimation(scaleAnim);
    group->start();

    widget->setZValue(1000);
    widget->setFocused(true);
}

void CardTable::unfocusCard()
{
    if (!m_focusedCard) return;

    auto *widget = m_focusedCard;
    m_focusedCard = nullptr;

    // Restore original dimensions immediately
    widget->setFocused(false);
    widget->setSize(m_focusedCardOriginalWidth, m_focusedCardOriginalHeight);

    auto *posAnim = new QPropertyAnimation(widget, "pos");
    posAnim->setDuration(300);
    posAnim->setEndValue(m_focusedCardOriginalPos);
    posAnim->setEasingCurve(QEasingCurve::OutCubic);

    auto *scaleAnim = new QPropertyAnimation(widget, "scale");
    scaleAnim->setDuration(300);
    scaleAnim->setStartValue(widget->scale());
    scaleAnim->setEndValue(m_focusedCardOriginalScale);
    scaleAnim->setEasingCurve(QEasingCurve::OutCubic);

    auto *group = new QParallelAnimationGroup(this);
    group->addAnimation(posAnim);
    group->addAnimation(scaleAnim);
    connect(group, &QParallelAnimationGroup::finished, [widget]() {
        widget->setZValue(0);
    });
    group->start();

    widget->setFlag(QGraphicsItem::ItemIsMovable, false);
}

void CardTable::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Let right-click through for context menu regardless of mode
    if (event->button() == Qt::RightButton) {
        QGraphicsScene::mousePressEvent(event);
        return;
    }

    if (m_mode == TableMode::Browse) {
        QGraphicsItem *item = itemAt(event->scenePos(), QTransform());

        if (!item) {
            // Clicked on background — if a card is focused, keep it
            return;
        }

        CardWidget *cardWidget = dynamic_cast<CardWidget*>(item);
        if (!cardWidget) return;

        if (m_focusedCard == cardWidget) {
            // Click on the already-focused card — forward to widget
            // so that the return arrow button and resize handles work
            QGraphicsScene::mousePressEvent(event);
            return;
        }

        // Click on a different card — focus it
        emit cardClickedInBrowse(cardWidget);
        return;
    }

    QGraphicsScene::mousePressEvent(event);
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

    connect(widget, &CardWidget::returnRequested, this, [this]() {
        unfocusCard();
    });

    emit cardDrawn(widget);
}

void CardTable::removeCardWidget(CardWidget *widget)
{
    if (!widget) return;

    if (m_focusedCard == widget) {
        m_focusedCard = nullptr;
    }

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
