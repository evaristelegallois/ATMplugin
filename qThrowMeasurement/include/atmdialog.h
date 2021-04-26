#ifndef ATMDIALOG_H
#define ATMDIALOG_H

#include <QDialog>

namespace Ui {
class ATMDialog;
}

class ATMDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ATMDialog(QWidget *parent = nullptr);
    ~ATMDialog();

private:
    Ui::ATMDialog *ui;
};

#endif // ATMDIALOG_H
