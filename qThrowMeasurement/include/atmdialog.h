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
    float computeTr(float x1, float x2, float y1, float y2, float a_m);
    void exportDataAsTxt();
    void exportDataAsImg();
    void importGeneratrixFromDB();
    void importGeneratrixFromTxt();
    void displayProfilesDlg();

protected:

	ccMainAppInterface* m_app;
    QList<QChartView*> m_charts;

    std::vector<ccPolyline*> m_profiles;
    std::vector<profileProcessor*> m_processors;
    std::vector<SegmentLinearRegression*> m_segments;
    std::vector <std::vector<SegmentLinearRegression*>> m_segmentList;

    std::vector<std::vector<int>> m_startIdx, m_endIdx, m_sStartIdx, m_sEndIdx;
    std::vector<int> m_transectPos;

    QChart* m_chart;
    QChartView* m_chartView;

    ccPolyline* m_generatrix = nullptr;

    float* m_y;
    int* m_id;
};

#endif // ATMDIALOG_H
