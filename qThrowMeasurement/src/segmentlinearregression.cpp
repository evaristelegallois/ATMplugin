#include "segmentlinearregression.h"
#include <QDebug>

SegmentLinearRegression::SegmentLinearRegression(int startInd, int endInd, float* x, float* y) :
	m_startInd(startInd), m_endInd(endInd)
{
	m_size = endInd - startInd + 1;

	for (int i = m_startInd; i < m_endInd + 1; i++) m_points.push_back(new QVector2D(x[i], y[i]));
	m_a = 0, m_b = 0, m_r = 0, m_rsquare = 0, m_var = 0;
}

SegmentLinearRegression::~SegmentLinearRegression()
{
	m_points.clear();
}

float SegmentLinearRegression::getVar()
{
	return m_var;
}

QVector2D SegmentLinearRegression::getAltVar()
{
	return m_altVar;
}

float SegmentLinearRegression::getRSquare()
{
	return m_rsquare;
}

float SegmentLinearRegression::getSlope()
{
	return m_a;
}

float SegmentLinearRegression::getIntercept()
{
	return m_b;
}

QVector2D SegmentLinearRegression::getStart()
{
	return QVector2D(m_points[0]->x(), m_points[0]->y());
}

QVector2D SegmentLinearRegression::getEnd()
{
	return QVector2D(m_points[m_size-1]->x(), m_points[m_size-1]->y());
}

int SegmentLinearRegression::getStartIndex()
{
	return m_startInd;
}

int SegmentLinearRegression::getEndIndex()
{
	return m_endInd;
}

int SegmentLinearRegression::getSize()
{
	return m_size;
}

QVector2D* SegmentLinearRegression::getPoint(int index)
{
	return m_points[index];
}

float SegmentLinearRegression::getAssociatedP()
{
	return m_p;
}

int SegmentLinearRegression::getUniqueSharedID()
{
	return m_id;
}

QVector3D SegmentLinearRegression::getColor()
{
	return m_color;
}


void SegmentLinearRegression::setVar(float var)
{
	m_var = var;
}

void SegmentLinearRegression::setAltVar(QVector2D altVar)
{
	m_altVar = altVar;
}

void SegmentLinearRegression::setRSquare(float rsquare)
{
	m_rsquare = rsquare;
}

void SegmentLinearRegression::setSlope(float slope)
{
	m_a = slope;
}

void SegmentLinearRegression::setIntercept(float intercept)
{
	m_b = intercept;
}

/*void SegmentLinearRegression::setStart(QVector2D start)
{
	m_start = start;
}

void SegmentLinearRegression::setEnd(QVector2D end)
{
	m_end = end;
}
*/

void SegmentLinearRegression::setStartIndex(int startInd)
{
	m_startInd = startInd;
}

void SegmentLinearRegression::setEndIndex(int endInd)
{
	m_endInd = endInd;
}

void SegmentLinearRegression::setAssociatedP(float p)
{
	m_p = p;
}

void SegmentLinearRegression::setUniqueSharedID(int id)
{
	m_id = id;
}

void SegmentLinearRegression::setColor(QVector3D color)
{
	m_color = color;
}

