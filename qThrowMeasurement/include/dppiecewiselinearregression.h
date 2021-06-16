#ifndef DPPIECEWISELINEARREGRESSION_H
#define DPPIECEWISELINEARREGRESSION_H

#include "segmentlinearregression.h"

class dPPiecewiseLinearRegression
{
public:
    dPPiecewiseLinearRegression(float* x, float* y, int n, float p, 
        int j = 0, const char* type = "var");

    float computeModel(float x, int i, int j);
    float computeResidual(int i);
    float computeSlope(int i, int j);
    float computeIntercept(int i, int j);
    float computeRScore(int i, int j);
    float computeVScore(int i, int j);
    float computeVariance(int i, int j);

    int getMaximumIndex(int j);
    std::vector<SegmentLinearRegression>* computeSegmentation();

    float computeArithmeticMean(float* x);

private:
    //attributes
    float*  m_x, * m_y;
    float m_p;
    int m_n;
    int m_j;
    const char* m_type;

    float m_a, m_b, m_r; //m_a, m_b useless

    std::vector<SegmentLinearRegression> * m_segments;
};

#endif // DPPIECEWISELINEARREGRESSION_H
