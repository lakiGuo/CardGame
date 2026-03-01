#ifndef CARDEDITDIALOG_H
#define CARDEDITDIALOG_H

#include "Card.h"
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>

class CardEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CardEditDialog(const Card &card, QWidget *parent = nullptr);

    Card getCard() const;

private slots:
    void onAccept();

private:
    void setupUI();

    QLineEdit *m_titleEdit{nullptr};
    QTextEdit *m_contentEdit{nullptr};
    QLabel *m_idLabel{nullptr};
    QLabel *m_createdLabel{nullptr};

    int m_cardId;
    QDateTime m_createdAt;
};

#endif // CARDEDITDIALOG_H
