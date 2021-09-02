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
//#                COPYRIGHT: Gabriel Parel, Sophie Viseur                 #
//#                                                                        #
//##########################################################################

#ifndef DPPIECEWISELINEARREGRESSION_H
#define DPPIECEWISELINEARREGRESSION_H

#include "segmentlinearregression.h"

/// <summary>
/// ATM plug-in core class; computes piecewise linear regression on (x,y) coordinates
/// with relevant parameters (size, p value, jumps, scoring funtion).
/// </summary>
class dPPiecewiseLinearRegression
{
public:
    /// <summary>
    /// Main constructor; takes relevant parameters as inputs.
    /// </summary>
    /// <param name="x">X coordinate of given profile</param>
    /// <param name="y">Y coordinate of given profile</param>
    /// <param name="n">Size of given profile (nb of points)</param>
    /// <param name="p">P value</param>
    /// <param name="j">0 if continuous segmentation, 1 else</param>
    /// <param name="type">Scoring function, variance-based is the default</param>
    dPPiecewiseLinearRegression(float* x, float* y, int n, float p, 
        int j = 0, const char* type = "var");
    //~dPPiecewiseLinearRegression();


    //// STATISTICAL COMPUTATION FUNCTIONS

    /// <summary>
    /// Computes a linear regression y = a*x + b on a given segment [i,j].
    /// </summary>
    /// <param name="x">X coordinate of segment [i,j]</param>
    /// <param name="i">Starting index of segment [i,j]</param>
    /// <param name="j">Ending index of segment [i,j]</param>
    /// <returns>Y coordinate (as such: y = a*x + b)</returns>
    float computeModel(float x, int i, int j);

    /// <summary>
    /// Computes the slope of the segment [i,j], using x, y inputs (see constructor).
    /// With segment [i,j] being of the form y = a*x + b, returns a.
    /// </summary>
    /// <param name="i">Start index of segment [i,j]</param>
    /// <param name="j">End index of segment [i,j]</param>
    /// <returns>Computed slope of the segment [i,j]</returns>
    float computeSlope(int i, int j);

    /// <summary>
    /// Computes the intercept of the segment [i,j], using x, y inputs (see constructor).
    /// With segment [i,j] being of the form y = a*x + b, returns b.
    /// </summary>
    /// <param name="i">Start index of segment [i,j]</param>
    /// <param name="j">End index of segment [i,j]</param>
    /// <returns>Computed intercept of the segment [i,j]</returns>
    float computeIntercept(int i, int j);

    /// <summary>
    /// Scoring function r²-based. Used in the segmentation phase of the piecewise
    /// linear regression to compute max scores (allows the generating of break points).
    /// Score is not normalized.
    /// </summary>
    /// <param name="i">Start index of segment [i,j]</param>
    /// <param name="j">End index of segment [i,j]</param>
    /// <returns>R²-based score of segment [i,j]</returns>
    float computeRScore(int i, int j);

    /// <summary>
    /// Scoring function variance-based. Used in the segmentation phase of the piecewise
    /// linear regression to compute max scores (allows the generating of break points).
    /// Score is not normalized.
    /// </summary>
    /// <param name="i">Start index of segment [i,j]</param>
    /// <param name="j">End index of segment [i,j]</param>
    /// <returns>Variance-based score of segment [i,j]</returns>
    float computeVScore(int i, int j);

    /// <summary>
    /// Compute the r² of segment [i,j] (after linear regression model computation).
    /// </summary>
    /// <param name="i">Start index of segment [i,j]</param>
    /// <param name="j">End index of segment [i,j]</param>
    /// <returns>r² of segment [i,j]</returns>
    float computeRSquare(int i, int j);

    /// <summary>
    /// Compute the variance of segment [i,j] (after linear regression model computation).
    /// </summary>
    /// <param name="i">Start index of segment [i,j]</param>
    /// <param name="j">End index of segment [i,j]</param>
    /// <returns>Variance of segment [i,j]</returns>
    float computeVar(int i, int j);

    /// <summary>
    /// Computes the arithmetic mean of the segment [i,j], using the given x coordinates.
    /// </summary>
    /// <param name="x">X coordinates of a given profile</param>
    /// <param name="i">Start index of segment [i,j]</param>
    /// <param name="j">End index of segment [i,j]</param>
    /// <returns>The arithmetic mean of segment [i,j]</returns>
    float computeArithmeticMean(float* x, int i, int j);

    //// SEGMENTATION FUNCTIONS

    /// <summary>
    /// S. Viseur's method. Inspired by Machne & Stadler's method (see dpseg R package)
    /// with key differences. Returns a given profile's break points, allowing the algorithm
    /// to create segments.
    /// </summary>
    /// <returns></returns>
    std::vector<int> profile_segmentation();

    /// <summary>
    /// The two following functions work as a pair. getMaximumIndexes() returns the indexes of
    /// a given profile's break points, allowing the backtracing part of computeSegmentation()
    /// to generate the segment outputs.
    /// 
    /// computeSegmentation() can work as a standalone if the backtracing part is included in the
    /// segmentation method (as is the case with S. Viseur's profile_segmentation()). If using
    /// another segmentation method, call it there.
    /// </summary>
    //! returns a list of indexes 
    int* getMaximumIndexes();
    //! returns a list of segments
    std::vector<SegmentLinearRegression*> computeSegmentation();

private:
    //attributes
    float*  m_x, * m_y; //coordinates
    float m_p; //p value
    int m_n, m_j, m_maxL, m_minL = 3; //size, jumps, max length, min length
    const char* m_type; //scoring function type

    //! list of max score indexes
    int* i_max;
    //! used to store computed scores
    float* scores, * maxScores;

    //! output segments
    std::vector<SegmentLinearRegression*> m_segments;
};

#endif // DPPIECEWISELINEARREGRESSION_H
