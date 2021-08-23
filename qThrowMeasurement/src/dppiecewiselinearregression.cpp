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

dPPiecewiseLinearRegression::~dPPiecewiseLinearRegression()
{
    m_segments.clear();
}

float dPPiecewiseLinearRegression::computeModel(float x, int i, int j)
{
    return computeSlope(i,j)*x + computeIntercept(i,j);
}

////SCORING FUNCTION (r² based)
float dPPiecewiseLinearRegression::computeRScore(int i, int j)
{
    float var1 = 0., var2 = 0.;
    float meanY = computeArithmeticMean(m_y, i, j);

    for (int k = i; k < j+1; k++)
    {
        var1 += pow((computeModel(m_x[k], i, j) - meanY), 2);
        var2 += (m_y[k] - meanY)* (m_y[k] - meanY);
    }
    
    return static_cast<float>(var1 / var2) - static_cast <float>(1.);
}

float dPPiecewiseLinearRegression::computeRSquare(int i, int j)
{
    float var1 = 0., var2 = 0.;
    float meanY = computeArithmeticMean(m_y, i, j);
    float slope = computeSlope(i, j);
    float intercept = computeIntercept(i, j);

    for (int k = i; k < j+1; k++)
    {
        float delta_k = m_y[k] - intercept - slope * m_x[k];
        var1 += delta_k * delta_k;
        var2 += (m_y[k] - meanY)* (m_y[k] - meanY);
    }

    return static_cast <float>(1.) - static_cast<float>(var1 / var2);
}

////SCORING FUNCTION (variance of residuals based)
float dPPiecewiseLinearRegression::computeVScore(int i, int j)
{
    /*
    float residuals = 0.;
    float slope = computeSlope(i, j);
    float intercept = computeIntercept(i, j);

    for (int k = i; k < j+1; k++)
    {
        float delta_k = m_y[k] - intercept - slope * m_x[k];
        residuals += delta_k * delta_k;
    }

    return  (-1. * residuals / static_cast<float>(m_n - 1.));
    */
    int start = 0, end = 0;
    if (i < j) start = i, end = j;
    else start = j, end = i;
    //qDebug() << "start, end" << start << end;

    float var1 = 0., var2 = 0.;
    float meanY = computeArithmeticMean(m_y, start, end);
    float slope = computeSlope(start, end);
    float intercept = computeIntercept(start, end);

    for (int k = start; k < end + 1; k++)
    {
        var1 += (slope*m_x[k] + intercept - m_y[k]) * (slope * m_x[k] + intercept - m_y[k]);
        var2 += (m_y[k] - meanY) * (m_y[k] - meanY);
    }

    //qDebug() << "var1, var2" << var1 << var2;

    return static_cast<float>(-1.*(var1 / var2));
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

int* dPPiecewiseLinearRegression::getMaximumIndexes()
{
    m_maxL = m_n; //get parameter if user input
    int idx = 0;
    float score = 0., maxScore = -3.40282e+038, prevScore = -m_p, currentScore = 0.;
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
            //qDebug() << "k" << k;

            //idx = i - (j - m_maxL); //are written over multiple times??
            if (k - m_j == -1) prevScore = -m_p; //S0 aka P?
            else prevScore = maxScores[k - m_j];
            //qDebug() << "prevscore" << prevScore;
            if (ij < 0) scores[i] = prevScore + computeVScore(0, j) - m_p;
            else scores[i] = prevScore + computeVScore(j, ij) - m_p;
        }
        
        //computation of max score for a given j
        int index = 0;
        for (int i = idx; i < m_maxL - m_minL + 1; i++)
        {
            //qDebug() << "score, maxScore" << scores[i] << maxScore;
            if (scores[i] > maxScore)
            {
                maxScore = scores[i];
                index = i;
            }
        }

        //S[j] = max(si);
        //idx = which_max(si);

        if (j > 1) maxScores[j] = maxScore;
        qDebug() << "max score @ j" << maxScores[j];
        imax[m_maxL-j+1] = index /*+ (j - m_maxL) + 1*/; // 0-based
        //qDebug() << "idx end" << idx;
    }

    //for (int j = 0; j < m_n; j++) qDebug() << "imax @ j" << j << imax[j];

    return imax;
}

std::vector<SegmentLinearRegression*> dPPiecewiseLinearRegression::computeSegmentation()
{
    QVector<int> maxI;
    int end = m_n - 1; // an array of size n goes from 0 to n-1
    maxI.push_front(end);
    qDebug() << "end" << end;

    i_max = getMaximumIndexes();

    //backtracing
    for ( ; end > 1; end)
    {
        qDebug() << "imax @ end" << i_max[end];
        end = i_max[end] - m_j;
        maxI.push_front(end);
        qDebug() << "end" << end;
    }

    if (m_j != 0) maxI[0] = 0;
    else maxI[0] = 0;

    qDebug() << "maxI size" << maxI.size();

    //for (int k =0; k < maxI.size(); k++) qDebug() << "maxI" << maxI[k];
    int nbVert = 0;
    for (int k = 0; k < maxI.size()-1; k++)
    {
        SegmentLinearRegression* segment = new SegmentLinearRegression(maxI[k], maxI[k+1], m_x, m_y);
        segment->setSlope(computeSlope(maxI[k], maxI[k + 1]));
        segment->setIntercept(computeIntercept(maxI[k], maxI[k + 1]));
        segment->setRSquare(computeRSquare(maxI[k], maxI[k + 1]));
        segment->setVar(computeVar(maxI[k], maxI[k + 1]));
        segment->setAltVar(computeAltVar(maxI[k], maxI[k + 1]));
        segment->setAssociatedP(m_p);
        segment->setColor(QVector3D(rand() % 359 + 0, 1, 1));
        //nbVert += segment->getSize();

        m_segments.push_back(segment);
    }

    //qDebug() << "nb vertices" << nbVert;

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

    return static_cast<float> (sumXY) / static_cast<float> (sumX2);
}

float dPPiecewiseLinearRegression::computeFirstSlope(int i, int j)
{
    float sumX = 0., sumY = 0., sumX2 = 0., sumXY = 0.;
    for (int k = 0; k < m_n; k++)
    {
        sumX = sumX + m_x[k];
        sumY = sumY + m_y[k];
    }

    float meanX = static_cast<float> (sumX) / static_cast<float> (m_n);
    float meanY = static_cast<float> (sumY) / static_cast<float> (m_n);

    for (int k = i; k < j+1; k++)
    {
        sumXY += (m_x[k] - meanX) * (m_y[k] - meanY);
        sumX2 += (m_x[k] - meanX) * (m_x[k] - meanX);
    }
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

float dPPiecewiseLinearRegression::computeFirstIntercept(int i, int j)
{
    float sumX = 0.;
    float sumY = 0.;

    for (int k = i; k < j+1; k++)
    {
        sumX = sumX + m_x[k];
        sumY = sumY + m_y[k];
    }

    //compute b
    return static_cast<float>(sumY - computeFirstSlope(i, j) * sumX) / static_cast<float>(m_n);
}

QVector2D dPPiecewiseLinearRegression::computeAltVar(int i, int j)
{
    float sumX = 0., sumY = 0.;
    float meanX = computeArithmeticMean(m_x, i, j);
    float meanY = computeArithmeticMean(m_y, i, j);

    for (int k = i; k < j + 1; k++)
    {
        sumX += (m_x[k] - meanX) * (m_x[k] - meanX);
        sumY += (m_y[k] - meanY) * (m_y[k] - meanY);
    }

    return QVector2D (static_cast<float> (sumX), static_cast<float> (sumY));
}
