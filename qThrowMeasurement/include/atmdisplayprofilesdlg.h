//##########################################################################
//#                                                                        #
//#                    CLOUDCOMPARE PLUGIN: ATMPlugin                      #
//#                                                                        #
//#  This program is free software; you can redistribute it and/or modify  #
//#  it under the terms of the GNU General Public License as published by  #
//#  the Free Software Foundation; version 2 of the License.               #
//#                                                                        #
//#  This program is distributed in the hope that it will be useful,       #
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
//#  GNU General Public License for more details.                          #
//#                                                                        #
//#                      COPYRIGHT: Gabriel Parel                          #
//#                                                                        #
//##########################################################################

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
        QWidget* parent = nullptr);
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
    std::vector<std::vector<QColor*>> m_segmentColors;
    std::vector<std::vector<int>> m_startIdx, m_endIdx/*, m_sStartIdx, m_sEndIdx*/;

protected:
    ccMainAppInterface* m_app;
};

#endif // ATMDISPLAYPROFILESDLG_H
