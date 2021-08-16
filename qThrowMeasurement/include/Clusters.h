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

// Local
#include "TreeNode.h"

// Extern
#include <vector>
#include <string>

#ifndef FRACDENSE_CLUSTER
#define FRACDENSE_CLUSTER

class Clusters
{
public:
	/**
	*\brief: Cut the tree at the threshold
	*\param 1: TreeNode*
	*\param 2: height where the tree will be cut
	*/
	explicit Clusters(TreeNode* Tree_complete, const double& threshold);

	/**
	*\brief: export group in ascii file
	*\param 1: string file path
	*\param 2: name of column
	*\return:
	*/
	void exportColumnBasedFile(const std::string& filepath, const std::vector<std::string>& name = std::vector<std::string>());

	/**
	*\brief: return vector of cluster
	*\param: Cluster number
	*\return: vector of TreeNode*
	*/
	inline const std::vector<TreeNode*>& getCluster(const int& numCluster) const;

	/**
	*\brief: return number of cluster
	*\return: int
	*/
	inline const int& get_number_clusters() const;

private:

	std::vector<std::vector<TreeNode*>> m_clusters;

	TreeNode* m_complete;

	double m_threshold;

	/**
	*\brief: Cut the tree at the threshold
	*\param 1: TreeNode*
	*\param 2: threshold double
	*\param 3: clusterNumber
	*/
	void compute_threshold_tree( TreeNode* root, const double& threshold, int& clusterNumber);

	/**
	*\brief: Affect TreeNode leaf to his cluster
	*\param 1: TreeNode*
	*\return 2:  int Cluster number
	*/
	void affect_group_cluster_tree(TreeNode* root, const int& clusterNumber);
};

inline const int& Clusters::get_number_clusters() const
{
	return static_cast<int>(m_clusters.size());
}

inline const std::vector<TreeNode*>& Clusters::getCluster(const int& numCluster) const
{
	return m_clusters[numCluster];
}

#endif
