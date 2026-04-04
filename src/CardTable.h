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

signals:
    void cardDrawn(CardWidget *widget);
    void cardEditRequested(CardWidget *widget);
    void cardDeleteRequested(CardWidget *widget);

private:
    std::vector<Card> m_cardPool;
    QList<CardWidget*> m_drawnCards;

    std::random_device m_rd;
    std::mt19937 m_gen;
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
