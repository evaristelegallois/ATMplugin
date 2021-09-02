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
//#                      COPYRIGHT: Gabriel Parel                          #
//#                                                                        #
//##########################################################################

#include "segmentlinearregression.h"

SegmentLinearRegression::SegmentLinearRegression(int startInd, int endInd, float* x, float* y) :
	m_startInd(startInd), m_endInd(endInd),
	m_id(-1), m_p(-1), m_transectPos(-1)
{
	m_size = endInd - startInd + 1;

	for (int i = m_startInd; i < m_endInd + 1; i++) m_points.push_back(new QVector2D(x[i], y[i]));
	m_a = 0, m_b = 0, m_r = 0, m_rsquare = 0, m_var = 0;
}

SegmentLinearRegression::~SegmentLinearRegression()
{
	m_points.clear();
}

//! getters
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

int SegmentLinearRegression::getTransectPosition()
{
	if (m_transectPos < 0) m_transectPos = 0;
	return m_transectPos;
}

QVector3D SegmentLinearRegression::getColor()
{
	return m_color;
}

//! setters
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

void SegmentLinearRegression::setTransectPosition(int idx)
{
	m_transectPos = idx;
}

void SegmentLinearRegression::setColor(QVector3D color)
{
	m_color = color;
}

