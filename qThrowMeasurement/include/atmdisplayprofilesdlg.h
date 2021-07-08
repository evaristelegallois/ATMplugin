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
    explicit ATMDisplayProfilesDlg(QWidget *parent = nullptr);
    void displayProfile(std::vector<SegmentLinearRegression*> entities,
        int selectedIndex = 0, QWidget* parent = 0);
    //~ATMDisplayProfilesDlg();

private:
    QChart* createLineChart(float* x, float* y, int n) const;
    int getSelectedIndex() const;
    void setItems(const QStringList& items, int defaultSelectedIndex);
    //Ui::ATMDisplayProfilesDlg *ui;

    QList<QChartView*> m_charts;
    QChartView* m_chartView;
};

#endif // ATMDISPLAYPROFILESDLG_H
