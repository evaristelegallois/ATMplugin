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
#include "LinearRegression.h"
#include "TreeNode.h"
#include <vector>
#include "HierarchicalAgglomerativeClustering.h"


class HAC_Average : virtual public HAC
{
public:
	/**
	*\brief: Hierarchical agglomerative clustering with average cost
	*\param: Matrix with the distance
	*/
	HAC_Average(const std::vector<std::vector<double>>& matrixData) : HAC(matrixData) {};

	/**
	*\brief: Compute the HAC with average cost until there is nbFinal group
	*\param: int nbFinal group
	*\return: TreeNode*
	*/
	virtual TreeNode* computeHAC(const int& nbFinal);
};