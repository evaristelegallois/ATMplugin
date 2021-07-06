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
		QVector2D intercept;

		//gets generatrix x position on profiles
		for (int j = 0; j < m_inputGeneratrix->size() - 1; j++)
		{
			QVector2D genStart = QVector2D(m_inputGeneratrix->getPoint(j)->x, m_inputGeneratrix->getPoint(j)->y);
			QVector2D genEnd = QVector2D(m_inputGeneratrix->getPoint(j+1)->x, m_inputGeneratrix->getPoint(j+1)->y);
			intercept = getIntersection(genStart, genEnd, m_inputSegment->getStart(), m_inputSegment->getEnd());

			qDebug() << "genstart" << genStart << "genend" << genEnd;
			qDebug() << "start" << m_inputSegment->getStart() << "end" << m_inputSegment->getEnd();
			//qDebug() << "intercept" << intercept.x() << intercept.y();
			if (intercept != QVector2D(0, 0))
			{
				//m_genPtIdx = i;
				break;
			}
			//qDebug() << "print gen idx" << m_genPtIdx;
			//if (m_genPtIdx != 0) break;


		}

		for (int i = start; i < end; i++)
		{
			m_outputCloud->addPoint(CCVector3(m_inputProfilePts[i]->x,
				m_inputProfilePts[i]->y, m_inputProfilePts[i]->z));
			if (intercept.x() + 0.01*intercept.x() < m_inputProfilePts[i]->x || intercept.x() - 0.01 * intercept.x() < m_inputProfilePts[i]->x)
			{
				m_genPtIdx = i;
				m_outputCloud->getScalarField(sfIdx)->addElement(359); //assigns one predefined color to generatrix position
			}
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

//gets intersection between two segments, caracterized by two points each (start & end)
QVector2D profileProcessor::getIntersection(QVector2D p1, QVector2D p2, QVector2D q1, QVector2D q2)
{
	QVector2D s1, s2, i;
	float s, t;
	s1 = p2 - p1;
	s2 = q2 - q1;

	/*s = (-s1.y() * (p1.x() - q1.x()) + s1.x() * (p1.y() - q1.y())) / (-s2.x() * s1.y() + s1.x() * s2.y());
	t = (s2.x() * (p1.y() - q1.y()) - s2.y() * (p1.x() - q1.x())) / (-s2.x() * s1.y() + s1.x() * s2.y());

	//qDebug() << "s" << s << "t" << t;
	//qDebug() << "s1" << s1.x() << "s2" << s2.x();

	if (s >= 0 && s <= 1 && t >= 0 && t <= 1) i = QVector2D(p1.x() + (t * s1.x()), p1.y() + (t * s1.y()));
	else i = QVector2D(0, 0); // No collision

	return i; 
	*/

	
	
	/*//dx = x2 - x1; 
	//dy = y2 - y1;

	//m1 = dy / dx;
	float m1 = s1.y() / s1.x(); //slope
	// y = mx + c
	// intercept c = y - mx
	float c1 = p1.y() - m1 * p1.x(); // which is same as y2 - slope * x2


	//dx = x2 - x1;
	//dy = y2 - y1;

	float m2 = s2.y() / s2.x();
	float c2 = q2.y() - m2 * q2.x(); // which is same as y2 - slope * x2

	if ((m1 - m2) == 0)
		qDebug() << "No Intersection between the lines";
	else

	{
		i = QVector2D((c2 - c1) / (m1 - m2), m1 * (c2 - c1) / (m1 - m2) + c1);
	}

	qDebug() << "m1" << m1 << "m2" << m2;
	qDebug() << "i" << i;

	return i;*/

	/*
	int get_line_intersection(float p0_x, float p0_y, float p1_x, float p1_y, 
    float p2_x, float p2_y, float p3_x, float p3_y, float *i_x, float *i_y)
{
    float s02_x, s02_y, s10_x, s10_y, s32_x, s32_y, s_numer, t_numer, denom, t;
    s10_x = p1_x - p0_x;
    s10_y = p1_y - p0_y;
    s32_x = p3_x - p2_x;
    s32_y = p3_y - p2_y;

    denom = s10_x * s32_y - s32_x * s10_y;
    if (denom == 0)
        return 0; // Collinear
    bool denomPositive = denom > 0;

    s02_x = p0_x - p2_x;
    s02_y = p0_y - p2_y;
    s_numer = s10_x * s02_y - s10_y * s02_x;
    if ((s_numer < 0) == denomPositive)
        return 0; // No collision

    t_numer = s32_x * s02_y - s32_y * s02_x;
    if ((t_numer < 0) == denomPositive)
        return 0; // No collision

    if (((s_numer > denom) == denomPositive) || ((t_numer > denom) == denomPositive))
        return 0; // No collision
    // Collision detected
    t = t_numer / denom;
    if (i_x != NULL)
        *i_x = p0_x + (t * s10_x);
    if (i_y != NULL)
        *i_y = p0_y + (t * s10_y);

    return 1;
}
	*/
	


}


char profileProcessor::getProfileID()
{
	return m_profileID;
}

