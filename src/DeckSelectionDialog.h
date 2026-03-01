#ifndef DECKSELECTIONDIALOG_H
#define DECKSELECTIONDIALOG_H

#include "Deck.h"
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <vector>

/**
 * @brief 卡组选择对话框
 *
 * 显示可用卡组列表，允许用户选择或创建新卡组
 */
class DeckSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param decks 可用的卡组列表
     * @param parent 父窗口
     */
    explicit DeckSelectionDialog(const std::vector<Deck> &decks, QWidget *parent = nullptr);

    /**
     * @brief 获取用户选择的卡组名称
     * @return 卡组名称，如果未选择返回空字符串
     */
    QString getSelectedDeckName() const;

    /**
     * @brief 设置选中的卡组
     * @param name 卡组名称
     */
    void setSelectedDeckName(const QString &name);

private slots:
    void onDeckSelected(QListWidgetItem *item);
    void onDeckDoubleClicked(QListWidgetItem *item);
    void onCreateNewDeckClicked();
    void onOkClicked();

private:
    void setupUI();
    void refreshDeckList(const std::vector<Deck> &decks);

    QListWidget *m_deckListWidget{nullptr};
    QPushButton *m_createNewButton{nullptr};
    QPushButton *m_okButton{nullptr};
    QPushButton *m_cancelButton{nullptr};
    QLabel *m_promptLabel{nullptr};

    std::vector<Deck> m_decks;
    QString m_selectedDeckName;
};

#endif // DECKSELECTIONDIALOG_H
