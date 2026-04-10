#ifndef CARDMANAGER_H
#define CARDMANAGER_H

#include "Card.h"
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <vector>
#include <algorithm>

class CardManager
{
public:
    CardManager();

    // Load cards from JSON file
    bool loadFromFile(const QString &filePath);

    // Save cards to JSON file
    bool saveToFile(const QString &filePath);

    // Get all cards
    std::vector<Card> cards() const { return m_cards; }

    // Set cards
    void setCards(const std::vector<Card> &cards) { m_cards = cards; }

    // Add a card
    void addCard(const Card &card) { m_cards.push_back(card); }

    // Clear all cards
    void clear() { m_cards.clear(); }

    // Get card count
    size_t count() const { return m_cards.size(); }

    // Create default sample cards
    void createDefaultCards();

    // Get default JSON file path
    static QString getDefaultJsonPath();

    // Search cards by keyword (case-insensitive, matches title and content)
    std::vector<Card> searchCards(const QString &keyword) const;

private:
    Card cardFromJson(const QJsonObject &obj);
    QJsonObject cardToJson(const Card &card);

    std::vector<Card> m_cards;
};

#endif // CARDMANAGER_H
