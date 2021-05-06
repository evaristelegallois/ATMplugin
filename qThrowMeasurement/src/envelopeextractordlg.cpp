#include "envelopeextractordlg.h"
#include "ui_envelopeextractordlg.h"

EnvelopeExtractorDlg::EnvelopeExtractorDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EnvelopeExtractorDlg)
{
    ui->setupUi(this);
}

EnvelopeExtractorDlg::~EnvelopeExtractorDlg()
{
    delete ui;
}
