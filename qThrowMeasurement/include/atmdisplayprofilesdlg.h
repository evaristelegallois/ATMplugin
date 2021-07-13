#ifndef ATMDISPLAYPROFILESDLG_H
#define ATMDISPLAYPROFILESDLG_H

#include <QDialog>
#include <ui_ATMDisplayProfilesDlg.h>

#include "segmentlinearregression.h"

#include <QtCharts>
#include <QChart>

using namespace QtCharts;

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
        QWidget* parent = nullptr);
    void displayProfile(int selectedIndex = 0, QWidget* parent = 0);
    //~ATMDisplayProfilesDlg();

    void exportDataAsTxt();
    void exportAllDataAsTxt();
    void exportDataAsImg();

private:
    QChart* createLineChart(float* x, float* y, int n) const;
    int getSelectedIndex() const;
    void setItems(const QStringList& items, int defaultSelectedIndex);
    //Ui::ATMDisplayProfilesDlg *ui;

    QList<QChartView*> m_charts;
    QChart* m_chart;
    QChartView* m_chartView;
    std::vector<std::vector<SegmentLinearRegression*>> m_entities;
};

#endif // ATMDISPLAYPROFILESDLG_H
