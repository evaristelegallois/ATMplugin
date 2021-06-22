#include "profileprocessor.h"
#include <QDebug>

profileProcessor::profileProcessor(ccPolyline* profile) : m_inputProfile(profile)
{
	const int n = profile->size();
	m_inputX = new float[n];
	m_inputY = new float[n];
}

profileProcessor::profileProcessor(SegmentLinearRegression* segment) : m_inputSegment(segment)
{

}

profileProcessor::~profileProcessor()
{
	delete m_inputProfile;
	delete m_outputProfile;
	delete m_inputSegment;
	delete m_inputX;
	delete m_inputY;
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
		//virtual const CCVector3* getCurrentPointCoordinates() const;

		s += AB.length();

		m_inputX[i] = s; //curvilinear abscissa (computed w/o z)
		m_inputY[i] = A->z; //z
		coordinates.append(new QVector2D(m_inputX[i], m_inputY[i]));
		m_inputProfilePts.append(A); //stores the (x,y,z) coordinates for later use
	}

	//manually adding the last point 
	coordinates.append(new QVector2D(m_inputX[m_inputProfile->size() - 1], m_inputY[m_inputProfile->size() - 1]));
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

	//setColor(const ccColor::Rgb & col)

	//! Returns the current point associated scalar value
	//inline virtual ScalarType getCurrentPointScalarValue() const 

	//! Sets the current point associated scalar value
	//inline virtual void setCurrentPointScalarValue(ScalarType value)
	qDebug() << "nb of segment vertices" << m_inputSegment->getSize();
	qDebug() << "nb of profile points" << m_inputProfilePts.size();

	ccPointCloud* vertices = new ccPointCloud("Vertices", 0);
	vertices->reserve(m_inputSegment->getSize());

	/*int index = vertices->addScalarField("Segmentation w/ p= " + segment->getP());
	vertices->setCurrentInScalarField(index);*/

	const char c_defaultSFName[] = "Segmentation";
	int sfIdx = vertices->getScalarFieldIndexByName(c_defaultSFName);
	if (sfIdx < 0)
		sfIdx = vertices->addScalarField(c_defaultSFName);

	vertices->setCurrentScalarField(sfIdx); //in?
	qDebug() << "adding SF OK";

	ScalarType color = m_inputSegment->getColor().x();

	for (int i = 0; i < m_inputSegment->getSize(); i++) //idx out of range issue w/ multiple profiles
	{
		QVector2D currentPoint = m_inputSegment->getPoint(i);
		const CCVector3* associatedPoint = m_inputProfilePts[i];

		qDebug() << "creating vectors OK";

		m_outputX[i] = associatedPoint->x;
		m_outputY[i] = associatedPoint->y;
		m_outputZ[i] = associatedPoint->z; // y = z

		vertices->addPoint(CCVector3(m_outputX[i], m_outputY[i], m_outputZ[i]));

		qDebug() << "creating vertices OK";

		/*//sets profile's color (NOT a scalar field)
		QVector3D color = segment->getColor();
		ccColor::Rgb colorRGB; //to edit; HSV colors needed
		colorRGB.r = color.x();
		colorRGB.g = color.y();
		colorRGB.b = color.z();*/

		//sets color as a scalar field
		vertices->setPointScalarValue(i, color); //invalid vector subscript
		qDebug() << "SF computation OK";
	}

	if (sfIdx >= 0)
	{
		vertices->getScalarField(sfIdx)->computeMinAndMax();
	}

	m_outputProfile = new ccPolyline(vertices);
	m_outputProfile->addPointIndex(0, m_inputSegment->getSize());

	//m_outputProfile->setColor(colorRGB);
	m_outputProfile->setWidth(5);
	qDebug() << "segment conversion OK";

	return m_outputProfile;
}