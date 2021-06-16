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
    std::vector<SegmentLinearRegression> segments[100]; //size allocation; TO EDIT
    m_segments = segments;

    //ASSERT X & Y SIZE
}

dPPiecewiseLinearRegression::~dPPiecewiseLinearRegression()
{
    m_segments = nullptr;
}

float dPPiecewiseLinearRegression::computeModel(float x, int i, int j)
{
    return computeSlope(i,j)*x + computeIntercept(i,j);
}
/*
float dPPiecewiseLinearRegression::computeResidual(int i) 
{
    return m_y[i] - m_b - m_a*m_x[i];
}
*/

////SCORING FUNCTION (r²)
float dPPiecewiseLinearRegression::computeRScore(int i, int j)
{
    float var1, var2 = 0;
    for (int k = i; k < j; k++)
    {
        var1 += pow((computeModel(m_x[k], i, j) - computeArithmeticMean(m_y)), 2);
        var2 += pow((m_y[k] - computeArithmeticMean(m_y)), 2);
    }
    
    return (var1 / var2) - 1;
}

////SCORING FUNCTION (variance)
float dPPiecewiseLinearRegression::computeVScore(int i, int j)
{
    float residuals = 0;
    for (int k = i; k < j; k++) residuals += m_y[k] - computeIntercept(i,j) 
        - computeSlope(i,j) * m_x[k];

    return -(1 / (m_n - 1)) * residuals;
}

int dPPiecewiseLinearRegression::getMaximumIndex(int j)
{
    int i_max = 0;
    for (int i = 0; i < m_n; i++)
    {
        float score, maxScore = 0;
        while (i < j)
        {
            if (m_type == "rsquare") score = computeRScore(i, j) - m_p;
            else score = computeVScore(i, j) - m_p; //default

            if (score > maxScore)
            {
                maxScore = score;
                i_max = i;
            }
        }
    }

    return i_max;
}

std::vector<SegmentLinearRegression>* dPPiecewiseLinearRegression::computeSegmentation()
{
    QVector<int> maxI;
    int end = m_n;

    //backtracing
    while (end > 1)
    {
        end = getMaximumIndex(end) - m_j;
        maxI.push_front(getMaximumIndex(end));
    }

    for (int k = 0; k < maxI.size()-1; k++)
    {
        SegmentLinearRegression segment = SegmentLinearRegression(maxI[k], maxI[k+1], m_x, m_y);
        segment.setSlope(computeSlope(maxI[k], maxI[k + 1]));
        segment.setIntercept(computeIntercept(maxI[k], maxI[k + 1]));
        segment.setRSquare(computeRScore(maxI[k], maxI[k + 1]));
        segment.setVar(computeVariance(maxI[k], maxI[k + 1]));
        segment.setColor(QVector3D(rand() % 359 + 0, 1, 1));

        m_segments->push_back(segment);
    }

    return m_segments;
}

float dPPiecewiseLinearRegression::computeArithmeticMean(float* x)
{
    float sum = 0;
    for (int i = 0; i < m_n; i++)  sum += x[i];

    return sum / m_n;
}

float dPPiecewiseLinearRegression::computeSlope(int i, int j)
{
    float sumX, sumY, sumX2, sumXY = 0;

    for (int k = i; k < j; k++)
    {
        sumX = sumX + m_x[k];
        sumX2 = sumX2 + m_x[k] * m_x[k];
        sumY = sumY + m_y[k];
        sumXY = sumXY + m_x[k] * m_y[k];
    }

    //compute a
    return (m_n * sumXY - sumX * sumY) / (m_n * sumX2 - sumX * sumX); 
    //to edit bc it suspiciously seems like it uses the forbidden variance computation
}

float dPPiecewiseLinearRegression::computeIntercept(int i, int j)
{
    float sumX, sumY = 0;

    for (int k = i; k < j; k++)
    {
        sumX = sumX + m_x[k];
        sumY = sumY + m_y[k];
    }

    //compute b
    return (sumY - m_b * sumX) / m_n;
}

float dPPiecewiseLinearRegression::computeVariance(int i, int j)
{
    float var = 0;
    for (int k = i; k < j; k++) var += pow(m_x[k] - computeArithmeticMean(m_x), 2);

    return var;
}
