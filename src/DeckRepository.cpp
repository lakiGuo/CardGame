#include "DeckRepository.h"
#include <QJsonArray>
#include <QDebug>

DeckRepository::DeckRepository(const QString &rootPath)
    : m_rootPath(rootPath)
{
}

QString DeckRepository::getDefaultDecksDirectory()
{
    return QDir::homePath() + "/.knowledgecardgame/decks/";
}

bool DeckRepository::ensureDirectoryExists()
{
    QDir dir(m_rootPath);
    if (!dir.exists()) {
        return dir.mkpath(m_rootPath);
    }
    return true;
}

std::vector<QString> DeckRepository::listAvailableDecks() const
{
    std::vector<QString> deckNames;

    QDir dir(m_rootPath);
    if (!dir.exists()) {
        return deckNames;
    }

    QStringList filters;
    filters << "*.json";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Readable | QDir::Writable);

    QFileInfoList fileList = dir.entryInfoList();
    for (const QFileInfo &fileInfo : fileList) {
        // 跳过隐藏文件（如 .index.json）
        if (fileInfo.fileName().startsWith(".")) {
            continue;
        }
        QString deckName = fileNameToDeckName(fileInfo.fileName());
        deckNames.push_back(deckName);
    }

    return deckNames;
}

bool DeckRepository::saveDeck(const Deck &deck)
{
    // 确保目录存在
    if (!ensureDirectoryExists()) {
        qWarning() << "Failed to create deck directory:" << m_rootPath;
        return false;
    }

    QString filePath = getDeckFilePath(deck.name());

    // 序列化卡组
    QJsonObject jsonObj = deck.toJson();
    QJsonDocument doc(jsonObj);

    // 写入文件
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for writing:" << filePath;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << "Deck saved to:" << filePath;
    return true;
}

bool DeckRepository::loadDeck(const QString &deckName, Deck &outDeck)
{
    QString filePath = getDeckFilePath(deckName);

    QFile file(filePath);
    if (!file.exists()) {
        qWarning() << "Deck file does not exist:" << filePath;
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for reading:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error in file:" << filePath
                   << "Error:" << error.errorString();
        return false;
    }

    if (!doc.isObject()) {
        qWarning() << "Invalid JSON format in file:" << filePath;
        return false;
    }

    outDeck = Deck::fromJson(doc.object());
    qDebug() << "Deck loaded from:" << filePath;
    return true;
}

bool DeckRepository::deleteDeck(const QString &deckName)
{
    QString filePath = getDeckFilePath(deckName);

    if (!QFile::exists(filePath)) {
        qWarning() << "Cannot delete non-existent file:" << filePath;
        return false;
    }

    return QFile::remove(filePath);
}

bool DeckRepository::deckExists(const QString &deckName) const
{
    QString filePath = getDeckFilePath(deckName);
    return QFile::exists(filePath);
}

QString DeckRepository::deckNameToFileName(const QString &deckName)
{
    QString fileName = deckName.toLower();

    // 替换空格为下划线
    fileName.replace(' ', '_');

    // 替换加号为 "plus" 或直接保留
    fileName.replace('+', 'p');

    // 移除特殊字符，只保留字母、数字、下划线和 'p'
    fileName.remove(QRegExp("[^a-z0-9_p]"));

    // 添加 .json 扩展名
    return fileName + ".json";
}

QString DeckRepository::fileNameToDeckName(const QString &fileName)
{
    // 移除 .json 扩展名
    QString name = fileName;
    if (name.endsWith(".json")) {
        name = name.left(name.length() - 5);
    }

    // 将下划线替换回空格
    name.replace('_', ' ');

    // 首字母大写
    if (!name.isEmpty()) {
        name[0] = name[0].toUpper();
    }

    return name;
}

QString DeckRepository::getDeckFilePath(const QString &deckName) const
{
    return m_rootPath + "/" + deckNameToFileName(deckName);
}
