#ifndef PROFILEPROCESSOR_H
#define PROFILEPROCESSOR_H

#include <QVector2D>

#include <ccPolyline.h>
#include <ccGenericPointCloud.h>
#include <segmentlinearregression.h>

class profileProcessor
{
public:
    profileProcessor(ccPolyline* profile);
    profileProcessor(SegmentLinearRegression* segment);

    ~profileProcessor();

    QVector2D profileToXY(ccPolyline* profile);
    ccPolyline segmentToProfile(SegmentLinearRegression* segment);

private:
    ccPolyline * m_inputProfile, * m_outputProfile;
    SegmentLinearRegression * m_inputSegment, * m_outputSegment;
    float * m_inputX, * m_inputY, * m_outputX, * m_outputY;

};

#endif // PROFILEPROCESSOR_H
