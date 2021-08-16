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


//Local
#include "Clusters.h"

//Extern
#include <vector>
#include <string>
#include <fstream>

using namespace std;

// Public
Clusters::Clusters(TreeNode* Tree, const double& threshold) : m_complete(Tree), m_threshold(threshold)
{
	int nb_cluster = 0;
	compute_threshold_tree(m_complete, m_threshold, nb_cluster);
}

void Clusters::exportColumnBasedFile(const string& filepath, const vector<string>& name)
{
	ofstream file(filepath);
	if (file)
	{
		file << "truncate at " << m_threshold <<endl;
		file << "element\tgroup" << endl;

		for (int c = 0; c < m_clusters.size(); c++)
		{
			for (int e = 0; e < m_clusters.at(c).size(); e++)
			{
				if (name.empty())
				{
					file << m_clusters.at(c).at(e)->getValue() << "\t" << c + 1 << endl;
				}
				else
				{
					file << name[m_clusters.at(c).at(e)->getValue()] << "\t" << c + 1 << endl;
				}
			}
		}
	}

	file.close();
}

// Private
void Clusters::compute_threshold_tree(TreeNode* root, const double& threshold, int& clusterNumber)
{

	if (root->getNodeLeft() != 0)
	{
		int indexRun = -1;

		// If both each node cross the threshold, we dont go further (indexRun = 2), 
		if (root->getGap() >= threshold && root->getNodeLeft()->getGap() < threshold)
		{
			clusterNumber++;
			affect_group_cluster_tree(root->getNodeLeft(), clusterNumber);
			// affect_group_cluster_tree(root->getNodeRight(), nb_clust);
			indexRun = 1;
		}

		if (root->getGap() >= threshold && root->getNodeRight()->getGap() < threshold)
		{
			clusterNumber++;
			affect_group_cluster_tree(root->getNodeRight(), clusterNumber);

			if (indexRun == 1)
			{
				indexRun = 2; // both nodes cross the threshold
			}
			else
			{
				indexRun = 0; // only the rigth node cross
			}
		}

		if (indexRun == 0) // Checking the left side of the tree
		{
			compute_threshold_tree(root->getNodeLeft(), threshold, clusterNumber);
		}
		else if (indexRun == 1) // Checking the rigth side of the tree
		{
			compute_threshold_tree(root->getNodeRight(), threshold, clusterNumber);
		}
		else if (indexRun == -1) // Checking both side
		{
			compute_threshold_tree(root->getNodeLeft(), threshold, clusterNumber);
			compute_threshold_tree(root->getNodeRight(), threshold, clusterNumber);
		}
	}
}

void Clusters::affect_group_cluster_tree(TreeNode* root, const int& clusterNumber)
{
	int nb_clust = clusterNumber;


	if (root->getNodeLeft() != 0) //if root is not a leaf
	{
		affect_group_cluster_tree(root->getNodeLeft(), nb_clust);
		affect_group_cluster_tree(root->getNodeRight(), nb_clust);
	}
	else
	{
		if (m_clusters.size() < nb_clust)
		{
			m_clusters.push_back(vector<TreeNode*>());
		}

		m_clusters[nb_clust - 1].push_back(root);
	}
}

