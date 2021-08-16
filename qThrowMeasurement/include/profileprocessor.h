#ifndef PROFILEPROCESSOR_H
#define PROFILEPROCESSOR_H

#include <QVector2D>
#include <QVector>

#include <ccPolyline.h>
#include <ccGenericPointCloud.h>
#include <segmentlinearregression.h>

#include "ccMainAppInterface.h"


class profileProcessor
{
public:
    profileProcessor(ccPolyline* profile, ccPolyline* generatrix);
    profileProcessor(SegmentLinearRegression* segment);

    ~profileProcessor();

    QVector<QVector2D*> profileToXY();
    ccPolyline* segmentToProfile(std::vector<SegmentLinearRegression*> segments);
    QVector2D getIntersection(QVector2D p1, QVector2D p2, QVector2D q1, QVector2D q2);
    char getProfileID();
    int getTransectPos();

    void displayProfile(ccMainAppInterface* app);

private:
    ccPolyline * m_inputProfile, * m_outputProfile, * m_inputGeneratrix;
    ccPointCloud* m_outputCloud;
    SegmentLinearRegression * m_inputSegment;
    float * m_inputX, * m_inputY, * m_outputX, * m_outputY, * m_outputZ; //no need of input Z bc 2D
    QVector<const CCVector3 *> m_inputProfilePts;
    int m_profileID, m_genPtIdx;

};

#endif // PROFILEPROCESSOR_H
