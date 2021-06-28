#ifndef DPPIECEWISELINEARREGRESSION_H
#define DPPIECEWISELINEARREGRESSION_H

#include "segmentlinearregression.h"

class dPPiecewiseLinearRegression
{
public:
    dPPiecewiseLinearRegression(float* x, float* y, int n, float p, 
        int j = 0, const char* type = "var");
    ~dPPiecewiseLinearRegression();

    float computeModel(float x, int i, int j);
    float computeResidual(int i);
    float computeFirstSlope(int i, int j);
    float computeSlope(int i, int j);
    float computeFirstIntercept(int i, int j);
    float computeIntercept(int i, int j);
    float computeRScore(int i, int j);
    float computeRSquare(int i, int j);
    float computeVScore(int i, int j);
    float computeVar(int i, int j);

    int getMaximumIndex(int j);
    std::vector<SegmentLinearRegression*> computeSegmentation();

    float computeArithmeticMean(float* x, int i, int j);

private:
    //attributes
    float*  m_x, * m_y;
    float m_p;
    int m_n, m_j, m_maxL, m_minL = 3;
    const char* m_type;

    std::vector<SegmentLinearRegression*> m_segments;
};

#endif // DPPIECEWISELINEARREGRESSION_H
