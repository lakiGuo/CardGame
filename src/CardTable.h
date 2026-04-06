#ifndef CARDTABLE_H
#define CARDTABLE_H

#include "Card.h"
#include "CardWidget.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <vector>
#include <random>

class CardTable : public QGraphicsScene
{
    Q_OBJECT

public:
    enum class TableMode { Idle, Play, Browse };

    explicit CardTable(QObject *parent = nullptr);
    explicit CardTable(const QRectF &sceneRect, QObject *parent = nullptr);
    explicit CardTable(qreal x, qreal y, qreal w, qreal h, QObject *parent = nullptr);

    void setCardPool(const std::vector<Card> &cards) { m_cardPool = cards; }
    std::vector<Card> cardPool() const { return m_cardPool; }

    void drawCards(int count);
    void clearTable();

    // 添加单个卡牌到场景并追踪
    void addCardWidget(CardWidget *widget);

    // 从场景中移除单个卡牌 widget
    void removeCardWidget(CardWidget *widget);

    // Browse mode
    void setMode(TableMode mode);
    TableMode mode() const { return m_mode; }
    void layoutCardsInGrid(const std::vector<Card> &cards);
    void focusCard(CardWidget *widget);
    void unfocusCard();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

signals:
    void cardDrawn(CardWidget *widget);
    void cardEditRequested(CardWidget *widget);
    void cardDeleteRequested(CardWidget *widget);
    void modeChanged(TableMode newMode);
    void cardClickedInBrowse(CardWidget *widget);

private:
    std::vector<Card> m_cardPool;
    QList<CardWidget*> m_drawnCards;

    std::random_device m_rd;
    std::mt19937 m_gen;

    TableMode m_mode{TableMode::Idle};
    CardWidget *m_focusedCard{nullptr};
    QPointF m_focusedCardOriginalPos;
    qreal m_focusedCardOriginalScale{1.0};
    qreal m_focusedCardOriginalWidth{0};
    qreal m_focusedCardOriginalHeight{0};
};

class CardTableView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit CardTableView(CardTable *scene, QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;
};

#endif // CARDTABLE_H
