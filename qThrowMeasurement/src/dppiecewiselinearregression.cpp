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

#include <QFile>
#include <QTextStream>

#include "dppiecewiselinearregression.h"
#include "segmentlinearregression.h"

using namespace CCCoreLib;

dPPiecewiseLinearRegression::dPPiecewiseLinearRegression(float* x, float* y, const int n, 
    float p, int j, const char* type) : m_x(x), m_y(y), m_n(n), m_p(p), m_j(j), m_type(type)
{
    //std::vector<SegmentLinearRegression*> segments = new std::vector<SegmentLinearRegression>[100]; //max nb of segments
    //m_segments = segments;
}

dPPiecewiseLinearRegression::~dPPiecewiseLinearRegression()
{
    /*m_segments = nullptr;
    delete m_segments;*/
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
    float var1 = 0., var2 = 0.;
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
    float residuals = 0.;

    for (int k = i; k < j; k++) residuals += pow(m_y[k] - computeIntercept(i, j) 
        - computeSlope(i, j) * m_x[k], 2);

    return -(static_cast<float>(1.) / static_cast<float>(m_n - 1.)) * residuals;
}

int dPPiecewiseLinearRegression::getMaximumIndex(int j)
{
    int i = 0, i_max = 0;
    float score = 0., maxScore = 3.40282e+038;

    for (int i = 0; i < j; i++)
    {
        if (m_type == "rsquare") score = computeRScore(i, j) - m_p;
        else score = computeVScore(i, j) - m_p; //default
        //qDebug() << "score" << score;

        if (score < maxScore)
        {
            maxScore = score;
            //qDebug() << "max score" << maxScore;
            i_max = i;
            //qDebug() << "imax" << i_max;
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
    while (end > 1) //for --
    {
        //qDebug() << "end" << end;
        //qDebug() << "get imax" << getMaximumIndex(end);
        end = getMaximumIndex(end) - m_j;
        maxI.push_front(end);
    }

    for (int i = 0; i < maxI.size(); i++)
    {
        //qDebug() << "maxI" << maxI[i];
    }

    qDebug() << "maxI size" << maxI.size();
    for (int k = 0; k < maxI.size()-1; k++)
    {
        //qDebug() << "segment loop" << k;
        /*SegmentLinearRegression* segment = new SegmentLinearRegression(maxI[k], maxI[k+1], m_x, m_y);
        segment->setSlope(computeSlope(maxI[k], maxI[k + 1]));
        segment->setIntercept(computeIntercept(maxI[k], maxI[k + 1]));
        segment->setRSquare(computeRScore(maxI[k], maxI[k + 1]));
        segment->setVar(computeVariance(maxI[k], maxI[k + 1]));
        segment->setColor(QVector3D(rand() % 359 + 0, 1, 1));*/

        //m_segments.push_back(segment);
    }

    return m_segments;
}

float dPPiecewiseLinearRegression::computeArithmeticMean(float* x)
{
    float sum = 0.;
    for (int i = 0; i < m_n; i++)  sum += x[i];

    return static_cast<float> (sum) / static_cast<float> (m_n);
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

    for (int k = i; k < j; k++)
    {
        sumXY += (m_x[k] - computeArithmeticMean(m_x))*(m_y[k] - computeArithmeticMean(m_y));
        sumX2 += pow((m_x[k] - computeArithmeticMean(m_x)), 2);
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

    for (int k = i; k < j; k++)
    {
        sumX = sumX + m_x[k];
        sumY = sumY + m_y[k];
    }

    //compute b
    return static_cast<float>(sumY - computeSlope(i,j) * sumX) / static_cast<float>(m_n);
}

float dPPiecewiseLinearRegression::computeVariance(int i, int j)
{
    float var = 0.;
    for (int k = i; k < j; k++) var += pow(m_x[k] - computeArithmeticMean(m_x), 2);

    return var;
}
