#ifndef DECKREPOSITORY_H
#define DECKREPOSITORY_H

#include "Deck.h"
#include <QString>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <vector>

/**
 * @brief DeckRepository 负责卡组的文件系统操作
 *
 * 管理卡组文件的读写、目录管理
 */
class DeckRepository
{
public:
    /**
     * @brief 构造函数
     * @param rootPath 卡组存储根目录
     */
    explicit DeckRepository(const QString &rootPath);

    /**
     * @brief 获取默认的卡组目录路径
     * @return ~/.knowledgecardgame/decks/
     */
    static QString getDefaultDecksDirectory();

    /**
     * @brief 确保卡组目录存在
     * @return 成功返回 true
     */
    bool ensureDirectoryExists();

    /**
     * @brief 列出所有可用的卡组名称
     * @return 卡组名称列表
     */
    std::vector<QString> listAvailableDecks() const;

    /**
     * @brief 保存卡组到文件
     * @param deck 要保存的卡组
     * @return 成功返回 true
     */
    bool saveDeck(const Deck &deck);

    /**
     * @brief 从文件加载卡组
     * @param deckName 卡组名称
     * @param outDeck 输出参数，加载的卡组
     * @return 成功返回 true
     */
    bool loadDeck(const QString &deckName, Deck &outDeck);

    /**
     * @brief 删除卡组文件
     * @param deckName 卡组名称
     * @return 成功返回 true
     */
    bool deleteDeck(const QString &deckName);

    /**
     * @brief 检查卡组是否存在
     * @param deckName 卡组名称
     * @return 存在返回 true
     */
    bool deckExists(const QString &deckName) const;

    /**
     * @brief 将卡组名称转换为文件名
     * @param deckName "C++ Basics" -> "cpp_basics.json"
     * @return 文件名
     */
    static QString deckNameToFileName(const QString &deckName);

    /**
     * @brief 将文件名转换为卡组名称
     * @param fileName "cpp_basics.json" -> "C++ Basics"
     * @return 卡组名称
     */
    static QString fileNameToDeckName(const QString &fileName);

    /**
     * @brief 获取卡组文件的完整路径
     * @param deckName 卡组名称
     * @return 完整路径
     */
    QString getDeckFilePath(const QString &deckName) const;

    /**
     * @brief 获取根目录路径
     * @return 根目录路径
     */
    QString getRootPath() const { return m_rootPath; }

private:
    QString m_rootPath;
};

#endif // DECKREPOSITORY_H
