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

#include "TreeNode.h"
#include <fstream>
#include<string>

using namespace std;

TreeNode::TreeNode(const int& value, const int& step, const float& gap): m_value(value), m_step(step), 
m_gap(gap), m_right(0), m_left(0)
{
}

void TreeNode::setNodeLeft(TreeNode* NodeLeft)
{
	delete m_left;
	m_left = NodeLeft;
}

void TreeNode::setNodeRight(TreeNode* NodeRight)
{
	delete m_right;
	m_right = NodeRight;
}

void TreeNode::printTree(string path)
{
	ofstream file{ path, ios::app };

	if (file.bad())
	{
		//cout << "file not open" << endl;

		return;
	}

	if (m_left == 0)
	{
		file << m_step << "\t" << m_value << "\t" << m_gap << "\t" << 0 << "\t" << 0 << endl;
		file.close();
		return;
	}

	file << m_step << "\t" << m_value << "\t" << "\t" << m_gap << "\t" << m_left->getValue() << "\t" << m_right->getValue() << endl;

	file.close();

	m_left->printTree(path);
	m_right->printTree(path);
}

TreeNode::~TreeNode()
{
	delete m_left;
	m_left = 0;

	delete m_right;
	m_right = 0;
}