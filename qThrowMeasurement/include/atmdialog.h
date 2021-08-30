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

#ifndef ATMDIALOG_H
#define ATMDIALOG_H

#include <ui_ATMDialog.h>

//qATM
#include "dppiecewiselinearregression.h"
#include "profileprocessor.h"
#include "atmdialog.h"

//Qt
#include <QtCharts>
#include <QChart>

using namespace QtCharts;

class ccMainAppInterface;
class ccPolyline;

/// <summary>
/// Main 2D plug-in dialog; computes automatic throw measurement from selected profiles +
/// user inputs. Allows the user to export data as img/txt, as well as display individual
/// profiles via the ATMDisplayProfileDlg.
/// </summary>
class ATMDialog : public QDialog, public Ui::ATMDialog
{
    Q_OBJECT

public:
    ATMDialog(ccMainAppInterface* app, std::vector<ccPolyline*> profiles);
    ~ATMDialog();

    /// <summary>
    /// Plots throw measurements results on a LineChart graph. Won't display anything if
    /// less than 2 profiles are used (will display a message instead).
    /// TO DO: replace IDs by y position on transect
    /// </summary>
    /// <param name="data">Throw measurement values</param>
    /// <param name="id">Corresponding profile IDs</param>
    /// <param name="n">Size of data lists</param>
    /// <returns></returns>
    QChart* createLineChart(float* data, int* id, int n) const;

    /// <summary>
    /// Takes selected profiles in input (from CC), as well as the transect + linear regression parameters 
    /// (from the dialog itself). Computes the piecewise linear reg. + calls computeThrowMeasurement() 
    /// to compute said measurement.
    /// </summary>
    void computeSegmentation();

    /// <summary>
    /// Computes throw measurement from selected profiles via computeTr().
    /// </summary>
    void computeThrowMeasurement();

    /// <summary>
    /// Computes throw measurement using the Puliti et al, 2020 equation :
    /// start (x1, y1) & end (x2, y2) of the cluster with the highest slope 
    /// are used to extrapolate linear regressions from the two other clusters.
    ///     y1 = x1*a_m + b1
    ///     y2 = x2*a_m + b2
    /// if average dip angle alpha is known, computes Tr as follows, with
    /// deltaB = |b1 - b2|, beta = atan(a_m):
    ///     Tr = (deltaB * (sin(s_alpha) * sin(beta + M_PI / 2))) / sin(s_alpha - beta)
    /// if average dip angle is unknown, Tr = |x1 - x2|
    /// </summary>
    /// <param name="x1">Curvilinear abscissa of start of highest-slope cluster</param>
    /// <param name="x2">Curvilinear abscissa of end of highest-slope cluster</param>
    /// <param name="y1">Height (y) of start of highest-slope cluster</param>
    /// <param name="y2">Height (y) of end of highest-slope cluster</param>
    /// <param name="a_m">Average slope from the non highest-slope clusters</param>
    /// <returns></returns>
    float computeTr(float x1, float x2, float y1, float y2, float a_m);

    //! export functions
    void exportDataAsTxt();
    void exportDataAsImg();

    /// <summary>
    /// import generatrix from CC database; relies on the qATMSelectEntitiesDlg, 
    /// a copy of ccEntitySelectionDlg from CC core database (credit goes to D. Girardeau)
    /// </summary>
    void importGeneratrixFromDB();

    /// <summary>
    /// INCOMPLETE FUNCTION: might be removed entirely.
    /// Imports generatrix from txt file, in case profiles and generatrix have been generated
    /// via other means.
    /// </summary>
    void importGeneratrixFromTxt();

    /// <summary>
    /// Opens the ATMDisplayProfilesDlg. Displays individual profiles as well as clustering &
    /// segmentation data for each.
    /// </summary>
    void displayProfilesDlg();

protected:

    //! Associated main window
	ccMainAppInterface* m_app;

    //// segmentation
    //! input profiles
    std::vector<ccPolyline*> m_profiles;
    //! profile -> segment & segment -> profile conversion
    std::vector<profileProcessor*> m_processors;
    //! output profile (as a list of segments)
    std::vector<SegmentLinearRegression*> m_segments;
    //! output segments
    std::vector <std::vector<SegmentLinearRegression*>> m_segmentList;

    //! index of intersection with generatrix (0 if none)
    std::vector<int> m_transectPos;
    //stores the input generatrix (transect)
    ccPolyline* m_generatrix = nullptr;

    //// clustering
    //! start & end indexes of cluster with the highest slope
    // m_sStartIdx and m_sEndIdx can be used to store every cluster start & end in case clustering data
    // need to be used (for export purposes)
    std::vector<std::vector<int>> m_startIdx, m_endIdx/*, m_sStartIdx, m_sEndIdx*/;

    //// chart display
    //! displayed chart
    QChart* m_chart;
    QChartView* m_chartView;
    //! stores throw measurement values
    float* m_y;
    //! stores profile IDs
    int* m_id;

};

#endif // ATMDIALOG_H
