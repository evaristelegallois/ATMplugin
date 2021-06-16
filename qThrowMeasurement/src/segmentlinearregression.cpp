#include "segmentlinearregression.h"

SegmentLinearRegression::SegmentLinearRegression(int startInd, int endInd, float* x, float* y) :
	m_startInd(startInd), m_endInd(endInd)
{
	//allocate size for m_points?
	int k = 0;
	for (int i = m_startInd; i < m_endInd; i++)
	{
		m_points[i] = QVector2D(x[i], y[i]);
		k++;
	}
	m_size = k;
}

SegmentLinearRegression::~SegmentLinearRegression()
{
	m_points = nullptr;
}

float SegmentLinearRegression::getVar()
{
	return m_var;
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
	return m_start;
}

QVector2D SegmentLinearRegression::getEnd()
{
	return m_end;
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

QVector2D SegmentLinearRegression::getPoint(int index)
{
	return m_points[index];
}

QVector3D SegmentLinearRegression::getColor()
{
	return m_color;
}

void SegmentLinearRegression::setVar(float var)
{
	m_var = var;
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
void SegmentLinearRegression::setStart(QVector2D start)
{
	m_start = start;
}

void SegmentLinearRegression::setEnd(QVector2D end)
{
	m_end = end;
}
void SegmentLinearRegression::setStartIndex(int startInd)
{
	m_startInd = startInd;
}

void SegmentLinearRegression::setEndIndex(int endInd)
{
	m_endInd = endInd;
}

void SegmentLinearRegression::setColor(QVector3D color)
{
	m_color = color;
}