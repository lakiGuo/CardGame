#ifndef DECKMANAGER_H
#define DECKMANAGER_H

#include "Deck.h"
#include <QObject>
#include <QString>
#include <vector>

/**
 * @brief DeckManager 管理多个卡组的运行时操作
 *
 * 负责卡组的添加、删除、切换
 * 跟踪当前活动的卡组
 */
class DeckManager : public QObject
{
    Q_OBJECT

public:
    explicit DeckManager(QObject *parent = nullptr);

    // 卡组管理
    void addDeck(const Deck &deck);
    void removeDeck(const QString &deckName);
    Deck* getDeck(const QString &deckName);
    const Deck* getDeck(const QString &deckName) const;
    std::vector<QString> getAllDeckNames() const;
    size_t deckCount() const { return m_decks.size(); }

    // 活动卡组
    void setActiveDeck(const QString &deckName);
    Deck* activeDeck();
    const Deck* activeDeck() const;
    bool hasActiveDeck() const { return m_activeDeck != nullptr; }

    // 便捷访问活动卡组
    std::vector<Card> getActiveDeckCards() const;
    size_t activeDeckCardCount() const;

signals:
    /**
     * @brief 活动卡组变更信号
     * @param deckName 新的活动卡组名称
     */
    void activeDeckChanged(const QString &deckName);

    /**
     * @brief 卡组添加信号
     * @param deckName 新添加的卡组名称
     */
    void deckAdded(const QString &deckName);

    /**
     * @brief 卡组移除信号
     * @param deckName 被移除的卡组名称
     */
    void deckRemoved(const QString &deckName);

private:
    std::vector<Deck> m_decks;
    Deck *m_activeDeck{nullptr};
};

#endif // DECKMANAGER_H
