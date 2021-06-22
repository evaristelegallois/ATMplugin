#ifndef ENVELOPEEXTRACTORDLG_H
#define ENVELOPEEXTRACTORDLG_H

#include <QDialog>

namespace Ui {
class EnvelopeExtractorDlg;
}

class EnvelopeExtractorDlg : public QDialog
{
    Q_OBJECT

public:
    explicit EnvelopeExtractorDlg(QWidget *parent = nullptr);
    ~EnvelopeExtractorDlg();

private:
    Ui::EnvelopeExtractorDlg *ui;
};

#endif // ENVELOPEEXTRACTORDLG_H
