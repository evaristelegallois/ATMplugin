//##########################################################################
//#                                                                        #
//#                    CLOUDCOMPARE PLUGIN: ATMPlugin                      #
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
//#                      COPYRIGHT: Gabriel Parel                          #
//#                                                                        #
//##########################################################################

#include "profileprocessor.h"
#include <QDebug>

profileProcessor::profileProcessor(ccPolyline* profile, ccPolyline* generatrix) :
	m_inputProfile(profile), m_inputGeneratrix(generatrix), 
	m_outputX(nullptr), m_outputY(nullptr), m_outputZ(nullptr),
	m_inputSegment(nullptr), m_outputCloud(nullptr), m_outputProfile(nullptr),
	m_genPtIdx(-1)
{
	const int n = profile->size();
	m_inputX = new float[n];
	m_inputY = new float[n];

	m_profileID = m_inputProfile->getUniqueIDForDisplay();
}

profileProcessor::~profileProcessor()
{
	delete[] m_inputGeneratrix;
	delete[] m_inputProfile;
	delete[] m_outputProfile;
	delete[] m_inputSegment;
	delete[] m_outputCloud;
	delete[] m_inputX;
	delete[] m_inputY;
	delete[] m_outputX;
	delete[] m_outputY;
	delete[] m_outputZ;
}

QVector<QVector2D*> profileProcessor::profileToXY()
{
	QVector<QVector2D*> coordinates;
	QVector2D intercept;
	float s = 0; //curvilinear abscissa

	//gets generatrix x position on profiles, if it exists
	for (int j = 0; j < m_inputGeneratrix->size() - 1; j++)
	{
		for (int i = 0; i < m_inputProfile->size() - 1; i++)
		{
			QVector2D genStart = QVector2D(m_inputGeneratrix->getPoint(j)->x, 
				m_inputGeneratrix->getPoint(j)->y);
			QVector2D genEnd = QVector2D(m_inputGeneratrix->getPoint(j + 1)->x, 
				m_inputGeneratrix->getPoint(j + 1)->y);
			QVector2D profStart = QVector2D(m_inputProfile->getPoint(i)->x, 
				m_inputProfile->getPoint(i)->y);
			QVector2D profEnd = QVector2D(m_inputProfile->getPoint(i+1)->x, 
				m_inputProfile->getPoint(i + 1)->y);

			intercept = getIntersection(genStart, genEnd, profStart, profEnd);

			if (intercept.x() + 0.01 * intercept.x() < m_inputProfile->getPoint(i)->x
				|| intercept.x() - 0.01 * intercept.x() < m_inputProfile->getPoint(i)->x)
			{
				m_genPtIdx = i;
				break; //to save time/memory, no need to continue looping after finding the intercept
			}
		}
		if (m_genPtIdx != 0) break; //idem
	}
	
	//(x,y,z) -> (s,z) conversion
	for (int i = 0; i < m_inputProfile->size()-1; i++) 
	{
		const CCVector3* A = m_inputProfile->getPoint(i);
		const CCVector3* B = m_inputProfile->getPoint(i+1);
		const QVector2D AB = QVector2D(B->x - A->x, B->y - A->y);

		s += AB.length();

		m_inputX[i] = s; //curvilinear abscissa (computed w/o z)
		m_inputY[i] = A->z; //z
		coordinates.append(new QVector2D(m_inputX[i], m_inputY[i]));
		m_inputProfilePts.append(A); //stores the (x,y,z) coordinates for later use

		//adding the last point
		if (i == m_inputProfile->size() - 2)
		{
			m_inputX[i+1] = s; //curvilinear abscissa (computed w/o z)
			m_inputY[i+1] = A->z; //z
			coordinates.append(new QVector2D(m_inputX[i+1], m_inputY[i+1]));
			m_inputProfilePts.append(A); //stores the (x,y,z) coordinates for later use
		}
	}

	return coordinates;
}

ccPolyline* profileProcessor::segmentToProfile(std::vector<SegmentLinearRegression*> segments)
{
	//creates the output point cloud
	QString name = QString("Vertices (Profile ID #%1, P=%2)").arg(m_profileID)
		.arg(segments[0]->getAssociatedP());
	m_outputCloud = new ccPointCloud(name.toStdString().c_str()); 
	m_outputCloud->reserve(m_inputProfilePts.size());

	//creates a scalar field to display segments using different colors
	name = QString("Segmentation P=%1").arg(segments[0]->getAssociatedP());
	//const char* c_defaultSFName = name.toStdString().c_str();

	const char c_defaultSFName[] = "Segmentation";
	int sfIdx = m_outputCloud->getScalarFieldIndexByName(c_defaultSFName); 
	//can't use name here, not const

	if (sfIdx < 0)
		sfIdx = m_outputCloud->addScalarField(c_defaultSFName);
	m_outputCloud->getScalarField(sfIdx)->reserve(m_inputProfilePts.size()); 
	//! scalar field size HAS TO EQUAL point cloud size
	m_outputCloud->setCurrentScalarField(sfIdx);

	ScalarType color;
	for (int i = 0; i < segments.size(); i++)
	{
		m_inputSegment = segments[i];
		m_inputSegment->setUniqueSharedID(m_profileID); //for later use
		m_inputSegment->setTransectPosition(m_genPtIdx); //for later use

		int start = m_inputSegment->getStartIndex();
		int end = m_inputSegment->getEndIndex();

		const int n = m_inputSegment->getSize();

		color = m_inputSegment->getColor().x();

		//(s,z) -> (x,y,z) conversion
		for (int i = start; i < end; i++)
		{
			m_outputCloud->addPoint(CCVector3(m_inputProfilePts[i]->x,
				m_inputProfilePts[i]->y, m_inputProfilePts[i]->z));
			if ( i == m_genPtIdx )
			{
				m_outputCloud->getScalarField(sfIdx)->addElement(359); 
				//assigns one predefined color to generatrix position
			}
			else m_outputCloud->getScalarField(sfIdx)->addElement(color);
			//assigns one color per segment
		}
	}

	//manually adding last point
	m_outputCloud->addPoint(CCVector3(m_inputProfilePts[m_inputProfilePts.size() - 1]->x,
		m_inputProfilePts[m_inputProfilePts.size()-1]->y, 
		m_inputProfilePts[m_inputProfilePts.size()-1]->z));

	//! scalar field size HAS TO BE EQUAL TO point cloud size, if not => it crashes here

	//manually adding last color
	m_outputCloud->getScalarField(sfIdx)->addElement(
		m_outputCloud->getPointScalarValue(m_inputProfilePts.size() - 2)); //last color

	//setting point cloud parameters
	m_outputCloud->setCurrentScalarField(sfIdx);
	m_outputCloud->getScalarField(sfIdx)->computeMinAndMax();
	m_outputCloud->setCurrentDisplayedScalarField(sfIdx);
	m_outputCloud->showSF(true);
	m_outputCloud->setPointSize(5);

	//creating the output polyline; gets same ID as initial polyline
	m_outputProfile = new ccPolyline(m_outputCloud, m_profileID);

	//setting polyline parameters here
	m_outputProfile->setForeground(true);
	m_outputProfile->set2DMode(false);
	m_outputProfile->reserve(m_inputProfilePts.size());
	m_outputProfile->addPointIndex(0, m_inputProfilePts.size());
	m_outputProfile->setWidth(1);
	m_outputProfile->addChild(m_outputCloud);

	return m_outputProfile;
}


//gets intersection between two segments, caracterized by two points each (start & end)
QVector2D profileProcessor::getIntersection(QVector2D p1, QVector2D p2, QVector2D q1, QVector2D q2)
{
	QVector2D s1, s2, i; // s1 & s2 sums, i intercept
	float s, t; // parameters, in[0, 1]; example, with t : r(t) = (1-t) * p1 + t * p2
	s1 = p2 - p1;
	s2 = q2 - q1;

	if ((-s2.x() * s1.y() + s1.x() * s2.y()) == 0) i = QVector2D(0, 0); // No collision
	bool isDenomPositive = (-s2.x() * s1.y() + s1.x() * s2.y()) > 0;

	if (((s1.x() * (p1.y() - q1.y()) - s1.y() * (p1.x() - q1.x())) < 0) 
		== isDenomPositive) i = QVector2D(0, 0); // No collision
	if (((s2.x() * (p1.y() - q1.y()) - s2.y() * (p1.x() - q1.x())) < 0) 
		== isDenomPositive) i = QVector2D(0, 0); // No collision

	if ((((s1.x() * (p1.y() - q1.y()) - s1.y() * (p1.x() - q1.x())) > 
		(-s2.x() * s1.y() + s1.x() * s2.y())) == isDenomPositive)
		|| (((s2.x() * (p1.y() - q1.y()) - s2.y() * (p1.x() - q1.x())) > 
			(-s2.x() * s1.y() + s1.x() * s2.y())) == isDenomPositive))
		i = QVector2D(0, 0); // No collision

	// s & t in [0, 1]
	s = (-s1.y() * (p1.x() - q1.x()) + s1.x() * (p1.y() - q1.y())) / 
		(-s2.x() * s1.y() + s1.x() * s2.y());
	t = (s2.x() * (p1.y() - q1.y()) - s2.y() * (p1.x() - q1.x())) / 
		(-s2.x() * s1.y() + s1.x() * s2.y());

	if (t >= 0 && t <= 1) i = QVector2D(p1.x() + (t * s1.x()), p1.y() + (t * s1.y())); //collision
	else i = QVector2D(0, 0); // No collision

	return i;
}

int profileProcessor::getProfileID()
{
	return m_profileID;
}


