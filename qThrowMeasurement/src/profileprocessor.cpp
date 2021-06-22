#include "profileprocessor.h"
#include <QDebug>

profileProcessor::profileProcessor(ccPolyline* profile) : m_inputProfile(profile)
{
	const int n = profile->size();
	m_inputX = new float[n];
	m_inputY = new float[n];
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
	//virtual const CCVector3* getCurrentPointCoordinates() const;
	
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
	}

	//manually adding the last point 
	coordinates.append(new QVector2D(m_inputX[m_inputProfile->size() - 1], 
		m_inputY[m_inputProfile->size() - 1]));
	m_inputProfilePts.append(m_inputProfile->getPoint(m_inputProfile->size() - 1));

	qDebug() << "coordinates OK";
	return coordinates;
}

ccPolyline* profileProcessor::segmentToProfile(SegmentLinearRegression* segment)
{
	m_inputSegment = segment;
	const int n = m_inputSegment->getSize();
	m_outputX = new float[n];
	m_outputY = new float[n];
	m_outputZ = new float[n];

	qDebug() << "nb of segment vertices" << n;
	qDebug() << "nb of profile points" << m_inputProfilePts.size();

	m_outputCloud = new ccPointCloud("Vertices"); //need to rename SF in loop? or give a vector of segments
	m_outputCloud->reserve(n);

	//creates a scalar field to display segments using different colors
	const char c_defaultSFName[] = "Segmentation";
	int sfIdx = m_outputCloud->getScalarFieldIndexByName(c_defaultSFName);
	if (sfIdx < 0)
		sfIdx = m_outputCloud->addScalarField(c_defaultSFName);
	m_outputCloud->getScalarField(sfIdx)->reserve(n);
	m_outputCloud->setCurrentScalarField(sfIdx); //in?

	ScalarType color = m_inputSegment->getColor().x();
	//qDebug() << "color value" << color;

	for (int i = 0; i < n; i++) //idx out of range issue w/ multiple profiles
	{
		m_outputCloud->addPoint(CCVector3(m_inputProfilePts[i]->x,
			m_inputProfilePts[i]->y, m_inputProfilePts[i]->z));
		m_outputCloud->getScalarField(sfIdx)->addElement(color);
	}

	m_outputCloud->setCurrentScalarField(sfIdx);
	m_outputCloud->getScalarField(sfIdx)->computeMinAndMax();
	m_outputCloud->setCurrentDisplayedScalarField(sfIdx);
	m_outputCloud->showSF(true);

	m_outputProfile = new ccPolyline(m_outputCloud);

	m_outputProfile->setForeground(true);
	m_outputProfile->setTempColor(ccColor::red);
	m_outputProfile->set2DMode(true);
	m_outputProfile->reserve(n);
	m_outputProfile->addPointIndex(0, n);
	m_outputProfile->setWidth(5);
	m_outputProfile->addChild(m_outputCloud);



	qDebug() << "segment conversion OK";
	return m_outputProfile;
}
/*
void profileProcessor::displayProfile(ccMainAppInterface* app)
{
	app->getActiveGLWindow()->addToDB(m_outputProfile);
}*/