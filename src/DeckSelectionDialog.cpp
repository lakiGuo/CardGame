#include "DeckSelectionDialog.h"
#include <QMessageBox>
#include <QInputDialog>
#include <algorithm>

DeckSelectionDialog::DeckSelectionDialog(const std::vector<Deck> &decks, QWidget *parent)
    : QDialog(parent)
    , m_decks(decks)
{
    setWindowTitle("Select a Deck");
    resize(500, 400);

    setupUI();
    refreshDeckList(decks);
}

QString DeckSelectionDialog::getSelectedDeckName() const
{
    return m_selectedDeckName;
}

void DeckSelectionDialog::setSelectedDeckName(const QString &name)
{
    m_selectedDeckName = name;

    // 在列表中选中对应的项
    for (int i = 0; i < m_deckListWidget->count(); ++i) {
        QListWidgetItem *item = m_deckListWidget->item(i);
        if (item && item->data(Qt::UserRole).toString() == name) {
            m_deckListWidget->setCurrentItem(item);
            break;
        }
    }
}

void DeckSelectionDialog::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 提示标签
    m_promptLabel = new QLabel("Select a deck to play with:", this);
    m_promptLabel->setStyleSheet("font-size: 14px; font-weight: 600;");
    mainLayout->addWidget(m_promptLabel);

    // 卡组列表
    m_deckListWidget = new QListWidget(this);
    m_deckListWidget->setStyleSheet(
        "QListWidget {"
        "  background-color: #1a252f;"
        "  border: 1px solid #34495e;"
        "  border-radius: 6px;"
        "  padding: 5px;"
        "}"
        "QListWidget::item {"
        "  padding: 10px;"
        "  margin: 2px;"
        "  border-radius: 4px;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: #3498db;"
        "  color: white;"
        "}"
        "QListWidget::item:hover {"
        "  background-color: #2c3e50;"
        "}"
    );
    connect(m_deckListWidget, &QListWidget::itemClicked,
            this, &DeckSelectionDialog::onDeckSelected);
    connect(m_deckListWidget, &QListWidget::itemDoubleClicked,
            this, &DeckSelectionDialog::onDeckDoubleClicked);
    mainLayout->addWidget(m_deckListWidget);

    // 按钮布局
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    m_createNewButton = new QPushButton("Create New Deck", this);
    m_createNewButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #27ae60;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 10px 16px;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #2ecc71;"
        "}"
    );
    connect(m_createNewButton, &QPushButton::clicked,
            this, &DeckSelectionDialog::onCreateNewDeckClicked);
    buttonLayout->addWidget(m_createNewButton);

    m_deleteButton = new QPushButton("Delete Deck", this);
    m_deleteButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #c0392b;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 10px 16px;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #e74c3c;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #5a6c7d;"
        "}"
    );
    m_deleteButton->setEnabled(false);
    connect(m_deleteButton, &QPushButton::clicked,
            this, &DeckSelectionDialog::onDeleteDeckClicked);
    buttonLayout->addWidget(m_deleteButton);

    buttonLayout->addStretch();

    m_cancelButton = new QPushButton("Cancel", this);
    m_cancelButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #7f8c8d;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 10px 16px;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #95a5a6;"
        "}"
    );
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(m_cancelButton);

    m_okButton = new QPushButton("OK", this);
    m_okButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #3498db;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 10px 16px;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #5dade2;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #5a6c7d;"
        "}"
    );
    m_okButton->setEnabled(false);
    connect(m_okButton, &QPushButton::clicked,
            this, &DeckSelectionDialog::onOkClicked);
    buttonLayout->addWidget(m_okButton);

    mainLayout->addLayout(buttonLayout);
}

void DeckSelectionDialog::refreshDeckList(const std::vector<Deck> &decks)
{
    m_deckListWidget->clear();

    for (const Deck &deck : decks) {
        QString itemText = QString("%1 (%2 cards)")
            .arg(deck.name())
            .arg(deck.cardCount());

        QListWidgetItem *item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, deck.name());

        // 添加描述作为工具提示
        if (!deck.description().isEmpty()) {
            item->setToolTip(deck.description());
        }

        m_deckListWidget->addItem(item);
    }

    // 如果有卡组，默认选中第一个
    if (m_deckListWidget->count() > 0) {
        m_deckListWidget->setCurrentRow(0);
        onDeckSelected(m_deckListWidget->item(0));
    }
}

void DeckSelectionDialog::onDeckSelected(QListWidgetItem *item)
{
    if (item) {
        m_selectedDeckName = item->data(Qt::UserRole).toString();
        m_okButton->setEnabled(true);
        m_deleteButton->setEnabled(true);
    }
}

void DeckSelectionDialog::onDeckDoubleClicked(QListWidgetItem *item)
{
    if (item) {
        onDeckSelected(item);
        accept();
    }
}

void DeckSelectionDialog::onCreateNewDeckClicked()
{
    bool ok;
    QInputDialog *dialog = new QInputDialog(this);
    dialog->setWindowTitle("Create New Deck");
    dialog->setLabelText("Enter deck name:");
    dialog->setTextValue("");
    dialog->setStyleSheet(
        "QInputDialog { color: black; }"
        "QLabel { color: black; }"
        "QPushButton { color: black; }"
        "QLineEdit { color: black; }"
    );
    dialog->exec();
    ok = dialog->result() == QDialog::Accepted;
    QString deckName = dialog->textValue();
    delete dialog;

    if (ok && !deckName.isEmpty()) {
        m_selectedDeckName = deckName;

        // 通知调用者创建新卡组
        // 注意：实际的创建逻辑将在 MainWindow 中处理
        accept();
    }
}

void DeckSelectionDialog::onOkClicked()
{
    if (!m_selectedDeckName.isEmpty()) {
        accept();
    }
}

void DeckSelectionDialog::onDeleteDeckClicked()
{
    if (m_selectedDeckName.isEmpty()) return;

    QMessageBox confirmBox(this);
    confirmBox.setWindowTitle("Delete Deck");
    confirmBox.setText(QString("Are you sure you want to delete deck '%1'?").arg(m_selectedDeckName));
    confirmBox.setInformativeText("This will permanently delete the deck file from disk.");
    confirmBox.setIcon(QMessageBox::Warning);
    confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmBox.setDefaultButton(QMessageBox::No);
    confirmBox.setStyleSheet(
        "QMessageBox { color: black; }"
        "QLabel { color: black; }"
        "QPushButton { color: black; }"
    );

    if (confirmBox.exec() == QMessageBox::Yes) {
        emit deckDeleteRequested(m_selectedDeckName);

        // 从列表中移除
        for (int i = 0; i < m_deckListWidget->count(); ++i) {
            QListWidgetItem *item = m_deckListWidget->item(i);
            if (item && item->data(Qt::UserRole).toString() == m_selectedDeckName) {
                delete m_deckListWidget->takeItem(i);
                break;
            }
        }

        // 从内部列表中移除
        m_decks.erase(
            std::remove_if(m_decks.begin(), m_decks.end(),
                [this](const Deck &deck) { return deck.name() == m_selectedDeckName; }),
            m_decks.end()
        );

        m_selectedDeckName.clear();
        m_okButton->setEnabled(false);
        m_deleteButton->setEnabled(false);

        if (m_deckListWidget->count() > 0) {
            m_deckListWidget->setCurrentRow(0);
            onDeckSelected(m_deckListWidget->item(0));
        }
    }
}
