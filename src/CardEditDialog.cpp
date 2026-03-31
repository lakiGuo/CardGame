#include "CardEditDialog.h"
#include "LatexRenderer.h"
#include "LatexParser.h"
#include <QDebug>
#include <QBuffer>

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

    // Stacked widget for edit/preview toggle
    m_contentStack = new QStackedWidget(this);

    // Page 0: Edit mode
    m_contentEdit = new QTextEdit(this);
    m_contentEdit->setPlaceholderText("Enter card content... (Use $...$ for inline math, $$...$$ for display math)");
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
    m_contentStack->addWidget(m_contentEdit);

    // Page 1: Preview mode
    m_previewBrowser = new QTextBrowser(this);
    m_previewBrowser->setReadOnly(true);
    m_previewBrowser->setOpenExternalLinks(false);
    m_previewBrowser->setStyleSheet(R"(
        QTextBrowser {
            padding: 8px;
            border: 1px solid #5dade2;
            border-radius: 4px;
            background-color: #2c3e50;
            color: #ecf0f1;
            font-size: 12px;
        }
    )");
    m_contentStack->addWidget(m_previewBrowser);

    layout->addWidget(m_contentStack);

    layout->addSpacing(15);

    // Buttons
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    // Preview toggle button
    m_previewButton = new QPushButton("Preview", this);
    m_previewButton->setStyleSheet(R"(
        QPushButton {
            background-color: #8e44ad;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 20px;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #9b59b6;
        }
    )");
    connect(m_previewButton, &QPushButton::clicked, this, &CardEditDialog::onPreviewToggled);
    buttonLayout->addWidget(m_previewButton);

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

void CardEditDialog::onPreviewToggled()
{
    m_previewMode = !m_previewMode;

    if (m_previewMode) {
        QString content = m_contentEdit->toPlainText();
        m_previewBrowser->setHtml(buildPreviewHtml(content));
        m_previewButton->setText("Edit");
    } else {
        m_previewButton->setText("Preview");
    }

    m_contentStack->setCurrentIndex(m_previewMode ? 1 : 0);
}

QString CardEditDialog::buildPreviewHtml(const QString &content) const
{
    if (!LatexParser::containsLatex(content) || !LatexRenderer::instance()->isAvailable()) {
        return QString("<pre style='white-space: pre-wrap; font-family: inherit; color: #ecf0f1;'>%1</pre>")
            .arg(content.toHtmlEscaped());
    }

    QList<ContentSegment> segments = LatexParser::parse(content);
    QString html;

    for (const auto &seg : segments) {
        if (seg.type == ContentSegment::Text) {
            html += seg.text.toHtmlEscaped().replace("\n", "<br>");
        } else {
            bool displayMode = (seg.type == ContentSegment::DisplayMath);
            QPixmap pixmap = LatexRenderer::instance()->render(seg.text, displayMode);

            if (!pixmap.isNull()) {
                int targetHeight = displayMode ? 60 : 24;
                QPixmap scaled = pixmap.scaledToHeight(targetHeight, Qt::SmoothTransformation);

                QByteArray ba;
                QBuffer buffer(&ba);
                buffer.open(QIODevice::WriteOnly);
                scaled.save(&buffer, "PNG");

                QString imgTag = QString("<img src='data:image/png;base64,%1' alt='%2' />")
                    .arg(QString(ba.toBase64()))
                    .arg(seg.text.toHtmlEscaped());

                if (displayMode) {
                    html += "<div style='text-align: center; margin: 8px 0;'>" + imgTag + "</div>";
                } else {
                    html += imgTag;
                }
            } else {
                QString delim = displayMode ? "$$" : "$";
                html += "<span style='color: #e74c3c;'>" + delim + seg.text.toHtmlEscaped() + delim + "</span>";
            }
        }
    }

    return html;
}

#include "CardEditDialog.moc"
