#include "Deck.h"
#include <QJsonDocument>

Deck::Deck()
    : m_name("Untitled")
    , m_description("")
    , m_createdAt(QDateTime::currentDateTime())
    , m_lastModified(QDateTime::currentDateTime())
{
}

Deck::Deck(const QString &name, const QString &description)
    : m_name(name)
    , m_description(description)
    , m_createdAt(QDateTime::currentDateTime())
    , m_lastModified(QDateTime::currentDateTime())
{
}

void Deck::setName(const QString &name)
{
    m_name = name;
    updateTimestamp();
}

void Deck::setDescription(const QString &description)
{
    m_description = description;
    updateTimestamp();
}

std::vector<Card> Deck::cards() const
{
    return m_cardManager.cards();
}

void Deck::addCard(const Card &card)
{
    m_cardManager.addCard(card);
    updateTimestamp();
}

void Deck::removeCard(int cardId)
{
    // 获取当前卡牌列表
    std::vector<Card> currentCards = m_cardManager.cards();
    std::vector<Card> newCards;

    for (const Card &card : currentCards) {
        if (card.id() != cardId) {
            newCards.push_back(card);
        }
    }

    m_cardManager.setCards(newCards);
    updateTimestamp();
}

void Deck::clearCards()
{
    m_cardManager.clear();
    updateTimestamp();
}

size_t Deck::cardCount() const
{
    return m_cardManager.count();
}

void Deck::updateTimestamp()
{
    m_lastModified = QDateTime::currentDateTime();
}

QJsonObject Deck::toJson() const
{
    QJsonObject obj;
    obj["version"] = "2.0";
    obj["name"] = m_name;
    obj["description"] = m_description;
    obj["createdAt"] = m_createdAt.toString(Qt::ISODate);
    obj["lastModified"] = m_lastModified.toString(Qt::ISODate);

    // 序列化卡牌数组
    QJsonArray cardsArray;
    for (const Card &card : m_cardManager.cards()) {
        // 使用 CardManager 的序列化方法
        QJsonObject cardObj;
        cardObj["id"] = card.id();
        cardObj["title"] = card.title();
        cardObj["content"] = card.content();
        cardObj["created"] = card.createdAt().toString(Qt::ISODate);
        cardsArray.append(cardObj);
    }
    obj["cards"] = cardsArray;

    return obj;
}

Deck Deck::fromJson(const QJsonObject &json)
{
    Deck deck;

    // 基本信息
    deck.m_name = json["name"].toString("Untitled");
    deck.m_description = json["description"].toString("");
    deck.m_createdAt = QDateTime::fromString(json["createdAt"].toString(), Qt::ISODate);
    deck.m_lastModified = QDateTime::fromString(json["lastModified"].toString(), Qt::ISODate);

    // 加载卡牌
    if (json.contains("cards") && json["cards"].isArray()) {
        QJsonArray cardsArray = json["cards"].toArray();
        for (const QJsonValue &value : cardsArray) {
            QJsonObject cardObj = value.toObject();
            Card card(
                cardObj["id"].toInt(0),
                cardObj["title"].toString(""),
                cardObj["content"].toString("")
            );
            deck.m_cardManager.addCard(card);
        }
    }

    return deck;
}
