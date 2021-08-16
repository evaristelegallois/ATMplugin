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

/**
*\brief: Compute the matrix distance 
*\param: vector of Linear Regression
*\return: vector<vector<double>>
*/
std::vector<std::vector<double>> matrixDistance(const std::vector<LinearRegression*>& lrs);