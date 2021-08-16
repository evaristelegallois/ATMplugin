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

vector<vector<double>> matrixDistance(const vector<LinearRegression*>& lrs)
{
	int sizeLinearRegression = static_cast<int>(lrs.size());

	vector<vector<double>> matrix;

	for (int i = 0; i < sizeLinearRegression; i++)
	{
		vector<double> ligne(sizeLinearRegression, 0);
		matrix.push_back(ligne);
	}

	for (int l = 0; l < sizeLinearRegression; l++)
	{
		for (int c = l + 1; c < sizeLinearRegression; c++)
		{
			matrix[l][c] = distance_comparison_slope(*lrs[l], *lrs[c]);
			matrix[c][l] = matrix[l][c];
		}
	}

	return matrix;
}

