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
	delete m_outputSegment;
	//delete x & y lists
}

QVector2D profileProcessor::profileToXY(ccPolyline* profile)
{
	//setColor(const ccColor::Rgb & col)
	//CCVector3* getNextPoint()

	//virtual const CCVector3* getCurrentPointCoordinates() const;
	
	//! Returns the current point associated scalar value
	//inline virtual ScalarType getCurrentPointScalarValue() const 

	//! Sets the current point associated scalar value
	//inline virtual void setCurrentPointScalarValue(ScalarType value)

	for (int i = 0; i < profile->size(); i++)
	{
		const CCVector3 *currentPoint = profile->getPoint(i);
		//virtual const CCVector3* getCurrentPointCoordinates() const;

		//not the curvilinear abcsissa YET
		m_inputX[i] = currentPoint->x;
		m_inputY[i] = currentPoint->y;
	}
}

ccPolyline profileProcessor::segmentToProfile(SegmentLinearRegression* segment)
{
	for (int i = 0; i < segment->getSize(); i++)
	{
		QVector2D currentPoint = segment->getPoint(i);

		//pointers?
		m_outputX[i] = currentPoint.x();
		m_outputY[i] = currentPoint.y();
	}
}