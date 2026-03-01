#include "Card.h"

Card::Card()
    : m_id(0)
    , m_title()
    , m_content()
    , m_createdAt(QDateTime::currentDateTime())
{
}

Card::Card(int id, const QString &title, const QString &content)
    : m_id(id)
    , m_title(title)
    , m_content(content)
    , m_createdAt(QDateTime::currentDateTime())
{
}

Card::Card(const Card &other)
    : m_id(other.m_id)
    , m_title(other.m_title)
    , m_content(other.m_content)
    , m_createdAt(other.m_createdAt)
{
}

Card &Card::operator=(const Card &other)
{
    if (this != &other) {
        m_id = other.m_id;
        m_title = other.m_title;
        m_content = other.m_content;
        m_createdAt = other.m_createdAt;
    }
    return *this;
}
