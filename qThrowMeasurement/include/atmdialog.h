#ifndef ATMDIALOG_H
#define ATMDIALOG_H

#include <ui_ATMDialog.h>

#include "dppiecewiselinearregression.h"
#include "profileprocessor.h"
#include "atmdialog.h"

#include <QtCharts>
#include <QChart>

using namespace QtCharts;

class ccMainAppInterface;
class ccPolyline;

class ATMDialog : public QDialog, public Ui::ATMDialog
{
    Q_OBJECT

public:
    ATMDialog(ccMainAppInterface* app, std::vector<ccPolyline*> profiles);
    //~ATMDialog();

    QChart* createLineChart(float* data, int* id, int n) const;
    void computeSegmentation();
    void computeThrowMeasurement();
    void exportDataAsTxt();
    void importGeneratrixFromDB();
    void displayProfilesDlg();

protected:

	//! Path length
	double m_pathLength;
	ccMainAppInterface* m_app;
    QList<QChartView*> m_charts;

    std::vector<ccPolyline*> m_profiles;
    std::vector<profileProcessor*> m_processors;
    std::vector<SegmentLinearRegression*> m_segments;
    std::vector <std::vector<SegmentLinearRegression*>> m_segmentList;

    ccPolyline* m_generatrix = nullptr;

    ATMDialog* m_atmDlg;

/*
private:
    Ui::ATMDialog *ui;
*/

};

#endif // ATMDIALOG_H
