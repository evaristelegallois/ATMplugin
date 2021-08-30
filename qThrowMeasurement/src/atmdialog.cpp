//##########################################################################
//#                                                                        #
//#                    CLOUDCOMPARE PLUGIN: ATMPlugin                      #
//#                                                                        #
//#  This program is free software; you can redistribute it and/or modify  #
//#  it under the terms of the GNU General Public License as published by  #
//#  the Free Software Foundation; version 2 of the License.               #
//#                                                                        #
//#  This program is distributed in the hope that it will be useful,       #
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
//#  GNU General Public License for more details.                          #
//#                                                                        #
//#                      COPYRIGHT: Gabriel Parel                          #
//#                                                                        #
//##########################################################################

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
#include "StatisticalTools.h"

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
    m_profiles(profiles),
	m_id(nullptr),
	m_y(nullptr),
	m_chart(nullptr)
{
    setupUi(this);
    
	//connecting signals to corresponding slots
    connect(computeMain, &QPushButton::released, this, &ATMDialog::computeSegmentation);
    connect(displayProfilesBtn, &QPushButton::released, this, &ATMDialog::displayProfilesDlg);
    connect(genFromDBBtn, &QPushButton::released, this, &ATMDialog::importGeneratrixFromDB);
    //connect(genFromTxtBtn, &QPushButton::released, this, ATMDialog::importGeneratrixFromTxt); //not ready
    connect(saveAsTxtBtn, &QPushButton::released, this, &ATMDialog::exportDataAsTxt);
    connect(saveAsImgBtn, &QPushButton::released, this, &ATMDialog::exportDataAsImg);

	//chartView creation and display
	m_chartView = new QChartView();
	baseLayout->addWidget(m_chartView, 1, 2);
}

ATMDialog::~ATMDialog()
{
	//release memory
	m_segmentList.clear();
	m_segmentList.shrink_to_fit();
	m_profiles.clear();
	m_profiles.shrink_to_fit();
	m_processors.clear();
	m_processors.shrink_to_fit();

	//delete m_generatrix;
	//delete m_chartView;
}

QChart* ATMDialog::createLineChart(float* data, int* id, int n) const
{
    QChart* chart = new QChart();
    chart->setTitle("Fault displacement relative to position on transect");

	//creating series + setting series properties
    QLineSeries* series = new QLineSeries(chart); //line
	QScatterSeries* points = new QScatterSeries(chart); //individual points
	points->setMarkerShape(QScatterSeries::MarkerShapeCircle);
	points->setColor(QColor(0, 255, 0));
	points->setBorderColor(QColor(0, 255, 0));
	points->setMarkerSize(5.0);

	//creating axes
	QCategoryAxis* axisX = new QCategoryAxis;
	//IF USING AN ARBITRARY AXIS:  QValueAxis* axisX = new QValueAxis;
    QValueAxis* axisY = new QValueAxis;
    axisX->setTitleText("Position on transect (y)");
    axisY->setTitleText("Fault displacement (m)"); 
	axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue); //comment is using an arbitrary axis

	//setting min and max values; might be improved
	float min = -10., max = 10.;
	for (int i = 0; i < n; i++)
	{
		if (data[i] < min) min = data[i] - 10.;
		if (data[i] > max) max = data[i] + 10.;
	}
	axisY->setMin(min);
	axisY->setMax(max);

	//filling out the series
    for (int i = 0; i < n; i++)
    {
        series->append(id[i], data[i]);
		points->append(id[i], data[i]);
		axisX->append("ID #" + QString::number(m_processors[i]->getProfileID()), i); //comment is using an arbitrary axis
    }

	chart->legend()->hide();

	//adding series to the chart
    chart->addSeries(series);
	chart->addSeries(points);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

	//attaching series to axes
    series->attachAxis(axisX);
    series->attachAxis(axisY);
	points->attachAxis(axisX);
	points->attachAxis(axisY);

    return chart;
}

void ATMDialog::computeSegmentation()
{
	//// segmentation parameters initialization 
	// break-point value p, jumps = 1 if discontinuity between segments is allowed, else 0
	// var or r² scoring function, dip angle alpha (if known), and generatrix (transect)
	s_p = this->pDoubleSpinBox->value();

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

	//allocate memory for inputs/outputs
	std::vector<QVector<QVector2D*>> inputs;
	inputs.reserve(m_profiles.size());
	std::vector<ccPolyline*> outputs;
	outputs.reserve(m_profiles.size());
	m_processors.reserve(m_profiles.size());
	m_segmentList.reserve(m_profiles.size());

	//// segmentation
	for (int i = 0; i < m_profiles.size(); i++)
	{
		m_processors.push_back(new profileProcessor(m_profiles[i], m_generatrix));
		inputs.push_back(m_processors[i]->profileToXY());
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

		outputs.push_back(m_processors[i]->segmentToProfile(m_segments)); 
		//m_transectPos.push_back(m_processors[i]->getTransectPos());

		m_app->addToDB(outputs[i]); //displays outputs
	}

	//compute displacement
	computeThrowMeasurement();
	m_app->redrawAll();

	//release memory
	m_processors.clear();
	m_processors.shrink_to_fit();
	m_segments.clear();
	m_segments.shrink_to_fit();
}

void ATMDialog::computeThrowMeasurement()
{
	m_y = new float[m_profiles.size()]; //Tr values
	m_id = new int[m_profiles.size()]; //profile IDs

	//IF USING AN ARBITRARY AXIS: fill out the x list with the y position instead
	int* x = new int[m_profiles.size()]; //used for display
	for (int i = 0; i < m_profiles.size(); i++)
	{
		x[i] = i;
		m_id[i] = m_processors[i]->getProfileID();
	}

	for (int i = 0; i < m_profiles.size(); i++)
	{
		std::vector<SegmentLinearRegression*> currentProfile;
		currentProfile.reserve(m_segmentList[i].size());
		currentProfile = m_segmentList[i]; //comment here if pre-made segments are needed
	
		//un-comment below if pre-made segments are needed

		/*
		float* xX, * yY;
		int idx = 0;
		int idxP = m_profiles[i]->size();
		xX = new float[m_profiles[i]->size()];
		yY = new float[m_profiles[i]->size()];
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
		
		//un-comment one of the profile below if pre-made segments are needed

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
		

		//FUCINO crop 1
		
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
		
		
		//FUCINO crop 2

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
		//storing segment coordinates as vectors of doubles for HAC computation
		for (int j = 0; j < currentProfile.size(); j++)
		{
			std::vector<double> xVal, yVal;
			//computing linear regression from 2-points segments makes no sense, so
			//we instead only use segments which size > 2
			if (currentProfile[j]->getSize() > 2)
			{
				xVal.push_back(static_cast<double> (currentProfile[j]->getPoint(0)->x()));
				yVal.push_back(static_cast<double> (currentProfile[j]->getPoint(0)->y()));

				for (int k = 1; k < currentProfile[j]->getSize() + s_jumps; k++)
				{
					//end of segment[i] = start of segment[i+1] if jumps = 0
					//to avoid overlapping, we don't add the same value twice
					if (currentProfile[j]->getPoint(k)->x() != currentProfile[j]->getPoint(k - 1)->x())
					{
						xVal.push_back(static_cast<double> (currentProfile[j]->getPoint(k)->x()));
						yVal.push_back(static_cast<double> (currentProfile[j]->getPoint(k)->y()));
					}
				}

				//needed for HAC computation
				LinearRegression* lr = new LinearRegression(xVal, yVal);
				listLR.push_back(lr);
			}
		}
		
		/// compute HAC; uses FracDense plug-in functions
		std::vector<std::vector<double>> matrix(matrixDistance(listLR));

		HAC_Average av(matrix);
		TreeNode* HAC_av = av.computeHAC(1); //root

		TreeNode* leftNode = HAC_av->getNodeLeft();
		TreeNode* rightNode = HAC_av->getNodeRight();

		int clusterNb = 0;
		Clusters* cluster;
		if (leftNode != nullptr && rightNode != nullptr) //if it's not a leaf
		{
			double max, threshold;
			if (listLR.size() < 3)
			{
				//if less than three segments, we go for two clusters
				max = std::max(leftNode->getGap(), rightNode->getGap());
				threshold = (HAC_av->getGap() + max) / 2.;
			}
			else {
				//if 3 clusters or more, we go for three clusters
				TreeNode* currentNode;
				if (leftNode->getGap() < rightNode->getGap()) currentNode = rightNode;
				else currentNode = leftNode;
				TreeNode* nextRNode = currentNode->getNodeRight();
				TreeNode* nextLNode = currentNode->getNodeLeft();
				max = std::max(nextRNode->getGap(), nextLNode->getGap());
				threshold = max + static_cast <float> (rand()) /
					(static_cast <float> (RAND_MAX / (currentNode->getGap() - max)));
			}

			cluster = new Clusters(HAC_av, threshold);
			clusterNb = cluster->get_number_clusters();
		}

		//if there's only 1 segment (leaf), or the clustering didn't work
		if (clusterNb == 0)
		{
			qDebug() << "The clustering didn't work. Try using a lower p parameter.";
			std::vector<int> listStart, listEnd;
			listStart.push_back(0);
			listEnd.push_back(0);
			m_startIdx.push_back(listStart);
			m_endIdx.push_back(listEnd);
			//m_sStartIdx.push_back(listStart);
			//m_sEndIdx.push_back(listEnd);

			m_y[i] = 0; //arbitrary value

			continue;
		}

		//if clustering OK
		std::vector<std::vector<TreeNode*>> tnClusters; //tree node clusters
		tnClusters.reserve(clusterNb);
		std::vector<std::vector<SegmentLinearRegression*>> sgClusters; //segment clusters
		sgClusters.reserve(clusterNb);

		float* averageSlopes = new float[clusterNb];
		std::vector<float> segmentSlopes; //stores slope for each segment
		segmentSlopes.reserve(m_segmentList[i].size());

		float maxSlope = 0.;
		int maxL = 0;
		//adding segments to their corresponding clusters
		for (int l = 0; l < clusterNb; l++)
		{
			tnClusters.push_back(cluster->getCluster(l));
			std::vector<SegmentLinearRegression*> sgList;
			float slope = 0.;
			for (int m = 0; m < tnClusters[l].size(); m++)
			{
				int index = tnClusters[l][m]->getValue();
				sgList.push_back(currentProfile[index]);
				slope += currentProfile[index]->getSlope();
			}

			//finding the highest-slope cluster
			averageSlopes[l] = slope / sgList.size(); //average slope for each cluster
			if (averageSlopes[l] > maxSlope)
			{
				maxSlope = averageSlopes[l];
				maxL = l;
			}

			sgClusters.push_back(sgList); 
			//sgClusters[l] = segments clustered, averageSlopes[l] = average slope of each cluster
		}

		//// getting the end and start indexes of the highest-slope cluster
		// for display purpose; the LineChart uses the entire point list and so
		// starting & ending points of each segment being parts of the 
		// highest-slope cluster are needed
		std::vector<int> listStart, listEnd;
		listStart.reserve(sgClusters[maxL].size());
		listEnd.reserve(sgClusters[maxL].size());

		for (int j = 0; j < sgClusters[maxL].size(); j++)
		{
			int startIndex = sgClusters[maxL][j]->getStartIndex();
			int endIndex = sgClusters[maxL][j]->getEndIndex();
			listStart.push_back(startIndex);
			listEnd.push_back(endIndex);
		}

		m_startIdx.push_back(listStart);
		m_endIdx.push_back(listEnd);

		// the following part might be useless; was used to get the max slope cluster
		// in the highest-slope cluster (useful if there different separated parts in 
		// the highest-slope cluster)
		
		// x = curvilinear abs, y = height (= initial z), a_m = average slope
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
						//if segment[i] in the maxCluster isn't followed by segment[i+1]
						if (sgClusters[maxL][l]->getEndIndex() != sgClusters[maxL][l + 1]->getStartIndex())
						{
							tempSgCluster.push_back(tempSgList);
							segmentSlopes.push_back(tempSlope / tempSgList.size());
						}
					}
				}
			}
		}

		// computing the average slope a_m from the other clusters
		float sumSlopes = 0.;
		for (int l = 0; l < clusterNb; l++)
		{
			if (l != maxL)
			{
				sumSlopes += averageSlopes[l];
			}
		}

		//same as above, might be useless now since a_m is computed
		//using the other non max clusters only
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
			a_m = sumSlopes / (clusterNb - 1);
			//gets throw measurement value (Tr)
			m_y[i] = static_cast<float> (computeTr(x1, x2, y1, y2, a_m));
		}
		else
		{
			y1 = sgClusters[maxL][0]->getStart().y();
			y2 = sgClusters[maxL][0]->getEnd().y();
			x1 = sgClusters[maxL][0]->getStart().x();
			x2 = sgClusters[maxL][0]->getEnd().x();
			a_m = sumSlopes / (clusterNb - 1);
			//gets throw measurement value (Tr)
			m_y[i] = static_cast<float> (computeTr(x1, x2, y1, y2, a_m));
		}
	}

	//display only; if only one selected profile, no graph is displayed (because drawing
	//a line needs two points); displays a message instead of a blank space
	if (m_profiles.size() < 2)
	{
		QLabel* label = new QLabel("At least two profiles need to be selected to display this graph.");
		baseLayout->addWidget(label, 1, 2, Qt::AlignCenter);
	}
	else
	{
		//compute throw along composite curvilinear abscissa
		//IF USING ARBITRARY AXIS: edit x values as needed
		m_chart = createLineChart(m_y, x, m_profiles.size());
		m_chartView->setChart(m_chart);
		m_chartView->update();
	}

}

float ATMDialog::computeTr(float x1, float x2, float y1, float y2, float a_m)
{
	//initialization
	float throwMeasurement = 0.;
	s_alpha = static_cast<float> (alphaDoubleSpinBox->value() * M_PI / 180); //dip angle, in radian

	if (s_alpha == 0) throwMeasurement = abs(x1 - x2);
	else
	{
		float beta = atan(a_m);
		float b1 = y1 - a_m * x1;
		float b2 = y2 - a_m * x2;
		float deltaB = abs(b1 - b2);

		//equation taken from Puliti et al, 2020
		throwMeasurement = (deltaB * (sin(s_alpha) * sin(beta + M_PI / 2))) / sin(s_alpha - beta);
	}

	return throwMeasurement;
}

void ATMDialog::displayProfilesDlg()
{
	ATMDisplayProfilesDlg* ATMDPDlg = new ATMDisplayProfilesDlg(m_segmentList, m_startIdx, m_endIdx);
}

void ATMDialog::importGeneratrixFromDB()
{
	QMainWindow* mainWindow = m_app->getMainWindow();
	if (!mainWindow)
		ccLog::Error("Main window not found!");

	//get the DB
	ccHObject* root = m_app->dbRootObject();
	ccHObject::Container polylines;
	if (root) root->filterChildren(polylines, true, CC_TYPES::POLY_LINE);

	if (!polylines.empty())
	{
		int index = qATMSelectEntitiesDlg::SelectEntity(polylines);
		//gets the generatrix from DB
		m_generatrix = static_cast<ccPolyline*>(polylines[index]);
		this->genName->setText(polylines[index]->getName());
	}
	else ccLog::Error("No polyline in DB!");
}

//INCOMPLETE FUNCTION
void ATMDialog::importGeneratrixFromTxt()
{
	QString name = QString("Transect");
	ccPointCloud* outputCloud = new ccPointCloud(name.toStdString().c_str());
	outputCloud->reserve(100000);

	std::vector<QVector3D*> coordList;

	//read txt file here
	// for (int i = 0; i < size file - 2; i++) coordList.push_back(QVector3D(file[i][0], file[i][1], file[i][2]));

	for (int i = 0; i < coordList.size()-2; i++)
	{
		outputCloud->addPoint(CCVector3(coordList[i]->x(), coordList[i]->y(), coordList[i]->z()));
	}
	outputCloud->shrinkToFit();

	ccPolyline* outputProfile = new ccPolyline(outputCloud);

	outputProfile->setForeground(true);
	outputProfile->set2DMode(false);
	outputProfile->reserve(outputCloud->size());
	outputProfile->addPointIndex(0, outputCloud->size());
	outputProfile->setWidth(5);
	//outputProfile->addChild(outputCloud);

	m_app->addToDB(outputProfile);
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

		//header
		stream << "nb of profiles " << m_segmentList.size() << "\n" << "p value " << s_p << "\n"
			<< "jump " << s_jumps << "\n" << "scoring function " << s_type << "\n";

		stream << "Tr" << "\t" << "profile #ID" << "\n";

		for (int i = 0; i < m_segmentList.size(); i++)
		{
			stream << m_y[i] << "\t" << m_id[i] << "\n";
		}

		//when done
		file.close();
		ccLog::Print(QString("[qATM] File '%1' successfully saved.").arg(path));
	}
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
		ccLog::Print(QString("[qATM] Image '%1' successfully saved.").arg(outputFilename));
	else ccLog::Print(QString("[qATM] Failed to save image '%1'!").arg(outputFilename));
}