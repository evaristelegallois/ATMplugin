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

#include <ui_ATMDisplayProfilesDlg.h>
#include "segmentlinearregression.h"

//Qt
#include <QDialog>
#include <QtCharts>
#include <QChart>

using namespace QtCharts;
class ccMainAppInterface;

/// <summary>
/// Secondary 2D plug-in dialog. Allows the user to visualize clustering + 
/// segmentation data, as well as export those in txt/img format.
/// </summary>
class ATMDisplayProfilesDlg : public QDialog, public Ui::ATMDisplayProfilesDlg
{
    Q_OBJECT

public:
    /// <summary>
    /// Constructor; takes profiles as input (as vectors of vectors of segments), as well
    /// as starting & ending indexes of the highest-slope cluster.
    /// </summary>
    explicit ATMDisplayProfilesDlg(std::vector<std::vector<SegmentLinearRegression*>> entities, 
        std::vector<std::vector<int>> startIdx, std::vector<std::vector<int>> endIdx,
        QWidget* parent = nullptr);

    /// <summary>
    /// Displays the profile at given index (in profiles list). 
    /// </summary>
    /// <param name="selectedIndex">Current index in list</param>
    /// <param name="parent">Main dialog</param>
    void displayProfile(int selectedIndex = 0, QWidget* parent = 0);
    void displayChart();
    ~ATMDisplayProfilesDlg();

    //! export functions
    void exportDataAsTxt();
    void exportAllDataAsTxt();
    void exportDataAsImg();
    void exportAllDataAsImg();

private:
    /// <summary>
    /// Plots clustering or segmentation results of a given profile on a LineChart graph.
    /// </summary>
    /// <param name="x">X coordinates (curvilinear abscissa)</param>
    /// <param name="y">Y coordinates (height)</param>
    /// <param name="n">Size of x, y lists</param>
    /// <returns></returns>
    QChart* createLineChart(float* x, float* y, int n) const;


    //the two functions below are copied from the ccEntitySelectionDlg,
    //credit goes to CC's owner D. Girardeau

    /// <summary>
    /// Returns the selected profile's index (from the profiles' list).
    /// </summary>
    /// <returns>Current list index</returns>
    int getSelectedIndex() const;

    /// <summary>
    /// Creates the profiles' list (QStringList) from selected profiles.
    /// </summary>
    /// <param name="items">The items' list</param>
    /// <param name="defaultSelectedIndex">Current selected index (0 by default)</param>
    void setItems(const QStringList& items, int defaultSelectedIndex);

    //// chart display
    //! displayed chart
    QChart* m_chart;
    QChartView* m_chartView;

    //! segmentation outputs obtained from selected profiles
    std::vector<std::vector<SegmentLinearRegression*>> m_entities;
    //! start & end indexes of highest-slope cluster
    // m_sStartIdx & m_sEndIdx could be used to store all clusters start & end idx if need be
    std::vector<std::vector<int>> m_startIdx, m_endIdx/*, m_sStartIdx, m_sEndIdx*/;

protected:
    //! Associated main window
    ccMainAppInterface* m_app;
};

#endif // ATMDISPLAYPROFILESDLG_H
