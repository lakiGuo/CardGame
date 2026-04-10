#include "DeckManager.h"
#include <QDebug>

DeckManager::DeckManager(QObject *parent)
    : QObject(parent)
    , m_activeDeck(nullptr)
{
}

void DeckManager::addDeck(const Deck &deck)
{
    m_decks.push_back(deck);
    emit deckAdded(deck.name());
}

void DeckManager::removeDeck(const QString &deckName)
{
    auto it = std::remove_if(m_decks.begin(), m_decks.end(),
        [&deckName](const Deck &deck) {
            return deck.name() == deckName;
        });

    if (it != m_decks.end()) {
        m_decks.erase(it, m_decks.end());
        emit deckRemoved(deckName);

        // 如果删除的是活动卡组，清除活动卡组指针
        if (m_activeDeck && m_activeDeck->name() == deckName) {
            m_activeDeck = nullptr;
            emit activeDeckChanged(QString());
        }
    }
}

Deck* DeckManager::getDeck(const QString &deckName)
{
    for (Deck &deck : m_decks) {
        if (deck.name() == deckName) {
            return &deck;
        }
    }
    return nullptr;
}

const Deck* DeckManager::getDeck(const QString &deckName) const
{
    for (const Deck &deck : m_decks) {
        if (deck.name() == deckName) {
            return &deck;
        }
    }
    return nullptr;
}

std::vector<QString> DeckManager::getAllDeckNames() const
{
    std::vector<QString> names;
    for (const Deck &deck : m_decks) {
        names.push_back(deck.name());
    }
    return names;
}

void DeckManager::setActiveDeck(const QString &deckName)
{
    Deck *deck = getDeck(deckName);
    if (deck) {
        m_activeDeck = deck;
        emit activeDeckChanged(deckName);
        qDebug() << "Active deck set to:" << deckName;
    } else {
        qWarning() << "Failed to set active deck: deck not found:" << deckName;
    }
}

Deck* DeckManager::activeDeck()
{
    return m_activeDeck;
}

const Deck* DeckManager::activeDeck() const
{
    return m_activeDeck;
}

std::vector<Card> DeckManager::getActiveDeckCards() const
{
    if (m_activeDeck) {
        return m_activeDeck->cards();
    }
    return std::vector<Card>();
}

size_t DeckManager::activeDeckCardCount() const
{
    if (m_activeDeck) {
        return m_activeDeck->cardCount();
    }
    return 0;
}

std::vector<Card> DeckManager::searchActiveDeck(const QString &keyword) const
{
    if (m_activeDeck) {
        return m_activeDeck->searchCards(keyword);
    }
    return std::vector<Card>();
}
