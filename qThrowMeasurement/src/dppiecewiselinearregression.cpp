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
#include <QColor>
#include <GeometricalAnalysisTools.h>
#include <Jacobi.h>

#include "dppiecewiselinearregression.h"
#include "segmentlinearregression.h"

using namespace CCCoreLib;

dPPiecewiseLinearRegression::dPPiecewiseLinearRegression(float* x, float* y, const int n, 
    float p, int j, const char* type) : m_x(x), m_y(y), m_n(n), m_p(p), m_j(j), m_type(type)
{

}

dPPiecewiseLinearRegression::~dPPiecewiseLinearRegression()
{
    m_segments.clear();
}

float dPPiecewiseLinearRegression::computeModel(float x, int i, int j)
{
    return computeSlope(i,j)*x + computeIntercept(i,j);
}

////SCORING FUNCTION (r²)
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

        //qDebug() << "y" << m_y[k];
        //qDebug() << "x" << m_x[k];
    }

    return static_cast <float>(1.) - static_cast<float>(var1 / var2);
}

////SCORING FUNCTION (variance)
float dPPiecewiseLinearRegression::computeVScore(int i, int j)
{
    float residuals = 0.;
    float slope = computeFirstSlope(i, j);
    float intercept = computeFirstIntercept(i, j);

    /*for (int k = i; k < j; k++) residuals += pow(m_y[k] - intercept 
        - slope * m_x[k], 2);

    return -(static_cast<float>(1.) / static_cast<float>(m_n - 1.)) * residuals;*/

    for (int k = i; k < j+1; k++)
    {
        float delta_k = m_y[k] - intercept - slope * m_x[k];
        residuals += delta_k * delta_k;
    }

    return  (-1. * residuals / static_cast<float>(m_n - 1.));
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

int dPPiecewiseLinearRegression::getMaximumIndex(int j)
{
    int i = 0, i_max = 0;
    float score = 0., maxScore = /*3.40282e+038*/ -m_p;

    for (int i = 0; i < j+1; i++)
    {
        if (m_type == "rsquare") score = computeRScore(i, j) - m_p;
        else score = computeVScore(i, j) - m_p; //default

        if (score < maxScore)
        {
            maxScore = score;
            i_max = i;
        }
    }

    return i_max;
}

std::vector<SegmentLinearRegression*> dPPiecewiseLinearRegression::computeSegmentation()
{
    QVector<int> maxI;
    int end = m_n - 1; // an array of size n goes from 0 to n-1
    maxI.push_front(end);

    //backtracing
    for ( ; end > 1; end)
    {
        end = getMaximumIndex(end) - m_j;
        maxI.push_front(end);
    }

    if (m_j != 0) maxI[0] = 0;

    qDebug() << "maxI size" << maxI.size();

    for (int k =0; k < maxI.size(); k++) qDebug() << "maxI" << maxI[k];
    for (int k = 0; k < maxI.size()-1; k++)
    {
        SegmentLinearRegression* segment = new SegmentLinearRegression(maxI[k], maxI[k+1], m_x, m_y);
        segment->setSlope(computeSlope(maxI[k], maxI[k + 1]));
        segment->setIntercept(computeIntercept(maxI[k], maxI[k + 1]));
        segment->setRSquare(computeRSquare(maxI[k], maxI[k + 1]));
        segment->setVar(computeVar(maxI[k], maxI[k + 1]));
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

    /*for (int k = i; k < j; k++)
    {
        sumX = sumX + m_x[k];
        sumX2 = sumX2 + m_x[k] * m_x[k];
        sumY = sumY + m_y[k];
        sumXY = sumXY + m_x[k] * m_y[k];
    }*/

    float meanX = computeArithmeticMean(m_x, i, j);
    float meanY = computeArithmeticMean(m_y, i, j);
    //qDebug() << "meanX" << meanX;

    for (int k = i; k < j+1; k++)
    {
        sumXY += (m_x[k] - meanX)*(m_y[k] - meanY);
        sumX2 += (m_x[k] - meanX)*(m_x[k] - meanX);
    }

    //compute a
    //return static_cast<float>(m_n * sumXY - sumX * sumY) / static_cast<float>(m_n * sumX2 - sumX * sumX);
    //to edit bc it suspiciously seems like it uses the forbidden variance computation

    return static_cast<float> (sumXY) / static_cast<float> (sumX2);
}

float dPPiecewiseLinearRegression::computeFirstSlope(int i, int j)
{
    float sumX = 0., sumY = 0., sumX2 = 0., sumXY = 0.;

    /*for (int k = i; k < j; k++)
    {
        sumX = sumX + m_x[k];
        sumX2 = sumX2 + m_x[k] * m_x[k];
        sumY = sumY + m_y[k];
        sumXY = sumXY + m_x[k] * m_y[k];
    }*/

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

    //compute a
    //return static_cast<float>(m_n * sumXY - sumX * sumY) / static_cast<float>(m_n * sumX2 - sumX * sumX);
    //to edit bc it suspiciously seems like it uses the forbidden variance computation

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

/*
float dPPiecewiseLinearRegression::computeVariance(int i, int j)
{
    float var = 0.;
    float meanX = computeArithmeticMean(m_x, i, j);
    qDebug() << "mean" << meanX;
    for (int k = i; k < j; k++) 
    { 
        var += (m_x[k] - meanX)*(m_x[k] - meanX);
    }

    return static_cast<float> (var) / static_cast<float>(m_n - 1.);
}
*/
