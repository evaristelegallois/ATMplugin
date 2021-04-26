#include "atmdialog.h"
#include "ui_atmdialog.h"

ATMDialog::ATMDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ATMDialog)
{
    ui->setupUi(this);
}

ATMDialog::~ATMDialog()
{
    delete ui;
}
