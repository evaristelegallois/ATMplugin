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

#ifndef SEGMENTLINEARREGRESSION_H
#define SEGMENTLINEARREGRESSION_H

//Qt
#include <QVector2D>
#include <QVector3D>

//qCC_db
#include <ccPointCloud.h>
#include <ccPolyline.h>

/// <summary>
/// Container class; stores all the piecewise linear regression parameters +
/// intersection with transect (if any) as well as associated profile ID.
/// </summary>
class SegmentLinearRegression
{
public:
    /// <summary>
    /// Default constructor: a segment has a start & end indexes, used to get the corresponding
    /// points as follows: (x[start], y[start) -> (x[end], y[end]). 
    /// </summary>
    /// <param name="startInd">Starting index</param>
    /// <param name="endInd">Ending index</param>
    /// <param name="x">X coordinates of entire associated profile</param>
    /// <param name="y">Y coordinates of entire associated profile</param>
    SegmentLinearRegression(int startInd, int endInd, float* x, float* y);
    ~SegmentLinearRegression();

    //! getters; see corresponding functions in DPPiecewiseLinearRegression
    float getVar();
    QVector2D getAltVar(); //????
    float getRSquare();
    float getSlope();
    float getIntercept();
    QVector2D getStart(); //point
    QVector2D getEnd(); //point
    int getStartIndex();
    int getEndIndex();
    int getSize();
    float getAssociatedP();
    int getUniqueSharedID();
    int getTransectPosition();

    /// <summary>
    /// Gets the point (x[i], y[i]).
    /// </summary>
    /// <param name="index">Index of the desired point</param>
    /// <returns>A QVector2D(x,y)</returns>
    QVector2D* getPoint(int index);

    QVector3D getColor();

    //! setters
    void setVar(float var);
    void setAltVar(QVector2D altVar);
    void setRSquare(float rsquare);
    void setSlope(float slope);
    void setIntercept(float intercept);
    void setStartIndex(int startInd);
    void setEndIndex(int endInd);
    void setAssociatedP(float p);
    void setUniqueSharedID(int id);
    void setTransectPosition(int idx);

    void setColor(QVector3D color);

private:
    //! a = slope, b = intercept, rsquare = r², var = variance, p = p value
    float m_a, m_b, m_r, m_rsquare, m_var, m_p;
    //! start index, end index, segment size, profile ID, intercept with transect (if any)
    int m_startInd, m_endInd, m_size, m_id, m_transectPos;

    //! starting & ending points
    QVector2D m_start, m_end, m_altVar; //altVar????
    QVector3D m_color;

    //! segment points
    QVector<QVector2D*> m_points;
};

#endif // SEGMENTLINEARREGRESSION_H
