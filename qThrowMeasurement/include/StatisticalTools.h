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

#include <vector>

#include "LinearRegression.h"

#ifndef STATISTICAL_TOOLS
#define STATITISCAL_TOOLS

/**
*\brief: Computes the average value of a list of data
*\param: vector of double
*\return: double 
*/
double average(const std::vector<double>& values);

/**
*\brief: Computes the variance value of a list of data
*\param: vector of double
*\return: double
*/
double variance(const std::vector<double>& values);

/**
*\brief: Computes the standard deviation of a list of data
*\param: vector of double
*\return: double
*/
double standard_deviation(const std::vector<double>& values);

/**
*\brief: Computes the covariance between two lists of data of the same length
*\param 1: vector of double
*\param 2: vector of double, same length as the previous
*\return: double
*/
double covariance(const std::vector<double>& values_x, const std::vector<double>& values_y);

/**
*\brief: Computes the distance between two regressions slopes 
*\param 1: Linear regression 
*\param 2: Linear regression
*\return: double 
*/
double distance_comparison_slope(const LinearRegression& l1, const LinearRegression& l2);

#endif