#ifndef ATMDIALOG_H
#define ATMDIALOG_H

#include <ui_ATMDialog.h>

#include "envelopeextractor.h"

class ccMainAppInterface;
class ccPolyline;

class ATMDialog : public QDialog, public Ui::ATMDialog
{
    Q_OBJECT

public:
    ATMDialog(ccPolyline* polyline, ccMainAppInterface* app);
    //~ATMDialog();

    //! Loads parameters from persistent settings
    void loadParamsFromPersistentSettings();
    //! Saves parameters to persistent settings
    void saveParamsToPersistentSettings();

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
