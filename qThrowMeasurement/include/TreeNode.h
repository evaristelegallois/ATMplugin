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

#ifndef FRACDENSE_TREE_NODE
#define FRACDENSE_TREE_NODE

#include <string>

class TreeNode
{
public :
	/**
	*\brief: Node object used to compute HAC
	*\param 1: Value of the node
	*\param 2: Step of the HAC
	*\param 3: Height to join two node
	*/
	TreeNode(const int& value, const int& step, const float& gap); //Build the object

	/**
	*\brief: Get Value of the Node
	*\return: int
	*/
	inline const int& getValue();

	/**
	*\brief: Return current step
	*\return: int
	*/
	inline const int& getStep();

	/**
	*\brief: Return height needed to join node left and node right
	*\return: float
	*/
	inline const float& getGap();

	/**
	*\brief: Return a pointer to the node left
	*\return: TreeNode*
	*/
	inline TreeNode* getNodeLeft();

	/**
	*\brief: Return a pointer to the node right
	*\return: TreeNode*
	*/
	inline TreeNode* getNodeRight();

	/**
	*\brief: Set the node left with a pointer
	*\param: a pointer with a TreeNode
	*\return: None
	*/
	void setNodeLeft(TreeNode* NodeLeft);

	/**
	*\brief: Set the node right with a pointer
	*\param: a pointer with a TreeNode
	*\return: None
	*/
	void setNodeRight(TreeNode* NodeRight);

	/**
	*\brief: Write Ascii file with the value of Node and each descendant
	*\param: Path 
	*\return: None
	*/
	void printTree(std::string path);

	~TreeNode();

private :
	int m_value, m_step;
	float m_gap, m_cumulatedGap;

	TreeNode* m_left;
	TreeNode* m_right;
};

inline const int& TreeNode::getValue()
{
	return m_value;
}

inline const int& TreeNode::getStep()
{
	return m_step;
}

inline const float& TreeNode::getGap()
{
	return m_gap;
}

inline TreeNode* TreeNode::getNodeLeft()
{
	return m_left;
}

inline TreeNode* TreeNode::getNodeRight()
{
	return m_right;
}


#endif // !FRACDENSE_TREE_NODE

