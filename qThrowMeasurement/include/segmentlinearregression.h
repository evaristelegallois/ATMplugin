#ifndef SEGMENTLINEARREGRESSION_H
#define SEGMENTLINEARREGRESSION_H

#include <QVector2D>
#include <QVector3D>
#include <ccPointCloud.h>
#include <ccPolyline.h>

class SegmentLinearRegression
{
public:
    SegmentLinearRegression(int startInd, int endInd, float* x, float* y);
    ~SegmentLinearRegression();

    float getVar();
    QVector2D getAltVar();
    float getRSquare();
    float getSlope();
    float getIntercept();
    QVector2D getStart();
    QVector2D getEnd();
    int getStartIndex();
    int getEndIndex();
    int getSize();
    float getAssociatedP();
    QVector2D* getPoint(int index);

    QVector3D getColor();

    void setVar(float var);
    void setAltVar(QVector2D altVar);
    void setRSquare(float rsquare);
    void setSlope(float slope);
    void setIntercept(float intercept);
    void setStart(QVector2D start);
    void setEnd(QVector2D end);
    void setStartIndex(int startInd);
    void setEndIndex(int endInd);
    void setAssociatedP(float p);

    void setColor(QVector3D color);

private:
    float m_a, m_b, m_r, m_rsquare, m_var, m_p;
    QVector2D m_start, m_end, m_altVar;
    int m_startInd, m_endInd, m_size;
    QVector3D m_color;

    QVector<QVector2D*> m_points;
};

#endif // SEGMENTLINEARREGRESSION_H
