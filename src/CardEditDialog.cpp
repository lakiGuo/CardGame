#include "CardEditDialog.h"
#include <QDebug>

CardEditDialog::CardEditDialog(const Card &card, QWidget *parent)
    : QDialog(parent)
    , m_cardId(card.id())
    , m_createdAt(card.createdAt())
{
    setWindowTitle("Edit Card");
    resize(500, 400);

    setupUI();

    m_titleEdit->setText(card.title());
    m_contentEdit->setPlainText(card.content());
    m_idLabel->setText(QString("#%1").arg(card.id()));
    m_createdLabel->setText(card.createdAt().toString("yyyy-MM-dd hh:mm:ss"));
}

void CardEditDialog::setupUI()
{
    auto *layout = new QVBoxLayout(this);

    // ID and creation info
    auto *infoLayout = new QHBoxLayout();
    m_idLabel = new QLabel("", this);
    m_idLabel->setStyleSheet("QLabel { font-weight: bold; color: #3498db; font-size: 14px; }");

    m_createdLabel = new QLabel("", this);
    m_createdLabel->setStyleSheet("QLabel { color: #95a5a6; font-size: 11px; }");

    infoLayout->addWidget(m_idLabel);
    infoLayout->addStretch();
    infoLayout->addWidget(m_createdLabel);
    layout->addLayout(infoLayout);

    layout->addSpacing(15);

    // Title field
    auto *titleLayout = new QHBoxLayout();
    auto *titleLabel = new QLabel("Title:", this);
    titleLabel->setStyleSheet("QLabel { font-weight: bold; color: #ecf0f1; }");
    titleLayout->addWidget(titleLabel);

    m_titleEdit = new QLineEdit(this);
    m_titleEdit->setPlaceholderText("Enter card title...");
    m_titleEdit->setStyleSheet(R"(
        QLineEdit {
            padding: 8px;
            border: 1px solid #5dade2;
            border-radius: 4px;
            background-color: #2c3e50;
            color: #ecf0f1;
            font-size: 13px;
        }
        QLineEdit:focus {
            border: 2px solid #3498db;
        }
    )");
    titleLayout->addWidget(m_titleEdit);
    layout->addLayout(titleLayout);

    layout->addSpacing(10);

    // Content field
    auto *contentLabel = new QLabel("Content:", this);
    contentLabel->setStyleSheet("QLabel { font-weight: bold; color: #ecf0f1; }");
    layout->addWidget(contentLabel);

    m_contentEdit = new QTextEdit(this);
    m_contentEdit->setPlaceholderText("Enter card content...");
    m_contentEdit->setStyleSheet(R"(
        QTextEdit {
            padding: 8px;
            border: 1px solid #5dade2;
            border-radius: 4px;
            background-color: #2c3e50;
            color: #ecf0f1;
            font-size: 12px;
        }
        QTextEdit:focus {
            border: 2px solid #3498db;
        }
    )");
    layout->addWidget(m_contentEdit);

    layout->addSpacing(15);

    // Buttons
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    auto *cancelButton = new QPushButton("Cancel", this);
    cancelButton->setStyleSheet(R"(
        QPushButton {
            background-color: #e74c3c;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 20px;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #c0392b;
        }
    )");
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelButton);

    auto *saveButton = new QPushButton("Save", this);
    saveButton->setStyleSheet(R"(
        QPushButton {
            background-color: #27ae60;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 20px;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #2ecc71;
        }
    )");
    connect(saveButton, &QPushButton::clicked, this, &CardEditDialog::onAccept);
    buttonLayout->addWidget(saveButton);

    layout->addLayout(buttonLayout);

    // Set dialog background
    setStyleSheet(R"(
        CardEditDialog {
            background-color: #34495e;
        }
    )");
}

Card CardEditDialog::getCard() const
{
    Card card(m_cardId, m_titleEdit->text(), m_contentEdit->toPlainText());
    // Note: Card doesn't have setCreatedAt, so it uses current time
    // You may want to add this method to preserve original creation time
    return card;
}

void CardEditDialog::onAccept()
{
    QString title = m_titleEdit->text().trimmed();
    if (title.isEmpty()) {
        m_titleEdit->setFocus();
        return;
    }

    accept();
}

#include "CardEditDialog.moc"
