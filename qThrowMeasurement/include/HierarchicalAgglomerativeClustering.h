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

#include "TreeNode.h"
#include <vector>

#ifndef HIERARCHICAL_AGGLOMERATIVE_CLUSTERING
#define HIERARCHICAL_AGGLOMERATIVE_CLUSTERING

class HAC
{
public:
	/**
	*\brief: General hierarchical agglomerative clustering
	*\param: Matrix with the distance
	*/
	HAC(const std::vector<std::vector<double>>& matrixData) : m_matrixData(matrixData) {};

	/**
	*\brief: Compute the HAC until there is nbFinal group
	*\param: int nbFinal group
	*\return: TreeNode*
	*/
	virtual TreeNode* computeHAC(const int& nbFinal) = 0;

protected:
	std::vector<std::vector<double>> m_matrixData;
};

#endif


