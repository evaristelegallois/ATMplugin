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

#ifndef PROFILEPROCESSOR_H
#define PROFILEPROCESSOR_H

//Qt
#include <QVector2D>
#include <QVector>

//qCC_db
#include <ccPolyline.h>
#include <ccGenericPointCloud.h>

//qATM
#include <segmentlinearregression.h>

#include "ccMainAppInterface.h"

/// <summary>
/// The ProfileProcessor class:
///     1) transforms input polyline(s) into (x,y) coordinates (tables) that can
///     be used by the DPPiecewiseLinearRegression class
///     2) uses segments (DPPiecewiseLinearRegression outputs) as inputs and
///     transforms them into polylines for CC display.
/// </summary>
class profileProcessor
{
public:
    /// <summary>
    /// First constructor: takes a polyline as input for (x,y) conversion.
    /// </summary>
    /// <param name="profile">Selected profile</param>
    /// <param name="generatrix">Corresponding generatrix</param>
    profileProcessor(ccPolyline* profile, ccPolyline* generatrix);

    /// <summary>
    /// Second constructor: takes a segment as input for polyline creation.
    /// </summary>
    /// <param name="segment">Selected segment</param>
    profileProcessor(SegmentLinearRegression* segment);
    ~profileProcessor();

    /// <summary>
    /// polyline (x,y,z) -> (x',y') coordinates conversion; each (x,y) point is stored as a 
    /// QVector2D* in a QVector. x' = curvilinear abscissa, y' = z.
    /// <returns>A QVector of QVector2D(x,y)</returns>
    QVector<QVector2D*> profileToXY();

    /// <summary>
    /// segment (x,y) -> polyline (x',y',z') conversion; corresponding initial polyline is used to 
    /// get missing coordinates. Displays the segmentation results as a scalar field. Displays and stores
    /// the intersection with the generatrix (fault location).
    /// </summary>
    /// <param name="segments">Segments computed from piecewise linear regression</param>
    /// <returns>The corresponding polyline with its associated point cloud & scalar field</returns>
    ccPolyline* segmentToProfile(std::vector<SegmentLinearRegression*> segments);

    /// <summary>
    /// Computes the intersection between the current profile and the generatrix using parametric
    /// curve equations of curve 1 := p1 to p2 and curve 2 := q1 to q2.
    /// </summary>
    /// <param name="p1">Starting point of curve 1</param>
    /// <param name="p2">Ending point of curve 1</param>
    /// <param name="q1">Starting point of curve 2</param>
    /// <param name="q2">Ending point of curve 2</param>
    /// <returns>The intersection (x,y) coordinates if it exists, else (0,0)</returns>
    QVector2D getIntersection(QVector2D p1, QVector2D p2, QVector2D q1, QVector2D q2);

    int getProfileID(); //gets initial profile's unique ID

private:
    //! relevant input & output polylines
    ccPolyline * m_inputProfile, * m_outputProfile, * m_inputGeneratrix;
    //! output cloud (for display)
    ccPointCloud* m_outputCloud;
    SegmentLinearRegression * m_inputSegment;
    //! input & output coordinates
    float * m_inputX, * m_inputY, * m_outputX, * m_outputY, * m_outputZ; //no need of input Z bc 2D
    //! initial profile's points
    QVector<const CCVector3 *> m_inputProfilePts; //CC uses CCVector3 to store points coordinates

    int m_profileID, m_genPtIdx;

};

#endif // PROFILEPROCESSOR_H
