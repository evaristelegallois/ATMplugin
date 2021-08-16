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


#include "HAC_Average.h"
#include "LinearRegression.h"
#include "TreeNode.h"
#include <vector>

using namespace std;

TreeNode* HAC_Average::computeHAC(const int& nbFinal)
{
	TreeNode* dendoRoot;
	TreeNode* tmpRoot;
	TreeNode* tmpNodeRight;
	TreeNode* tmpNodeLeft;

	int step = 0;
	vector<vector<double>> tmpMatrix;
	vector<vector<double>> comparisonMatrix = m_matrixData;

	const int sizeMat = static_cast<int>(comparisonMatrix.size());

	// Initialisation of the clusterSizeList containing the size of each cluster
	// and the unmergedRootList a list to access all the unmerged root

	int nbCLuster = sizeMat;

	vector<int> clusterSizeList;
	vector<TreeNode*> unmergedRootList;

	for (int i = 0; i < sizeMat; i++)
	{
		clusterSizeList.push_back(1);
		unmergedRootList.push_back(new TreeNode(i, 0, 0));
	}


	while (nbCLuster > nbFinal) //Stop when the desired number of cluster is reach
	{
		// Calculation of the best clusters to merge at a step 'step'

		float min = static_cast<float>(comparisonMatrix[0][1]);
		float tmpVal = 0.;
		unsigned int x = 0;
		unsigned int y = 1;

		for (int r = 0; r < sizeMat - 1 - step; r++)
		{
			for (int c = r + 1; c < sizeMat - step; c++)
			{
				if (comparisonMatrix[r][c] < min)
				{
					min = static_cast<float>(comparisonMatrix[r][c]);
					x = r;
					y = c;
				}
			}
		}


		// Recalculation of the distance matrix with acknowledging the change made by grouping clusters

		/*
		Note :
		x = index in the matrix (but also the cluster_List) of one of the cluster merged (the one who is kept)
		y = index in the matrix (but also the cluster_List) of one of the cluster merged (the one who is deleted)
		cluster_List is a matrix representing for each cluster the index of which scanline belongs
		*/


		tmpMatrix = comparisonMatrix;

		for (int i = 0; i < nbCLuster; i++)
		{
			for (int j = 0; j < nbCLuster; j++)
			{
				if (i == j)
				{
					tmpMatrix[i][i] = 0.;
				}
				else if (i == x)
				{
					double d1, d2;
					double n1, n2;

					d1 = comparisonMatrix[x][j];
					d2 = comparisonMatrix[y][j];

					n1 = static_cast<double>(clusterSizeList[x]);
					n2 = static_cast<double>(clusterSizeList[y]);

					double d = (n1 * d1 + n2 * d2) / (n1 + n2);

					tmpMatrix[i][j] = d;
					tmpMatrix[j][i] = d;
				}
			}
		}

		tmpMatrix.erase(tmpMatrix.begin() + (y));

		for (unsigned int i = 0; i < tmpMatrix.size(); i++)
		{
			tmpMatrix[i].erase(tmpMatrix[i].begin() + y);
		}
		comparisonMatrix = tmpMatrix;

		clusterSizeList[x] += clusterSizeList[y];

		clusterSizeList.erase(clusterSizeList.begin() + y);

		tmpNodeLeft = unmergedRootList[x];
		tmpNodeRight = unmergedRootList[y];

		tmpRoot = new TreeNode(unmergedRootList[x]->getValue(), step + 1, min);
		tmpRoot->setNodeLeft(tmpNodeLeft);
		tmpRoot->setNodeRight(tmpNodeRight);

		unmergedRootList[x] = tmpRoot;

		unmergedRootList.erase(unmergedRootList.begin() + y);

		nbCLuster--;
		step++;
	}

	dendoRoot = unmergedRootList[0];

	return dendoRoot;
}