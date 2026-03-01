#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "CardTable.h"
#include "CardManager.h"
#include "DeckManager.h"
#include "DeckRepository.h"
#include "Card.h"
#include <QMainWindow>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <vector>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onAddCardClicked();          // 添加新卡片到卡组
    void onPlayClicked();             // Play：清空桌面并重新发牌
    void onClearTableClicked();
    void onDealCountChanged(int value);
    void onSelectDeckClicked();       // 选择卡组
    void onSaveDeckClicked();
    void onCardEditRequested(CardWidget *widget);
    void onActiveDeckChanged(const QString &deckName);  // 活动卡组变更

private:
    void setupUI();
    void applyStylesheet();
    void updateDealCountLabel();
    void updateDeckInfoLabel();
    void loadLatestDeck();
    void dealCardsToTable(int count);
    void playCardToTable(const Card &card);
    void showDeckSelectionDialog();   // 显示卡组选择对话框
    void checkAndMigrateOldData();    // 检查并迁移旧数据
    void migrateOldCardsJson(const QString &sourcePath);  // 迁移旧卡组

    CardTable *m_table{nullptr};
    CardTableView *m_view{nullptr};
    DeckManager *m_deckManager{nullptr};
    DeckRepository *m_deckRepository{nullptr};

    QPushButton *m_addCardButton{nullptr};
    QPushButton *m_dealButton{nullptr};        // 原 "Deal Cards" -> "Play"
    QPushButton *m_saveDeckButton{nullptr};
    QPushButton *m_selectDeckButton{nullptr};  // 原 "Load Deck" -> "Select Deck"
    QPushButton *m_clearButton{nullptr};
    QSlider *m_dealCountSlider{nullptr};
    QLabel *m_dealCountLabel{nullptr};
    QLabel *m_deckInfoLabel{nullptr};
    QLabel *m_statusLabel{nullptr};

    int m_dealCount{5};
};

#endif // MAINWINDOW_H
