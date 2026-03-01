#ifndef CARD_H
#define CARD_H

#include <QString>
#include <QDateTime>
#include <QMetaType>

class Card
{
public:
    Card();
    Card(int id, const QString &title, const QString &content);
    Card(const Card &other);
    Card &operator=(const Card &other);

    int id() const { return m_id; }
    QString title() const { return m_title; }
    QString content() const { return m_content; }
    QDateTime createdAt() const { return m_createdAt; }

    void setId(int id) { m_id = id; }
    void setTitle(const QString &title) { m_title = title; }
    void setContent(const QString &content) { m_content = content; }

private:
    int m_id;
    QString m_title;
    QString m_content;
    QDateTime m_createdAt;
};

Q_DECLARE_METATYPE(Card)

#endif // CARD_H
