#ifndef CARD_H
#define CARD_H

#include <QString>
#include <QDateTime>
#include <QUuid>
#include <QMetaType>

class Card
{
public:
    Card();
    Card(const QUuid &id, const QString &title, const QString &content);
    Card(const Card &other);
    Card &operator=(const Card &other);

    QUuid id() const { return m_id; }
    QString title() const { return m_title; }
    QString content() const { return m_content; }
    QDateTime createdAt() const { return m_createdAt; }

    void setId(const QUuid &id) { m_id = id; }
    void setTitle(const QString &title) { m_title = title; }
    void setContent(const QString &content) { m_content = content; }

private:
    QUuid m_id;
    QString m_title;
    QString m_content;
    QDateTime m_createdAt;
};

Q_DECLARE_METATYPE(Card)

#endif // CARD_H
