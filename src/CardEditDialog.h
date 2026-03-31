#ifndef CARDEDITDIALOG_H
#define CARDEDITDIALOG_H

#include "Card.h"
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QTextBrowser>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QStackedWidget>

class CardEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CardEditDialog(const Card &card, QWidget *parent = nullptr);

    Card getCard() const;

private slots:
    void onAccept();
    void onPreviewToggled();

private:
    void setupUI();
    QString buildPreviewHtml(const QString &content) const;

    QLineEdit *m_titleEdit{nullptr};
    QTextEdit *m_contentEdit{nullptr};
    QStackedWidget *m_contentStack{nullptr};
    QTextBrowser *m_previewBrowser{nullptr};
    QPushButton *m_previewButton{nullptr};
    QLabel *m_idLabel{nullptr};
    QLabel *m_createdLabel{nullptr};
    bool m_previewMode{false};

    int m_cardId;
    QDateTime m_createdAt;
};

#endif // CARDEDITDIALOG_H
