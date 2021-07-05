#include "profileprocessor.h"
#include <QDebug>

profileProcessor::profileProcessor(ccPolyline* profile, ccPolyline* generatrix) :
	m_inputProfile(profile), m_inputGeneratrix(generatrix)
{
	const int n = profile->size();
	m_inputX = new float[n];
	m_inputY = new float[n];

	m_profileID = m_inputProfile->getUniqueIDForDisplay();
}

profileProcessor::~profileProcessor()
{
	delete[] m_inputProfile;
	delete[] m_outputProfile;
	delete[] m_inputSegment;
	delete[] m_inputX;
	delete[] m_inputY;
}

QVector<QVector2D*> profileProcessor::profileToXY()
{

	//gets generatrix x position on profiles
	for (int i = 0; i < m_inputGeneratrix->size(); i++)
	{
		for (int j = 0; j < m_inputProfile->size(); j++) 
		{
			qDebug() << "gen x" << m_inputGeneratrix->getPoint(i)->x << "prof x" << m_inputProfile->getPoint(j)->x;
			if (m_inputGeneratrix->getPoint(i)->x == m_inputProfile->getPoint(j)->x)
				m_genPtIdx = j;
				//m_genX = i; //to compute throw along generatrix
		}

		//qDebug() << "print gen idx" << m_genPtIdx;
	}

	QVector<QVector2D*> coordinates;
	float s = 0;
	qDebug() << "input profile OK";
	qDebug() << "profile size" << m_inputProfile->size();
	
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

		if (i == m_inputProfile->size() - 2)
		{
			m_inputX[i+1] = s; //curvilinear abscissa (computed w/o z)
			m_inputY[i+1] = A->z; //z
			coordinates.append(new QVector2D(m_inputX[i+1], m_inputY[i+1]));
			m_inputProfilePts.append(A); //stores the (x,y,z) coordinates for later use
		}
	}

	/*//manually adding the last point 
	coordinates.append(new QVector2D(m_inputX[m_inputProfile->size() - 1], 
		m_inputY[m_inputProfile->size() - 1]));
	m_inputProfilePts.append(m_inputProfile->getPoint(m_inputProfile->size() - 1));*/

	qDebug() << "coordinates size" << coordinates.size();
	qDebug() << "coordinates OK";
	return coordinates;
}

ccPolyline* profileProcessor::segmentToProfile(std::vector<SegmentLinearRegression*> segments)
{
	qDebug() << "nb of profile points" << m_inputProfilePts.size();

	QString name = QString("Vertices (Profile ID #%1)").arg(m_profileID);
	m_outputCloud = new ccPointCloud(name.toStdString().c_str()); 
	m_outputCloud->reserve(m_inputProfilePts.size());

	//creates a scalar field to display segments using different colors
	//using sfs constraints us to use point clouds...
	name = QString("Segmentation P=%1").arg(segments[0]->getAssociatedP());
	//const char* c_defaultSFName = name.toStdString().c_str();

	const char c_defaultSFName[] = "Segmentation";
	int sfIdx = m_outputCloud->getScalarFieldIndexByName(c_defaultSFName);
	if (sfIdx < 0)
		sfIdx = m_outputCloud->addScalarField(c_defaultSFName);
	m_outputCloud->getScalarField(sfIdx)->reserve(m_inputProfilePts.size());
	m_outputCloud->setCurrentScalarField(sfIdx);

	ScalarType color;
	for (int i = 0; i < segments.size(); i++)
	{
		m_inputSegment = segments[i];
		int start = m_inputSegment->getStartIndex();
		int end = m_inputSegment->getEndIndex();

		const int n = m_inputSegment->getSize();

		color = m_inputSegment->getColor().x();

		for (int i = start; i < end; i++)
		{
			m_outputCloud->addPoint(CCVector3(m_inputProfilePts[i]->x,
				m_inputProfilePts[i]->y, m_inputProfilePts[i]->z));
			if (i == m_genPtIdx) m_outputCloud->getScalarField(sfIdx)->addElement(359); //assigns one predefined color to generatrix position
			else m_outputCloud->getScalarField(sfIdx)->addElement(color);
		}
	}

	//manually adding last point
	m_outputCloud->addPoint(CCVector3(m_inputProfilePts[m_inputProfilePts.size()-1]->x,
		m_inputProfilePts[m_inputProfilePts.size()-1]->y, m_inputProfilePts[m_inputProfilePts.size()-1]->z));
	m_outputCloud->getScalarField(sfIdx)->addElement(m_outputCloud->getPointScalarValue(m_inputProfilePts.size() - 2)); //last color

	qDebug() << "point cloud size" << m_outputCloud->size();

	m_outputCloud->setCurrentScalarField(sfIdx);
	m_outputCloud->getScalarField(sfIdx)->computeMinAndMax();
	m_outputCloud->setCurrentDisplayedScalarField(sfIdx);
	m_outputCloud->showSF(true);
	m_outputCloud->setPointSize(5);

	m_outputProfile = new ccPolyline(m_outputCloud, m_profileID);

	m_outputProfile->setForeground(true);
	m_outputProfile->set2DMode(false);
	m_outputProfile->reserve(m_inputProfilePts.size());
	m_outputProfile->addPointIndex(0, m_inputProfilePts.size());
	m_outputProfile->setWidth(5);
	m_outputProfile->addChild(m_outputCloud);

	qDebug() << "polyline size" << m_outputProfile->size();

	qDebug() << "segment conversion OK";
	return m_outputProfile;
}

char profileProcessor::getProfileID()
{
	return m_profileID;
}

