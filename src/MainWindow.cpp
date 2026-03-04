#include "MainWindow.h"
#include "CardEditDialog.h"
#include "DeckSelectionDialog.h"
#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QDateTime>
#include <random>
#include <numeric>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 初始化 DeckManager 和 DeckRepository
    m_deckRepository = new DeckRepository(DeckRepository::getDefaultDecksDirectory());
    m_deckManager = new DeckManager(this);

    // 连接信号
    connect(m_deckManager, &DeckManager::activeDeckChanged,
            this, &MainWindow::onActiveDeckChanged);

    // 确保卡组目录存在
    m_deckRepository->ensureDirectoryExists();

    setupUI();
    applyStylesheet();

    // 检查并迁移旧数据，然后加载最新卡组
    checkAndMigrateOldData();
    loadLatestDeck();
}

void MainWindow::setupUI()
{
    setWindowTitle("Knowledge Card Game");
    resize(1200, 800);

    // Create central widget and main layout
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Create control panel
    auto *controlPanel = new QWidget(centralWidget);
    controlPanel->setFixedWidth(280);
    controlPanel->setObjectName("controlPanel");

    auto *panelLayout = new QVBoxLayout(controlPanel);
    panelLayout->setContentsMargins(20, 20, 20, 20);
    panelLayout->setSpacing(15);

    // Title
    auto *titleLabel = new QLabel("Knowledge Cards", controlPanel);
    titleLabel->setObjectName("titleLabel");
    panelLayout->addWidget(titleLabel);

    panelLayout->addSpacing(20);

    // Deck info
    m_deckInfoLabel = new QLabel("Deck: 0 cards", controlPanel);
    m_deckInfoLabel->setObjectName("deckInfoLabel");
    m_deckInfoLabel->setAlignment(Qt::AlignCenter);
    panelLayout->addWidget(m_deckInfoLabel);

    panelLayout->addSpacing(20);

    // Deal count section
    auto *countLabel = new QLabel("Cards to Deal:", controlPanel);
    countLabel->setObjectName("sectionLabel");
    panelLayout->addWidget(countLabel);

    m_dealCountSlider = new QSlider(Qt::Horizontal, controlPanel);
    m_dealCountSlider->setObjectName("cardSlider");
    m_dealCountSlider->setRange(1, 10);
    m_dealCountSlider->setValue(m_dealCount);
    connect(m_dealCountSlider, &QSlider::valueChanged, this, &MainWindow::onDealCountChanged);
    panelLayout->addWidget(m_dealCountSlider);

    m_dealCountLabel = new QLabel(QString::number(m_dealCount), controlPanel);
    m_dealCountLabel->setObjectName("countLabel");
    m_dealCountLabel->setAlignment(Qt::AlignCenter);
    panelLayout->addWidget(m_dealCountLabel);

    panelLayout->addSpacing(15);

    // Deck actions section
    auto *deckLabel = new QLabel("Deck Actions:", controlPanel);
    deckLabel->setObjectName("sectionLabel");
    panelLayout->addWidget(deckLabel);

    m_addCardButton = new QPushButton("Add Card", controlPanel);
    m_addCardButton->setObjectName("primaryButton");
    connect(m_addCardButton, &QPushButton::clicked, this, &MainWindow::onAddCardClicked);
    panelLayout->addWidget(m_addCardButton);

    m_saveDeckButton = new QPushButton("Save Deck", controlPanel);
    m_saveDeckButton->setObjectName("fileButton");
    connect(m_saveDeckButton, &QPushButton::clicked, this, &MainWindow::onSaveDeckClicked);
    panelLayout->addWidget(m_saveDeckButton);

    m_selectDeckButton = new QPushButton("Select Deck", controlPanel);
    m_selectDeckButton->setObjectName("fileButton");
    connect(m_selectDeckButton, &QPushButton::clicked, this, &MainWindow::onSelectDeckClicked);
    panelLayout->addWidget(m_selectDeckButton);

    panelLayout->addSpacing(15);

    // Game actions section
    auto *gameLabel = new QLabel("Game:", controlPanel);
    gameLabel->setObjectName("sectionLabel");
    panelLayout->addWidget(gameLabel);

    m_dealButton = new QPushButton("Play", controlPanel);
    m_dealButton->setObjectName("secondaryButton");
    connect(m_dealButton, &QPushButton::clicked, this, &MainWindow::onPlayClicked);
    panelLayout->addWidget(m_dealButton);

    m_clearButton = new QPushButton("Clear Table", controlPanel);
    m_clearButton->setObjectName("dangerButton");
    connect(m_clearButton, &QPushButton::clicked, this, &MainWindow::onClearTableClicked);
    panelLayout->addWidget(m_clearButton);

    panelLayout->addStretch();

    // Status bar
    m_statusLabel = new QLabel("Ready. Create or load a deck to begin!", controlPanel);
    m_statusLabel->setObjectName("statusBar");
    panelLayout->addWidget(m_statusLabel);

    // Create card table
    m_table = new CardTable(this);
    m_view = new CardTableView(m_table, centralWidget);
    m_view->setObjectName("cardView");

    connect(m_table, &CardTable::cardEditRequested, this, &MainWindow::onCardEditRequested);

    mainLayout->addWidget(controlPanel);
    mainLayout->addWidget(m_view);
}

void MainWindow::applyStylesheet()
{
    QString qss = R"(
        /* Main Window */
        QMainWindow {
            background-color: #2d3238;
        }

        QWidget {
            font-family: 'Segoe UI', 'Helvetica Neue', Arial, sans-serif;
            color: #ecf0f1;
        }

        /* Control Panel */
        #controlPanel {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #34495e, stop:1 #2c3e50);
            border-right: 1px solid #1a1f25;
        }

        /* Title Label */
        #titleLabel {
            font-size: 24px;
            font-weight: bold;
            color: #3498db;
            text-align: center;
            padding: 10px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3498db, stop:1 #2980b9);
            -webkit-background-clip: text;
            border-radius: 8px;
        }

        /* Section Labels */
        #sectionLabel {
            font-size: 14px;
            font-weight: 600;
            color: #bdc3c7;
            margin-top: 10px;
        }

        /* Count Label */
        #countLabel {
            font-size: 32px;
            font-weight: bold;
            color: #3498db;
            background-color: #1a252f;
            border-radius: 8px;
            padding: 10px;
            margin-top: 5px;
        }

        /* Slider */
        #cardSlider::groove:horizontal {
            height: 8px;
            background: #1a252f;
            border-radius: 4px;
        }

        #cardSlider::handle:horizontal {
            width: 20px;
            height: 20px;
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3498db, stop:1 #2980b9);
            border-radius: 10px;
            margin: -6px 0;
        }

        #cardSlider::handle:horizontal:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #5dade2, stop:1 #3498db);
        }

        /* Primary Button */
        #primaryButton {
            background-color: #27ae60;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 20px;
            font-size: 14px;
            font-weight: 600;
        }

        #primaryButton:hover {
            background-color: #2ecc71;
        }

        #primaryButton:pressed {
            background-color: #229954;
        }

        /* Secondary Button */
        #secondaryButton {
            background-color: #f39c12;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 20px;
            font-size: 14px;
            font-weight: 600;
        }

        #secondaryButton:hover {
            background-color: #f1c40f;
        }

        #secondaryButton:checked {
            background-color: #e67e22;
        }

        /* Danger Button */
        #dangerButton {
            background-color: #c0392b;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 20px;
            font-size: 14px;
            font-weight: 600;
        }

        #dangerButton:hover {
            background-color: #e74c3c;
        }

        /* File Button */
        #fileButton {
            background-color: #7f8c8d;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 16px;
            font-size: 13px;
        }

        #fileButton:hover {
            background-color: #95a5a6;
        }

        /* Deck Info Label */
        #deckInfoLabel {
            color: #2ecc71;
            font-size: 16px;
            font-weight: bold;
            padding: 10px;
            background-color: #1a252f;
            border-radius: 6px;
            border: 2px solid #2ecc71;
        }

        /* Status Bar */
        #statusBar {
            background-color: #1a252f;
            color: #95a5a6;
            padding: 8px;
            border-radius: 4px;
            font-size: 11px;
        }

        /* Status Label */
        #statusLabel {
            background-color: #3498db;
            color: white;
            padding: 10px;
            border-radius: 6px;
            font-size: 12px;
            font-weight: 600;
        }

        /* Card View */
        #cardView {
            background-color: #2d3238;
            border: none;
        }

        /* Scrollbar */
        QScrollBar:vertical {
            background: #1a252f;
            width: 10px;
            border-radius: 5px;
        }

        QScrollBar::handle:vertical {
            background: #34495e;
            border-radius: 5px;
            min-height: 20px;
        }

        QScrollBar::handle:vertical:hover {
            background: #3498db;
        }

        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )";

    setStyleSheet(qss);
}

// ============================================================================
// New Slot Implementations
// ============================================================================

void MainWindow::onAddCardClicked()
{
    // 检查是否有活动卡组
    if (!m_deckManager->hasActiveDeck()) {
        showWarning("No Deck Selected",
            "Please select a deck first before adding cards.");
        showDeckSelectionDialog();
        return;
    }

    // 创建一个新卡片并添加到当前活动卡组
    Deck *deck = m_deckManager->activeDeck();
    int newId = static_cast<int>(deck->cardCount()) + 1;
    Card newCard(newId, "New Card", "Enter card content here...");

    CardEditDialog dialog(newCard, this);
    if (dialog.exec() == QDialog::Accepted) {
        Card updatedCard = dialog.getCard();
        deck->addCard(updatedCard);
        updateDeckInfoLabel();
        m_statusLabel->setText(QString("Card '%1' added to '%2'")
            .arg(updatedCard.title())
            .arg(deck->name()));
    }
}

void MainWindow::onPlayClicked()
{
    // 1. 检查是否有活动卡组
    if (!m_deckManager->hasActiveDeck()) {
        showDeckSelectionDialog();
        if (!m_deckManager->hasActiveDeck()) {
            return;  // 用户取消选择
        }
    }

    // 2. 清空桌面（关键变更）
    m_table->clearTable();

    // 3. 从活动卡组获取卡牌
    std::vector<Card> deck = m_deckManager->getActiveDeckCards();
    if (deck.empty()) {
        showWarning("Empty Deck",
            "The selected deck has no cards!\nPlease add cards or select a different deck.");
        return;
    }

    // 4. 随机抽取并显示
    dealCardsToTable(m_dealCount);

    // 5. 更新状态
    m_statusLabel->setText(QString("Dealt %1 cards from '%2'")
        .arg(m_dealCount)
        .arg(m_deckManager->activeDeck()->name()));
}

void MainWindow::onClearTableClicked()
{
    m_table->clearTable();
    m_statusLabel->setText("Table cleared.");
}

void MainWindow::onDealCountChanged(int value)
{
    m_dealCount = value;
    updateDealCountLabel();
}

void MainWindow::onSelectDeckClicked()
{
    showDeckSelectionDialog();
}

void MainWindow::onSaveDeckClicked()
{
    if (!m_deckManager->hasActiveDeck()) {
        showWarning("No Deck",
            "Please select a deck first.");
        return;
    }

    Deck *deck = m_deckManager->activeDeck();
    deck->updateTimestamp();

    if (m_deckRepository->saveDeck(*deck)) {
        m_statusLabel->setText(QString("Saved deck '%1' (%2 cards)")
            .arg(deck->name())
            .arg(deck->cardCount()));
    } else {
        showWarning("Save Failed",
            "Failed to save deck to file.");
    }
}

void MainWindow::onCardEditRequested(CardWidget *widget)
{
    if (!widget) return;

    CardEditDialog dialog(widget->card(), this);
    if (dialog.exec() == QDialog::Accepted) {
        Card updatedCard = dialog.getCard();
        widget->setCard(updatedCard);
        m_statusLabel->setText(QString("Card '%1' updated").arg(updatedCard.title()));
    }
}

// ============================================================================
// Helper Functions
// ============================================================================

void MainWindow::updateDealCountLabel()
{
    m_dealCountLabel->setText(QString::number(m_dealCount));
}

void MainWindow::updateDeckInfoLabel()
{
    if (m_deckManager->hasActiveDeck()) {
        const Deck *deck = m_deckManager->activeDeck();
        m_deckInfoLabel->setText(
            QString("Deck: %1 (%2 cards)")
                .arg(deck->name())
                .arg(deck->cardCount())
        );
        m_deckInfoLabel->setStyleSheet("");  // 恢复默认样式
    } else {
        m_deckInfoLabel->setText("No Deck Selected");
        m_deckInfoLabel->setStyleSheet("color: #e74c3c;");  // 红色警告
    }
}

void MainWindow::loadLatestDeck()
{
    // 列出所有可用的卡组
    std::vector<QString> deckNames = m_deckRepository->listAvailableDecks();

    if (deckNames.empty()) {
        m_statusLabel->setText("No decks found. Create or import a deck to begin!");
        updateDeckInfoLabel();
        return;
    }

    // 加载所有可用的卡组到内存
    for (const QString &name : deckNames) {
        Deck deck;
        if (m_deckRepository->loadDeck(name, deck)) {
            m_deckManager->addDeck(deck);
        }
    }

    // 如果有卡组，自动选择第一个
    if (m_deckManager->deckCount() > 0) {
        m_deckManager->setActiveDeck(deckNames[0]);
        m_statusLabel->setText(QString("Auto-loaded deck: '%1'").arg(deckNames[0]));
    }

    updateDeckInfoLabel();
}

void MainWindow::dealCardsToTable(int count)
{
    std::vector<Card> deck = m_deckManager->getActiveDeckCards();

    if (deck.empty()) {
        m_statusLabel->setText("Deck is empty!");
        return;
    }

    count = std::min(count, static_cast<int>(deck.size()));

    // 随机发牌
    std::vector<size_t> indices(deck.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(indices.begin(), indices.end(), gen);

    for (int i = 0; i < count; ++i) {
        const Card &card = deck[indices[i]];
        playCardToTable(card);
    }
}

void MainWindow::playCardToTable(const Card &card)
{
    auto *widget = new CardWidget(card);

    // 随机位置
    QRectF sceneRect = m_table->sceneRect();
    QPointF center = sceneRect.center();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<qreal> distX(-200, 200);
    std::uniform_real_distribution<qreal> distY(-150, 150);

    QPointF targetPos = center + QPointF(distX(gen), distY(gen));

    // 从边缘飞入
    qreal startX = (gen() % 2 == 0) ? -500 : 500;
    qreal startY = (gen() % static_cast<int>(sceneRect.height())) - sceneRect.height()/2;
    widget->setPos(startX, startY);

    // 使用 addCardWidget 添加卡牌（会被自动追踪到 m_drawnCards）
    m_table->addCardWidget(widget);

    // 动画到目标位置
    widget->animateTo(targetPos, 500);
}

// ============================================================================
// New Helper Functions
// ============================================================================

void MainWindow::showDeckSelectionDialog()
{
    // 加载所有可用卡组
    std::vector<QString> deckNames = m_deckRepository->listAvailableDecks();
    std::vector<Deck> decks;

    for (const QString &name : deckNames) {
        Deck deck;
        if (m_deckRepository->loadDeck(name, deck)) {
            decks.push_back(deck);
        }
    }

    // 显示对话框
    DeckSelectionDialog dialog(decks, this);

    // 如果有活动卡组，设置为当前选中
    if (m_deckManager->hasActiveDeck()) {
        dialog.setSelectedDeckName(m_deckManager->activeDeck()->name());
    }

    if (dialog.exec() == QDialog::Accepted) {
        QString selectedName = dialog.getSelectedDeckName();

        // 检查是否是新卡组
        if (m_deckManager->getDeck(selectedName) == nullptr) {
            // 创建新卡组
            Deck newDeck(selectedName, "Created on " + QDateTime::currentDateTime().toString());
            m_deckRepository->saveDeck(newDeck);
            m_deckManager->addDeck(newDeck);
            m_statusLabel->setText(QString("Created new deck: '%1'").arg(selectedName));
        }

        m_deckManager->setActiveDeck(selectedName);
    }
}

void MainWindow::onActiveDeckChanged(const QString &deckName)
{
    updateDeckInfoLabel();
    qDebug() << "Active deck changed to:" << deckName;
}

void MainWindow::checkAndMigrateOldData()
{
    // 检查旧文件位置
    QString oldPaths[] = {
        "cards.json",
        QDir::homePath() + "/.knowledgecardgame/cards.json"
    };

    for (const QString &path : oldPaths) {
        if (QFile::exists(path)) {
            qDebug() << "Found old deck file:" << path;
            migrateOldCardsJson(path);
            break;
        }
    }
}

void MainWindow::migrateOldCardsJson(const QString &sourcePath)
{
    // 1. 加载旧数据
    CardManager oldManager;
    if (oldManager.loadFromFile(sourcePath)) {
        // 2. 创建 "Default" 卡组
        Deck defaultDeck("Default", "Migrated from original cards.json");
        for (const Card &card : oldManager.cards()) {
            defaultDeck.addCard(card);
        }

        // 3. 保存到新位置
        if (m_deckRepository->saveDeck(defaultDeck)) {
            // 4. 备份原文件
            QString backupPath = sourcePath + ".backup";
            if (QFile::exists(backupPath)) {
                QFile::remove(backupPath);
            }
            QFile::rename(sourcePath, backupPath);

            qDebug() << "Migrated old deck to 'Default', backed up to:" << backupPath;
            m_statusLabel->setText("Migrated old deck to 'Default'");
        }
    }
}

void MainWindow::showWarning(const QString &title, const QString &text)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(title);
    msgBox.setText(text);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setStyleSheet(
        "QMessageBox { color: black; }"
        "QLabel { color: black; }"
        "QPushButton { color: black; }"
    );
    msgBox.exec();
}

#include "MainWindow.moc"
