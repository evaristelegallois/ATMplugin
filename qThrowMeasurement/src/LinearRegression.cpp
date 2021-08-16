//##########################################################################
//#                                                                        #
//#                CLOUDCOMPARE PLUGIN: FracDense                          #
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
//#                COPYRIGHT: AMANDINE FRATANI, CEREGE                     #
//#                                                                        #
//##########################################################################

#include <vector>
#include "LinearRegression.h"
#include "StatisticalTools.h"

using namespace std;

LinearRegression::LinearRegression(const vector<double>& values_x, const vector<double>& values_y) :
	m_values_x(values_x), m_values_y(values_y)
{
	m_slope = covariance(m_values_x, m_values_y) / variance(m_values_x);
	m_ordinate = average(m_values_y) - m_slope * average(m_values_x);
}

