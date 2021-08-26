#include "atmdialog.h"
#include "ui_atmdialog.h"

#include "qThrowMeasurement.h"
#include "qatmselectentitiesdlg.h"
#include "atmdisplayprofilesdlg.h"

//FracDense plug-in
#include "LinearRegression.h"
#include "MatrixDistance.h"
#include "TreeNode.h"
#include "HAC_Average.h"
#include "Clusters.h"

//qCC_plugins
#include "ccMainAppInterface.h"

//qCC_db
#include <ccPolyline.h>

//Qt
#include <QSettings>
#include <QMainWindow>
#include <QApplication>
#include <QDebug>
#include <QtCharts>

//system b 
#include<assert.h>
#include<math.h>

using namespace QtCharts;

//semi-persistent dialog values
static float s_p = 0.01;
static const char* s_type = "var";
static int s_size, s_jumps = 0;
static float s_alpha = 0;

ATMDialog::ATMDialog(ccMainAppInterface* app, std::vector<ccPolyline*> profiles) :
    QDialog(app ? app->getMainWindow() : nullptr),
    Ui::ATMDialog(),
	m_app(app),
    m_profiles(profiles)
{
    setupUi(this);

	//connect(step, static_cast<void (QDoubleSpinBox::*)(double)> (&QDoubleSpinBox::valueChanged), 
	//this, &ATMDialog::onStepChanged);
    
    connect(computeMain, &QPushButton::released, this, &ATMDialog::computeSegmentation);
    connect(displayProfilesBtn, &QPushButton::released, this, &ATMDialog::displayProfilesDlg);
    connect(genFromDBBtn, &QPushButton::released, this, &ATMDialog::importGeneratrixFromDB);
    //connect(genFromTxtBtn, &QPushButton::released, this, ATMDialog::importGeneratrixFromTxt);
    connect(saveAsTxtBtn, &QPushButton::released, this, &ATMDialog::exportDataAsTxt);
    connect(saveAsImgBtn, &QPushButton::released, this, &ATMDialog::exportDataAsImg);

	m_chartView = new QChartView();
	baseLayout->addWidget(m_chartView, 1, 2);
}

//LINE CHART FOR CUMULATIVE DISPLACEMENT
//add save as below
QChart* ATMDialog::createLineChart(float* data, int* id, int n) const
{
    QChart* chart = new QChart();
    chart->setTitle("Cumulative displacement relative to position on transect");
    QLineSeries* series = new QLineSeries(chart);
	QScatterSeries* points = new QScatterSeries(chart);
	points->setMarkerShape(QScatterSeries::MarkerShapeCircle);
	points->setColor(QColor(0, 255, 0));
	points->setBorderColor(QColor(0, 255, 0));
	points->setMarkerSize(5.0);
    //QString name("ID #");

    //QValueAxis* axisX = new QValueAxis;
	QCategoryAxis* axisX = new QCategoryAxis;
    QValueAxis* axisY = new QValueAxis;
    axisX->setTitleText("Position on transect (y)");
    axisY->setTitleText("Cumulative fault displacement (m)"); //not working??
	axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);

	float min = -10., max = 10.;
	for (int i = 0; i < n; i++)
	{
		if (data[i] < min) min = data[i] - 10.;
		if (data[i] > max) max = data[i] + 10.;
	}
	axisY->setMin(min);
	axisY->setMax(max);

    for (int i = 0; i < n; i++)
    {
        series->append(id[i], data[i]);
		//series->append(m_processors[i]->getProfileID(), data[i]);
		points->append(id[i], data[i]);
		qDebug() << "y, x" << data[i] << id[i];
        //series->setName(name + QString::number(id[i]));
		axisX->append("ID #" + QString::number(m_processors[i]->getProfileID()), i);
        //axisX->append("ID #" + QString::number(m_processors[i]->getProfileID()), i+1);
    }
    chart->legend()->hide();
    chart->addSeries(series);
	chart->addSeries(points);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);
	points->attachAxis(axisX);
	points->attachAxis(axisY);

    //chart->createDefaultAxes();
    return chart;
}

void ATMDialog::computeSegmentation()
{
	s_p = this->pDoubleSpinBox->value();
	//if (this->alphaCheckBox->isChecked()) s_alpha = static_cast<float> (alphaDoubleSpinBox->value() * 180/M_PI); //in radian
	if (this->jCheckBox->isChecked()) s_jumps = 1;
	else s_jumps = 0;
	if (this->scoreComboBox->currentText() == "Variance of residuals") s_type = "var";
	else s_type = "rsquare";

	if (m_generatrix == nullptr) 
	{
		QMessageBox msgBox;
		msgBox.setText("Please select a generatrix.");
		msgBox.exec();
		return;
	}
	qDebug() << "p" << s_p;

	std::vector<QVector<QVector2D*>> inputs;
	inputs.reserve(m_profiles.size());
	std::vector<ccPolyline*> outputs;
	outputs.reserve(m_profiles.size());
	m_processors.reserve(m_profiles.size());
	m_segmentList.reserve(m_profiles.size());

	for (int i = 0; i < m_profiles.size(); i++)
	{
		m_processors.push_back(new profileProcessor(m_profiles[i], m_generatrix));
		inputs.push_back(m_processors[i]->profileToXY());
		qDebug() << "profile XY OK";
	}

	for (int i = 0; i < m_profiles.size(); i++)
	{
		const int n = inputs[i].size();
		inputs[i].reserve(n);
		float* x = new float[n];
		float* y = new float[n];

		for (int j = 0; j < n; j++)
		{
			x[j] = inputs[i][j]->x();
			y[j] = inputs[i][j]->y();
		}

		//get linear regression parameters here
		dPPiecewiseLinearRegression* model = new dPPiecewiseLinearRegression(x, y, n, 
			s_p, s_jumps, s_type);
		m_segments = model->computeSegmentation();
		m_segmentList.push_back(m_segments);

		qDebug() << "list segments size" << m_segmentList.size();
		qDebug() << "segmentation model OK";

		outputs.push_back(m_processors[i]->segmentToProfile(m_segments)); 
		// ISSUE HERE invalid vector subscript

		m_transectPos.push_back(m_processors[i]->getTransectPos());
		qDebug() << "outputs OK";
		m_app->addToDB(outputs[i]);
	}

	//compute displacement
	computeThrowMeasurement();
	m_app->redrawAll();

	//release memory
	m_profiles.clear();
	m_processors.clear();
	//m_segments.clear();
	//m_segmentList.clear();
	inputs.clear();
	outputs.clear();
}

void ATMDialog::computeThrowMeasurement()
{
	m_y = new float[m_profiles.size()];
	m_id = new int[m_profiles.size()];
	int* x = new int[m_profiles.size()];
	for (int i = 0; i < m_profiles.size(); i++)
	{
		x[i] = i;
		m_id[i] = m_processors[i]->getProfileID();
	}

	for (int i = 0; i < m_profiles.size(); i++)
	{
		qDebug() << "i" << i;
		std::vector<SegmentLinearRegression*> currentProfile;
		currentProfile.reserve(m_segmentList[i].size());
		currentProfile = m_segmentList[i];

	
		/*float* xX, * yY;
		int idx = 0;
		int idxP = m_profiles[i]->size();
		xX = new float[m_profiles[i]->size()];
		yY = new float[m_profiles[i]->size()];
		qDebug() << "xX, yY size" << m_profiles[i]->size();
		for (int u = 0; u < m_segmentList[i].size(); u++)
		{
			for (int v = 0; v < m_segmentList[i][u]->getSize() - 1; v++)
			{
				xX[idx] = m_segmentList[i][u]->getPoint(v)->x();
				yY[idx] = m_segmentList[i][u]->getPoint(v)->y();
				idx++;
			}
		}
		*/

		//MAGNOLA

		/*
		//profile 2 test 4
		currentProfile.push_back(new SegmentLinearRegression(0, 451, xX, yY));
		currentProfile[0]->setSlope(0.44);
		currentProfile.push_back(new SegmentLinearRegression(451, 772, xX, yY));
		currentProfile[1]->setSlope(1.12);
		currentProfile.push_back(new SegmentLinearRegression(772, 1135, xX, yY));
		currentProfile[2]->setSlope(0.75);
		*/

		/*
		//profile 6 test 2
		currentProfile.push_back(new SegmentLinearRegression(0, 226, xX, yY));
		currentProfile[0]->setSlope(0.82);
		currentProfile.push_back(new SegmentLinearRegression(226, 255, xX, yY));
		currentProfile[1]->setSlope(0.33);
		currentProfile.push_back(new SegmentLinearRegression(255, 288, xX, yY));
		currentProfile[2]->setSlope(0.41);
		*/
		
		/*
		//profile 3 test 2
		currentProfile.push_back(new SegmentLinearRegression(0, 159, xX, yY));
		currentProfile[0]->setSlope(0.47);
		currentProfile.push_back(new SegmentLinearRegression(159, 189, xX, yY));
		currentProfile[1]->setSlope(1.69);
		currentProfile.push_back(new SegmentLinearRegression(189, 298, xX, yY));
		currentProfile[2]->setSlope(0.86);
		*/
		

		/*
		//profile 3 test 3
		currentProfile.push_back(new SegmentLinearRegression(0, 273, xX, yY));
		currentProfile[0]->setSlope(0.23);
		currentProfile.push_back(new SegmentLinearRegression(273, 401, xX, yY));
		currentProfile[1]->setSlope(0.4);
		currentProfile.push_back(new SegmentLinearRegression(401, 621, xX, yY));
		currentProfile[2]->setSlope(0.62);
		currentProfile.push_back(new SegmentLinearRegression(621, 692, xX, yY));
		currentProfile[3]->setSlope(0.76);
		currentProfile.push_back(new SegmentLinearRegression(692, 734, xX, yY));
		currentProfile[4]->setSlope(0.47);
		currentProfile.push_back(new SegmentLinearRegression(734, 868, xX, yY));
		currentProfile[5]->setSlope(0.66);
		currentProfile.push_back(new SegmentLinearRegression(868, 1052, xX, yY));
		currentProfile[6]->setSlope(0.43);
		currentProfile.push_back(new SegmentLinearRegression(1052, 1319, xX, yY));
		currentProfile[7]->setSlope(0.32);
		*/
		

		//FUCINO 1
		
		/*
		//profile 4
		currentProfile.push_back(new SegmentLinearRegression(0, 31, xX, yY));
		currentProfile[0]->setSlope(0.24);
		currentProfile.push_back(new SegmentLinearRegression(31, 66, xX, yY));
		currentProfile[1]->setSlope(0.33);
		currentProfile.push_back(new SegmentLinearRegression(66, 84, xX, yY));
		currentProfile[2]->setSlope(0.44);
		currentProfile.push_back(new SegmentLinearRegression(84, 157, xX, yY));
		currentProfile[3]->setSlope(0.59);
		currentProfile.push_back(new SegmentLinearRegression(157, 230, xX, yY));
		currentProfile[4]->setSlope(0.52);
		currentProfile.push_back(new SegmentLinearRegression(230, 298, xX, yY));
		currentProfile[5]->setSlope(0.56);
		*/
		
		
		//FUCINO 2

		/*
		//profile 2
		currentProfile.push_back(new SegmentLinearRegression(0, 58, xX, yY));
		currentProfile[0]->setSlope(0.39);
		currentProfile.push_back(new SegmentLinearRegression(58, 134, xX, yY));
		currentProfile[1]->setSlope(0.54);
		currentProfile.push_back(new SegmentLinearRegression(134, 517, xX, yY));
		currentProfile[2]->setSlope(0.5);
		*/


		std::vector<LinearRegression*> listLR;
		qDebug() << "segment list size" << currentProfile.size();
		
		/*
		for (int j = 0; j < currentProfile.size(); j++)
		{
			std::vector<double> xVal, yVal;

			qDebug() << "current segment size" << currentProfile[j]->getSize();
			qDebug() << "start, end" << currentProfile[j]->getStartIndex() << 
				currentProfile[j]->getEndIndex();

			for (int k = 0; k < currentProfile[j]->getSize()-1 + s_jumps; k++) //-1 if start = end
			{
				xVal.push_back(static_cast<double> (currentProfile[j]->getPoint(k)->x()));
				yVal.push_back(static_cast<double> (currentProfile[j]->getPoint(k)->y()));
			}

			LinearRegression* lr = new LinearRegression(xVal, yVal);
			listLR.push_back(lr);
		}
		*/

		std::vector<double> xVal, yVal;
		int idx = 0;
		for (; idx < currentProfile.size(); idx++)
		{
			//qDebug() << "current segment size" << currentProfile[idx]->getSize();
			//qDebug() << "start, end" << currentProfile[idx]->getStartIndex() <<
				//currentProfile[idx]->getEndIndex();

			if (currentProfile[idx]->getSize() == 2)
			{
				int size = 0;
				xVal.push_back(static_cast<double> (currentProfile[idx]->getPoint(0)->x()));
				yVal.push_back(static_cast<double> (currentProfile[idx]->getPoint(0)->y()));
				size++;
				idx++;

				while (size < 3 && idx < currentProfile.size())
				{
					for (int k = 0; k < currentProfile[idx]->getSize() - 1 + s_jumps; k++) //-1 if start = end
					{
						xVal.push_back(static_cast<double> (currentProfile[idx]->getPoint(k)->x()));
						yVal.push_back(static_cast<double> (currentProfile[idx]->getPoint(k)->y()));
						size++;
					}

					idx++;
				}

				//qDebug() << "xVal size" << xVal.size();
				LinearRegression* lr = new LinearRegression(xVal, yVal);
				listLR.push_back(lr);
			}

			else
			{
				for (int k = 0; k < currentProfile[idx]->getSize() - 1 + s_jumps; k++) //-1 if start = end
				{
					xVal.push_back(static_cast<double> (currentProfile[idx]->getPoint(k)->x()));
					yVal.push_back(static_cast<double> (currentProfile[idx]->getPoint(k)->y()));
				}

				//qDebug() << "xVal size" << xVal.size();
				LinearRegression* lr = new LinearRegression(xVal, yVal);
				listLR.push_back(lr);
			}
			//qDebug() << "idx" << idx;
		}
		xVal.clear();
		yVal.clear();
		
		
		// compute HAC
		std::vector<std::vector<double>> matrix(matrixDistance(listLR));

		qDebug() << "listLR size" << listLR.size();
		qDebug() << "matrix size" << matrix.size();

		for (int j = 0; j < matrix.size(); j++)
		{
			for (int k = 0; k < matrix.size(); k++) 		qDebug() << "matrix values" << matrix[j][k];
		}

		HAC_Average av(matrix);
		TreeNode* HAC_av = av.computeHAC(1); //root

		TreeNode* leftNode = HAC_av->getNodeLeft();
		TreeNode* rightNode = HAC_av->getNodeRight();

		//IF TWO CLUSTERS ARE NEEDED
		//double max = std::max(leftNode->getGap(), rightNode->getGap());
		//double threshold = (HAC_av->getGap() + max) / 2.;

		
		//IF THREE CLUSTERS ARE NEEDED
		TreeNode* currentNode;
		if (leftNode->getGap() < rightNode->getGap()) currentNode = rightNode;
		else currentNode = leftNode;
		TreeNode* nextRNode = currentNode->getNodeRight();
		TreeNode* nextLNode = currentNode->getNodeLeft();
		double max = std::max(nextRNode->getGap(), nextLNode->getGap());
		double threshold = max + static_cast <float> (rand()) / 
			(static_cast <float> (RAND_MAX / (currentNode->getGap() - max)));

		//qDebug() << "threshold" << threshold;

		//if (leftNode == nullptr && rightNode == nullptr) //it's a leaf
		Clusters* cluster = new Clusters(HAC_av, threshold);

		qDebug() << "clustering ok";

		int clusterNb = cluster->get_number_clusters();
		if (clusterNb == 0)
		{
			qDebug() << "The clustering didn't work. Try using a lower p parameter.";
			std::vector<int> listStart, listEnd;
			listStart.push_back(0);
			listEnd.push_back(0);
			m_startIdx.push_back(listStart);
			m_endIdx.push_back(listEnd);

			continue;
			//abort the process here?
		}

		std::vector<std::vector<TreeNode*>> tnClusters;
		tnClusters.reserve(clusterNb);
		std::vector<std::vector<SegmentLinearRegression*>> sgClusters;
		sgClusters.reserve(clusterNb);

		float* averageSlopes = new float[clusterNb];
		std::vector<float> segmentSlopes;
		segmentSlopes.reserve(m_segmentList[i].size());

		qDebug() << "cluster size alloc ok";

		float maxSlope = 0.;
		int maxL = 0;
		for (int l = 0; l < clusterNb; l++)
		{
			tnClusters.push_back(cluster->getCluster(l));
			qDebug() << "tnClusters size" << clusterNb;
			std::vector<SegmentLinearRegression*> sgList;

			float slope = 0.;
			for (int m = 0; m < tnClusters[l].size(); m++)
			{
				int index = tnClusters[l][m]->getValue();
				sgList.push_back(currentProfile[index]);
				slope += currentProfile[index]->getSlope();
				//segmentSlopes.push_back(currentProfile[index]->getSlope());
			}

			averageSlopes[l] = slope / sgList.size(); //average slope for each cluster
			if (averageSlopes[l] > maxSlope)
			{
				maxSlope = averageSlopes[l];
				maxL = l;
			}

			qDebug() << "cluster nb " << l << "average slope" << averageSlopes[l];
			qDebug() << "sgList size" << sgList.size();
			sgClusters.push_back(sgList);
		}

		std::vector<int> listStart, listEnd;
		listStart.reserve(sgClusters[maxL].size());
		listEnd.reserve(sgClusters[maxL].size());

		for (int j = 0; j < sgClusters[maxL].size(); j++)
		{
			//qDebug() << "sgList maxL size" << sgClusters[maxL].size();
			int startIndex = sgClusters[maxL][j]->getStartIndex();
			int endIndex = sgClusters[maxL][j]->getEndIndex();
			listStart.push_back(startIndex);
			listEnd.push_back(endIndex);
			qDebug() << "start Idx" << startIndex;
		}

		m_startIdx.push_back(listStart);
		m_endIdx.push_back(listEnd);
		//qDebug() << "point list size" << m_startIdx[i].size();
		qDebug() << "start idx list size" << listStart.size();

		std::vector<int> listSStart, listSEnd;
		listSStart.reserve(currentProfile.size());
		listSEnd.reserve(currentProfile.size());

		for (int j = 0; j < currentProfile.size(); j++)
		{
			//qDebug() << "sgList maxL size" << sgClusters[maxL].size();
			int startIndex = currentProfile[j]->getStartIndex();
			int endIndex = currentProfile[j]->getEndIndex();
			listSStart.push_back(startIndex);
			listSEnd.push_back(endIndex);
		}

		m_sStartIdx.push_back(listSStart);
		m_sEndIdx.push_back(listSEnd);

		// x = curvilinear abs, y = height (= z), a_m = average slope
		float x1 = 0., x2 = 0., y1 = 0., y2 = 0, a_m = 0.; 
		std::vector<std::vector<SegmentLinearRegression*>> tempSgCluster;
		if (listStart.size() != 1)
		{
			for (int k = 0; k < listStart.size() - 1; k++)
			{
				if (listEnd[k] != listStart[k + 1])
				{
					float tempSlope = 0.;
					std::vector<SegmentLinearRegression*> tempSgList;
					tempSgList.reserve(listStart.size() - 1);

					for (int l = 0; l < sgClusters[maxL].size() - 1; l++)
					{
						tempSgList.push_back(sgClusters[maxL][l]);
						tempSlope += sgClusters[maxL][l]->getSlope();
						if (sgClusters[maxL][l]->getEndIndex() != sgClusters[maxL][l + 1]->getStartIndex())
						{
							tempSgCluster.push_back(tempSgList);
							segmentSlopes.push_back(tempSlope / tempSgList.size());
						}
					}
					qDebug() << "temp list size" << tempSgList.size();
				}
			}
		}

		qDebug() << "segment slopes list size" << segmentSlopes.size();
		qDebug() << "temp cluster list size" << tempSgCluster.size();
		if (segmentSlopes.size() != 0)
		{
			float maxSl = 0.;
			for (int j = 0; j < segmentSlopes.size(); j++)
			{
				if (segmentSlopes[j] > maxSl)
				{
					maxSl = segmentSlopes[j];
					y1 = tempSgCluster[j][0]->getStart().y();
					y2 = tempSgCluster[j][tempSgCluster[j].size()-1]->getEnd().y();
					x1 = tempSgCluster[j][0]->getStart().x();
					x2 = tempSgCluster[j][tempSgCluster[j].size() - 1]->getEnd().x();
				}
			}
			a_m = maxSl;
			//gets throw measurement value (Tr)
			m_y[i] = static_cast<float> (computeTr(x1, x2, y1, y2, a_m));
		}
		else
		{
			y1 = sgClusters[maxL][0]->getStart().y();
			y2 = sgClusters[maxL][0]->getEnd().y();
			x1 = sgClusters[maxL][0]->getStart().x();
			x2 = sgClusters[maxL][0]->getEnd().x();
			a_m = maxSlope;
			//gets throw measurement value (Tr)
			m_y[i] = static_cast<float> (computeTr(x1, x2, y1, y2, a_m));
		}
	}
	//compute cumulative throw along somewhat curvilinear abscissa
	m_chart = createLineChart(m_y, x, m_profiles.size());
	m_chartView->setChart(m_chart);
	m_chartView->update();
}

float ATMDialog::computeTr(float x1, float x2, float y1, float y2, float a_m)
{
	float throwMeasurement = 0.;
	s_alpha = M_PI / 3; //temp, gonna be given by user; needs to be in rad
	if (s_alpha == 0) throwMeasurement = abs(x1 - x2);
	else
	{
		float beta = atan(a_m);
		float b1 = y1 - a_m * x1;
		float b2 = y2 - a_m * x2;
		float deltaB = abs(b1 - b2);
		qDebug() << "deltaB" << deltaB;
		//equation taken from Puliti et al, 2020
		throwMeasurement = (deltaB * (sin(s_alpha) * sin(beta + M_PI / 2))) / sin(s_alpha - beta);
	}

	qDebug() << "Tr" << throwMeasurement;
	return throwMeasurement;
}

void ATMDialog::exportDataAsTxt()
{
	//open file saving dialog
	QString path = QString("displacementData_coordinates");
	QString outputFilename = QFileDialog::getSaveFileName(this, "Select destination",
		path, tr("Text files(*.txt)"));

	if (outputFilename.isEmpty())
		return;

	//coordinates
	QFile file(outputFilename);
	qDebug() << outputFilename;
	if (file.open(QIODevice::ReadWrite)) {
		QTextStream stream(&file);
		stream << "Tr" << "\t" << "profile #ID" << "\n";

		for (int i = 0; i < m_segmentList.size(); i++)
		{
			stream << m_y[i] << "\t" << m_id[i] << "\n";
		}

		//when done
		file.close();
	}
}

void ATMDialog::importGeneratrixFromDB()
{
	QMainWindow* mainWindow = m_app->getMainWindow();
	if (!mainWindow)
		ccLog::Error("Main window not found!");

	ccHObject* root = m_app->dbRootObject();
	ccHObject::Container polylines;
	if (root) root->filterChildren(polylines, true, CC_TYPES::POLY_LINE);

	if (!polylines.empty())
	{
		int index = qATMSelectEntitiesDlg::SelectEntity(polylines);
		m_generatrix = static_cast<ccPolyline*>(polylines[index]);
		this->genName->setText(polylines[index]->getName());
	}
	else
	{
		ccLog::Error("No polyline in DB!");
	}

	//deal with cancellation
}

void ATMDialog::displayProfilesDlg()
{
	ATMDisplayProfilesDlg* ATMDPDlg = new ATMDisplayProfilesDlg(m_segmentList, m_startIdx, 
		m_endIdx, m_sStartIdx, m_sEndIdx, m_transectPos);
}


void ATMDialog::exportDataAsImg()
{
    //open file saving dialog
    QString outputFilename = QFileDialog::getSaveFileName(this, "Select destination", "displacementData.png", tr("Images(*.png)"));

    if (outputFilename.isEmpty())
        return;

	QImage* img = new QImage(m_chartView->size().width(), m_chartView->size().height(), QImage::Format_ARGB32_Premultiplied);
	QPainter p(img);
	m_chartView->render(&p);
	p.end();
	img->save(outputFilename);
 
	if (img->save(outputFilename)) 
		m_app->dispToConsole(QString("[qATM] Image '%1' successfully saved.").arg(outputFilename),
			ccMainAppInterface::STD_CONSOLE_MESSAGE);
    else m_app->dispToConsole(QString("[qATM] Failed to save image '%1'!").arg(outputFilename), 
		ccMainAppInterface::WRN_CONSOLE_MESSAGE);

}