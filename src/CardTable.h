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

    void enterConnectionMode();
    void exitConnectionMode();
    bool isInConnectionMode() const { return m_connectionMode; }

signals:
    void cardDrawn(CardWidget *widget);
    void connectionCreated(CardWidget *from, CardWidget *to);
    void cardEditRequested(CardWidget *widget);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    void drawConnectionLine(const QPointF &start, const QPointF &end);
    void finalizeConnection(CardWidget *from, CardWidget *to);
    CardWidget* getCardWidgetAt(const QPointF &pos) const;

    std::vector<Card> m_cardPool;
    QList<CardWidget*> m_drawnCards;

    bool m_connectionMode{false};
    CardWidget *m_connectionStart{nullptr};
    QGraphicsLineItem *m_tempConnectionLine{nullptr};

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
