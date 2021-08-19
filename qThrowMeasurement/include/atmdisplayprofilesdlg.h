#ifndef ATMDISPLAYPROFILESDLG_H
#define ATMDISPLAYPROFILESDLG_H

#include <QDialog>
#include <ui_ATMDisplayProfilesDlg.h>

#include "segmentlinearregression.h"

#include <QtCharts>
#include <QChart>

using namespace QtCharts;
class ccMainAppInterface;

/*
namespace Ui {
class ATMDisplayProfilesDlg;
}
*/

class ATMDisplayProfilesDlg : public QDialog, public Ui::ATMDisplayProfilesDlg
{
    Q_OBJECT

public:
    explicit ATMDisplayProfilesDlg(std::vector<std::vector<SegmentLinearRegression*>> entities, 
        std::vector<std::vector<int>> startIdx, std::vector<std::vector<int>> endIdx, 
        std::vector<int> transectPos, QWidget* parent = nullptr);
    void displayProfile(int selectedIndex = 0, QWidget* parent = 0);
    void displayChart();
    //~ATMDisplayProfilesDlg();

    void exportDataAsTxt();
    void exportAllDataAsTxt();
    void exportDataAsImg();
    void exportAllDataAsImg();

private:
    QChart* createLineChart(float* x, float* y, int n) const;

    int getSelectedIndex() const;
    void setItems(const QStringList& items, int defaultSelectedIndex);
    //Ui::ATMDisplayProfilesDlg *ui;

    QList<QChartView*> m_charts;
    QChart* m_chart;
    QChartView* m_chartView;
    std::vector<std::vector<SegmentLinearRegression*>> m_entities;
    std::vector<std::vector<int>> m_startIdx, m_endIdx;
    std::vector<int> m_transectPos;

protected:
    ccMainAppInterface* m_app;
};

#endif // ATMDISPLAYPROFILESDLG_H
