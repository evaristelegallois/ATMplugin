#include "profileprocessor.h"

profileProcessor::profileProcessor(ccPolyline* profile) : m_inputProfile(profile)
{

}

profileProcessor::profileProcessor(SegmentLinearRegression* segment) : m_inputSegment(segment)
{

}

profileProcessor::~profileProcessor()
{
	delete m_inputProfile;
	delete m_outputProfile;
	delete m_inputSegment;
	//delete x & y lists
}

QVector<QVector2D> profileProcessor::profileToXY()
{
	//virtual const CCVector3* getCurrentPointCoordinates() const;
	
	QVector<QVector2D> coordinates;
	float s = 0;
	for (int i = 0; i < m_inputProfile->size(); i++)
	{
		const CCVector3* A = m_inputProfile->getPoint(i);
		const CCVector3* B = m_inputProfile->getPoint(i+1);
		const QVector2D AB = QVector2D(B->x - A->x, B->y - A->y);
		//virtual const CCVector3* getCurrentPointCoordinates() const;

		s += AB.length();

		m_inputX[i] = s; //curvilinear abscissa (computed w/o z)
		m_inputY[i] = A->z; //z
		coordinates[i] = QVector2D(m_inputX[i], m_inputY[i]);
		m_inputProfilePts[i] = A; //stores the (x,y,z) coordinates for later use
	}

	return coordinates;
}

ccPolyline* profileProcessor::segmentToProfile()
{
	//setColor(const ccColor::Rgb & col)

	//! Returns the current point associated scalar value
	//inline virtual ScalarType getCurrentPointScalarValue() const 

	//! Sets the current point associated scalar value
	//inline virtual void setCurrentPointScalarValue(ScalarType value)

	ccPointCloud* vertices = new ccPointCloud("Vertices", 0);
	vertices->reserve(m_inputSegment->getSize());

	/*int index = vertices->addScalarField("Segmentation w/ p= " + segment->getP());
	vertices->setCurrentInScalarField(index);*/

	for (int i = 0; i < m_inputSegment->getSize(); i++)
	{
		QVector2D currentPoint = m_inputSegment->getPoint(i);
		const CCVector3* associatedPoint = m_inputProfilePts[i];

		m_outputX[i] = associatedPoint->x;
		m_outputY[i] = associatedPoint->y;
		m_outputZ[i] = associatedPoint->z; // y = z

		vertices->addPoint(CCVector3(m_outputX[i], m_outputY[i], m_outputZ[i]));

		/*//sets profile's color (NOT a scalar field)
		QVector3D color = segment->getColor();
		ccColor::Rgb colorRGB; //to edit; HSV colors needed
		colorRGB.r = color.x();
		colorRGB.g = color.y();
		colorRGB.b = color.z();*/

		//sets color as a scalar field
		vertices->setPointScalarValue(i, m_inputSegment->getColor().x());
	}

	m_outputProfile = new ccPolyline(vertices);
	m_outputProfile->addPointIndex(0, m_inputSegment->getSize());

	//m_outputProfile->setColor(colorRGB);
	m_outputProfile->setWidth(5);

	return m_outputProfile;
}