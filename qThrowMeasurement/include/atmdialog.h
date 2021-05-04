#ifndef ATMDIALOG_H
#define ATMDIALOG_H

#include <ui_ATMDialog.h>

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

	//! Sets the path legnth
	void setPathLength(double l);

	//! Sets the generation step
	void setGenerationStep(double s);
	//! Sets he sections width
	void setSectionsWidth(double w);

	//! Returns the generation step
	double getGenerationStep() const;
	//! Returns the sections width
	double getSectionsWidth() const;

protected:
	void onStepChanged(double);

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
