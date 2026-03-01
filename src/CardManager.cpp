#include "CardManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QDebug>
#include <QDateTime>
#include <QDir>

CardManager::CardManager()
{
}

bool CardManager::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << error.errorString();
        return false;
    }

    if (!doc.isObject()) {
        qWarning() << "Invalid JSON format: expected object";
        return false;
    }

    QJsonObject root = doc.object();
    if (!root.contains("cards") || !root["cards"].isArray()) {
        qWarning() << "Invalid JSON format: missing 'cards' array";
        return false;
    }

    m_cards.clear();
    QJsonArray cardsArray = root["cards"].toArray();

    for (const QJsonValue &value : cardsArray) {
        if (value.isObject()) {
            m_cards.push_back(cardFromJson(value.toObject()));
        }
    }

    qDebug() << "Loaded" << m_cards.size() << "cards from" << filePath;
    return true;
}

bool CardManager::saveToFile(const QString &filePath)
{
    QJsonArray cardsArray;
    for (const Card &card : m_cards) {
        cardsArray.append(cardToJson(card));
    }

    QJsonObject root;
    root["cards"] = cardsArray;
    root["version"] = "1.0";
    root["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QJsonDocument doc(root);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for writing:" << filePath;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << "Saved" << m_cards.size() << "cards to" << filePath;
    return true;
}

void CardManager::createDefaultCards()
{
    m_cards = {
        Card(1, "C++", "C++ is a general-purpose programming language created by Bjarne Stroustrup. It supports multiple programming paradigms including procedural, object-oriented, and generic programming."),
        Card(2, "Qt Framework", "Qt is a cross-platform application development framework for creating graphical user interfaces as well as cross-platform applications that run on various software and hardware platforms."),
        Card(3, "Design Patterns", "Design patterns are best practices that the programmer can use to solve common problems when designing an application or system. They represent solutions to common problems."),
        Card(4, "STL", "The Standard Template Library (STL) is a set of C++ template classes to provide common programming data structures and functions such as lists, stacks, arrays, etc."),
        Card(5, "Smart Pointers", "Smart pointers are C++ objects that manage automatic memory deletion and help prevent memory leaks. Types include unique_ptr, shared_ptr, and weak_ptr."),
        Card(6, "RAII", "Resource Acquisition Is Initialization (RAII) is a programming idiom where resource allocation is tied to object lifetime, ensuring proper cleanup."),
        Card(7, "Lambda Expressions", "Lambda expressions are anonymous functions that can be used for short snippets of code that are not going to be reused and therefore do not require a name."),
        Card(8, "Templates", "Templates in C++ allow functions and classes to operate with generic types, allowing compile-time polymorphism and code reuse."),
        Card(9, "Concurrency", "C++ provides multiple ways for concurrent programming including threads, mutexes, condition variables, and async operations."),
        Card(10, "CMake", "CMake is a cross-platform build system that generates native makefiles and workspaces for various compilers and platforms.")
    };
}

QString CardManager::getDefaultJsonPath()
{
    // Check in current directory first
    QString localPath = "cards.json";
    if (QFile::exists(localPath)) {
        return localPath;
    }

    // Check in home directory
    QString homePath = QDir::homePath() + "/.knowledgecardgame/cards.json";
    return homePath;
}

Card CardManager::cardFromJson(const QJsonObject &obj)
{
    int id = obj["id"].toInt();
    QString title = obj["title"].toString();
    QString content = obj["content"].toString();

    Card card(id, title, content);

    if (obj.contains("created")) {
        QString createdStr = obj["created"].toString();
        QDateTime created = QDateTime::fromString(createdStr, Qt::ISODate);
        if (created.isValid()) {
            // Note: Card doesn't have setCreatedAt, so we use the current time
            // You may want to add a setCreatedAt method to Card class
        }
    }

    return card;
}

QJsonObject CardManager::cardToJson(const Card &card)
{
    QJsonObject obj;
    obj["id"] = card.id();
    obj["title"] = card.title();
    obj["content"] = card.content();
    obj["created"] = card.createdAt().toString(Qt::ISODate);
    return obj;
}
