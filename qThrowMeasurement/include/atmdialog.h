#ifndef ATMDIALOG_H
#define ATMDIALOG_H

#include <ui_ATMDialog.h>

#include <QtCharts>
#include <QChart>

using namespace QtCharts;

class ccMainAppInterface;
class ccPolyline;

class ATMDialog : public QDialog, public Ui::ATMDialog
{
    Q_OBJECT

public:
    ATMDialog(ccMainAppInterface* app);
    //~ATMDialog();

    QChart* createLineChart(float* data, int* id, int n) const;

protected:

	//! Path length
	double m_pathLength;
	ccMainAppInterface* app;
    QList<QChartView*> m_charts;

/*
private:
    Ui::ATMDialog *ui;
*/

};

#endif // ATMDIALOG_H
