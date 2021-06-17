#ifndef PROFILEPROCESSOR_H
#define PROFILEPROCESSOR_H

#include <QVector2D>
#include <QVector>

#include <ccPolyline.h>
#include <ccGenericPointCloud.h>
#include <segmentlinearregression.h>

class profileProcessor
{
public:
    profileProcessor(ccPolyline* profile);
    profileProcessor(SegmentLinearRegression* segment);

    ~profileProcessor();

    QVector<QVector2D> profileToXY();
    ccPolyline* segmentToProfile();

private:
    ccPolyline * m_inputProfile, * m_outputProfile;
    SegmentLinearRegression * m_inputSegment;
    float * m_inputX, * m_inputY, * m_outputX, * m_outputY, * m_outputZ; //no need of input Z bc 2D
    QVector<const CCVector3 *> m_inputProfilePts;

};

#endif // PROFILEPROCESSOR_H