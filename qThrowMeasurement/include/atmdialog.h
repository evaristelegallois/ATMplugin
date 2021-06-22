#ifndef ATMDIALOG_H
#define ATMDIALOG_H

#include <ui_ATMDialog.h>

class ccMainAppInterface;
class ccPolyline;

class ATMDialog : public QDialog, public Ui::ATMDialog
{
    Q_OBJECT

public:
    ATMDialog(ccMainAppInterface* app);
    //~ATMDialog();

protected:

	//! Path length
	double m_pathLength;
	ccMainAppInterface* app;

/*
private:
    Ui::ATMDialog *ui;
*/

};

#endif // ATMDIALOG_H
