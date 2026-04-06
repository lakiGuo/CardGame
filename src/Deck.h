#ifndef DECK_H
#define DECK_H

#include "CardManager.h"
#include <QString>
#include <QDateTime>
#include <QJsonObject>

/**
 * @brief Deck 类代表单个卡组
 *
 * 封装卡组的基本信息（名称、描述）和卡牌数据
 * 支持序列化/反序列化为 JSON 格式
 */
class Deck
{
public:
    Deck();
    explicit Deck(const QString &name, const QString &description = "");

    // 基本信息
    QString name() const { return m_name; }
    void setName(const QString &name);

    QString description() const { return m_description; }
    void setDescription(const QString &description);

    // 卡牌管理
    std::vector<Card> cards() const;
    void addCard(const Card &card);
    void removeCard(const QUuid &cardId);
    void updateCard(const QUuid &cardId, const Card &updatedCard);
    void clearCards();
    size_t cardCount() const;

    // 时间戳
    QDateTime createdAt() const { return m_createdAt; }
    QDateTime lastModified() const { return m_lastModified; }
    void updateTimestamp();

    // 序列化
    QJsonObject toJson() const;
    static Deck fromJson(const QJsonObject &json);

private:
    QString m_name;
    QString m_description;
    CardManager m_cardManager;
    QDateTime m_createdAt;
    QDateTime m_lastModified;
};

#endif // DECK_H
