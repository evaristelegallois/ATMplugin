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

#include <QtGui>

//qCC_db
#include <ccGenericPointCloud.h>
#include <ccHObjectCaster.h>
#include <ccPointCloud.h>
#include <ccPolyline.h>

#include "qThrowMeasurement.h"
#include "ccMainAppInterface.h"

//Qt
#include <QMainWindow>
#include <QVector2D>
#include <QVector>
#include <QColor>
#include <GeometricalAnalysisTools.h>
#include <Jacobi.h>

#include "dppiecewiselinearregression.h"
#include "segmentlinearregression.h"

using namespace CCCoreLib;

dPPiecewiseLinearRegression::dPPiecewiseLinearRegression(float* x, float* y, const int n, 
    float p, int j, const char* type) : m_x(x), m_y(y), m_n(n), m_p(p), m_j(j), m_type(type)
{
        i_max = new int[m_n];
        maxScores = new float[m_n];
        scores = new float[m_n];
}

/*dPPiecewiseLinearRegression::~dPPiecewiseLinearRegression()
{
    m_segments.clear();
    //m_segments.shrink_to_fit();

    delete[] i_max;
    delete[] scores;
    delete[] maxScores;
}
*/

float dPPiecewiseLinearRegression::computeModel(float x, int i, int j)
{
    //returns a*x + b
    return computeSlope(i,j)*x + computeIntercept(i,j);
}

float dPPiecewiseLinearRegression::computeRSquare(int i, int j)
{
    float var1 = 0., var2 = 0.;
    float meanY = computeArithmeticMean(m_y, i, j);
    float slope = computeSlope(i, j);
    float intercept = computeIntercept(i, j);

    for (int k = i; k < j + 1; k++)
    {
        float delta_k = m_y[k] - intercept - slope * m_x[k];
        var1 += delta_k * delta_k;
        var2 += (m_y[k] - meanY) * (m_y[k] - meanY);
    }

    return static_cast <float>(1.) - static_cast<float>(var1 / var2);
}

////SCORING FUNCTION (r² based)
float dPPiecewiseLinearRegression::computeRScore(int i, int j)
{
    float var1 = 0., var2 = 0.;
    float meanY = computeArithmeticMean(m_y, i, j);

    for (int k = i; k < j+1; k++)
    {
        var1 += (computeModel(m_x[k], i, j) - meanY)*(computeModel(m_x[k], i, j) - meanY);
        var2 += (m_y[k] - meanY)* (m_y[k] - meanY);
    }
    
    return static_cast<float>(var1 / var2) - static_cast <float>(1.);
}

////SCORING FUNCTION (variance of residuals based)
float dPPiecewiseLinearRegression::computeVScore(int i, int j)
{
    int start = 0, end = 0;
    if (i < j) start = i, end = j;
    else start = j, end = i;
    
    float residuals = 0.;
    float slope = computeSlope(start, end);
    float intercept = computeIntercept(start, end);

    for (int k = start; k < end+1; k++)
    {
        float delta_k = m_y[k] - intercept - slope * m_x[k];
        residuals += delta_k * delta_k;
    }

    //not normalized
    return  (-1. * residuals / static_cast<float>(end - start + 1.));
    
    //old version below, with normalized variance

    /*
    float var1 = 0., var2 = 0.;
    float meanY = computeArithmeticMean(m_y, start, end);
    float slope = computeSlope(start, end);
    float intercept = computeIntercept(start, end);

    for (int k = start; k < end + 1; k++)
    {
        var1 += (slope*m_x[k] + intercept - m_y[k]) * (slope * m_x[k] + intercept - m_y[k]);
        var2 += (m_y[k] - meanY) * (m_y[k] - meanY);
    }

    return static_cast<float>(-1. * (var1 / var2));
    */
}

float dPPiecewiseLinearRegression::computeVar(int i, int j)
{
    float residuals = 0.;
    float slope = computeSlope(i, j);
    float intercept = computeIntercept(i, j);

    /*for (int k = i; k < j; k++) residuals += pow(m_y[k] - intercept
        - slope * m_x[k], 2);

    return -(static_cast<float>(1.) / static_cast<float>(m_n - 1.)) * residuals;*/

    for (int k = i; k < j+1; k++)
    {
        float delta_k = m_y[k] - intercept - slope * m_x[k];
        residuals += delta_k * delta_k;
    }

    return  (residuals / static_cast<float>(j+1.-i - 1.));
}

//unused function; doesn't seem to work?
int* dPPiecewiseLinearRegression::getMaximumIndexes()
{
    m_maxL = m_n; //get parameter if user input
    int idx = 0;
    float score = 0., maxScore = -3.40282e+038, prevScore = -m_p;

    //initialization
    maxScores[0] = static_cast<float> (-m_p);
    maxScores[1]= static_cast<float> (-m_p);

    int* imax = new int[m_n];

    for (int j = 1; j < m_n; j++)
    {
        //int i_range = m_maxL - m_minL + 1; // i in [j-maxL, j-minL +1]
        idx = std::max(m_maxL-(j+1),0);
        //computation of all scores for a given j
        for (int i = idx; i < m_maxL - m_minL + 1; i++)
        {
            // current index i = (j+1)-maxl+1+idx - 1-based index
            int k = (j + 1) - m_maxL + i; // 0-based array index i
            int ij = k - 1; // sum of squares subtraction index
            //int n = j - ij; // length of segment i -> j

            //idx = i - (j - m_maxL); //are written over multiple times??
            if (k - m_j == -1) prevScore = -m_p; //S0 aka P?
            else prevScore = maxScores[k - m_j];
            if (ij < 0) scores[i] = prevScore + computeVScore(1, j) - m_p;
            else scores[i] = prevScore + computeVScore(j, ij) - m_p;
        }
        
        //computation of max score for a given j
        int index = 0;
        for (int i = idx; i < m_maxL - m_minL + 1; i++)
        {
            if (scores[i] > maxScore)
            {
                maxScore = scores[i]; //S[j] = max(si);
                index = i; //idx = which_max(si);
            }
        }

        if (j > 1) maxScores[j] = maxScore;
        imax[m_maxL-j+1] = index /*+ (j - m_maxL) + 1*/; // 0-based idx;
    }

    return imax;
}

//currently used function; credit goes to S. Viseur
std::vector<int> dPPiecewiseLinearRegression::profile_segmentation()
{
    int n_atoms = m_n;
    std::vector<double> val_x;
    std::vector<double> val_y;

    //. Build 2D data
    for (int i = 0; i < n_atoms; i++) {
        val_x.push_back(m_x[i]);
        val_y.push_back(m_y[i]);
    }

    std::vector<std::vector<double>> S;
    std::vector<double> S0;
    S0.push_back(-1 * m_p);
    std::vector<double> max_Sj;
    max_Sj.push_back(-1 * m_p);

    //. Compute scores
    for (int j = 1; j < n_atoms; j++) {
        std::vector<double> Sj;

        //. Loop between 0 and j, in order to find max_S
        for (int i1 = 0; i1 < j; i1++) {
            auto f2_x = val_x.cbegin() + i1;
            auto l2_x = val_x.cbegin() + j + 1;
            auto f2_y = val_y.cbegin() + i1;
            auto l2_y = val_y.cbegin() + j + 1;

            std::vector<double> vec2_x(f2_x, l2_x);
            std::vector<double> vec2_y(f2_y, l2_y);
            //SimpleRegression sreg2;
            double Si1_2 = 0.;

            //. variance computation
            Si1_2 = computeVScore(i1, j);

            double Si1 = Si1_2;
            Sj.push_back(Si1);
        }
        S.push_back(Sj);
        double cur_max_Sj = *std::max_element(Sj.begin(), Sj.end() - 1);
        max_Sj.push_back(cur_max_Sj);
    }

    int nb_Smax = max_Sj.size();
    std::vector<double> shift;
    auto i1 = max_Sj.cbegin();
    for (int i = 0; i < nb_Smax; i++) {
        auto i2 = max_Sj.cbegin() + i + 1;
        std::vector<double> max_Sj_i(i1, i2);
        double min_Smax = *std::min_element(max_Sj_i.begin(), max_Sj_i.end());
        shift.push_back(min_Smax);
    }

    std::vector<double> scores;
    scores.push_back(shift[0] + max_Sj[0]);
    for (int i = 1; i < nb_Smax; i++) {
        double score_i = shift[i - 1] + max_Sj[i];
        scores.push_back(score_i);
    }

    //. Print max_Sj
    int nb_S = S.size();
    int max_elem = -1;
    for (int j = 0; j < nb_S; j++) {
        std::vector<double> Sj = S[j];
        int nb_elem = Sj.size();
        if (nb_elem > max_elem) {
            max_elem = nb_elem;
        }
    }

    //. Backtracing
    int S_idx = S.size() - 1;
    int max_Sj_idx = max_Sj.size() - 1;
    int idx = n_atoms - 1;

    std::vector<int> break_points;
    break_points.push_back(n_atoms - 1);

    while (idx > 0) {
        int imin = idx;
        double min_S = scores[idx];
        for (int i1 = idx; i1 >= 0; i1--) {
            double val = scores[i1];
            if (val < min_S) {
                min_S = val;
                imin = i1;
            }
        }
        break_points.insert(break_points.begin(), imin);
        idx = imin - m_minL; 

        if (idx <= 0) {
            idx = 0;
            break_points.insert(break_points.begin(), 0);
        }
    }

    return break_points;
}

std::vector<SegmentLinearRegression*> dPPiecewiseLinearRegression::computeSegmentation()
{
    QVector<int> maxI;
    int end = m_n - 1; // an array of size n goes from 0 to n-1

    // old implementation; un-comment if using getMaximumIndexes() function
    
    //maxI.push_front(end);

    //i_max = getMaximumIndexes();
    /*
    //backtracing
    for ( ; end > 1; end)
    {
        end = i_max[end] - m_j;
        maxI.push_front(end);
    }
    */

    // IF USING ANOTHER SEGMENTATION FUNCTION: it goes there
    std::vector<int> imax = profile_segmentation();
    for (int i = 0; i < imax.size(); i++) maxI.push_back(imax[i]);

    // old implementation; see above 
    
    //if (m_j != 0) maxI[0] = 0;
    //else maxI[0] = 0;

    maxI[0] = 0;
    int nbVert = 0;
    //segment creation; sets all the relevant attributes as well
    for (int k = 0; k < maxI.size()-1; k++)
    {
        SegmentLinearRegression* segment = new SegmentLinearRegression(maxI[k], maxI[k+1], m_x, m_y);
        segment->setSlope(computeSlope(maxI[k], maxI[k + 1]));
        segment->setIntercept(computeIntercept(maxI[k], maxI[k + 1]));
        segment->setRSquare(computeRSquare(maxI[k], maxI[k + 1]));
        segment->setVar(computeVar(maxI[k], maxI[k + 1]));
        segment->setAssociatedP(m_p);
        segment->setColor(QVector3D(rand() % 359 + 0, 1, 1));

        m_segments.push_back(segment);
    }

    return m_segments;
}

float dPPiecewiseLinearRegression::computeArithmeticMean(float* x, int i, int j)
{
    float sum = 0.;
    for (int k = i; k < j+1; k++) sum = sum + x[k];

    return static_cast<float> (sum) / static_cast<float> (j-i+1.);
}

float dPPiecewiseLinearRegression::computeSlope(int i, int j)
{
    float sumX = 0., sumY = 0., sumX2 = 0., sumXY = 0.;
    float meanX = computeArithmeticMean(m_x, i, j);
    float meanY = computeArithmeticMean(m_y, i, j);

    for (int k = i; k < j+1; k++)
    {
        sumXY += (m_x[k] - meanX)*(m_y[k] - meanY);
        sumX2 += (m_x[k] - meanX)*(m_x[k] - meanX);
    }

    //compute a
    return static_cast<float> (sumXY) / static_cast<float> (sumX2);
}

float dPPiecewiseLinearRegression::computeIntercept(int i, int j)
{
    float sumX = 0.;
    float sumY = 0.;

    for (int k = i; k < j+1; k++)
    {
        sumX = sumX + m_x[k];
        sumY = sumY + m_y[k];
    }

    //compute b
    return static_cast<float>(sumY - computeSlope(i,j) * sumX) / static_cast<float>(j+1.-i);
}