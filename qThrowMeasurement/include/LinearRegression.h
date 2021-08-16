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

#pragma once

#ifndef LINEAR_REGRESSION
#define LINEAR_REGRESSION

#include <vector>

class LinearRegression
{
public :
	/**
	*\brief: Computes linear regression with the least-squares method
	*\param 1: List of x positions
	*\param 2: List of y positions
	*/
	explicit LinearRegression(const std::vector<double>& values_x, const std::vector<double>& values_y);

	/**
	*\brief: Get the slope of the linear regression calculated
	*\return: double
	*/
	inline const double& getSlope() const;

	/**
	*\brief: Get the ordinate value of the linear regression calculated
	*\return: double
	*/
	inline const double& getOrdinate() const;

	/**
	*\brief: Get the list of the x values
	*\return: vector of double
	*/
	inline const std::vector<double>& getValues_x() const;

	/**
	*\brief: Get the list of the y values
	*\return: vector of double
	*/
	inline const std::vector<double>& getValues_y() const;

private :
	
	double m_slope;

	double m_ordinate;

	std::vector<double> m_values_x;

	std::vector<double> m_values_y;
};

inline const double& LinearRegression::getSlope() const
{
	return m_slope;
}

inline const double& LinearRegression::getOrdinate() const
{
	return m_ordinate;
}

inline const std::vector<double>& LinearRegression::getValues_x() const
{
	return m_values_x;
}

inline const std::vector<double>& LinearRegression::getValues_y() const
{
	return m_values_y;
}

#endif
