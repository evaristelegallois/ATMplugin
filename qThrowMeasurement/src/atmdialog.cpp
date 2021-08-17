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
#include <assert.h>
#include<math.h>

using namespace QtCharts;

//semi-persistent dialog values
static float s_p = 1.00;
static const char* s_type = "var";
static int s_size, s_jumps = 0;

ATMDialog::ATMDialog(ccMainAppInterface* app, std::vector<ccPolyline*> profiles) :
    QDialog(app ? app->getMainWindow() : nullptr),
    Ui::ATMDialog(),
	m_app(app),
    m_profiles(profiles)
{
    setupUi(this);

	//connect(step, static_cast<void (QDoubleSpinBox::*)(double)> (&QDoubleSpinBox::valueChanged), this, &ATMDialog::onStepChanged);
    
    connect(computeMain, &QPushButton::released, this, &ATMDialog::computeSegmentation);
    connect(displayProfilesBtn, &QPushButton::released, this, &ATMDialog::displayProfilesDlg);
    connect(genFromDBBtn, &QPushButton::released, this, &ATMDialog::importGeneratrixFromDB);
    //connect(genFromTxtBtn, &QPushButton::released, this, ATMDialog::importGeneratrixFromTxt);
    connect(saveAsTxtBtn, &QPushButton::released, this, &ATMDialog::exportDataAsTxt);
    //connect(saveAsImgBtn, &QPushButton::released, this, ATMDialog::exportDataAsImg);

	m_chartView = new QChartView();
	baseLayout->addWidget(m_chartView, 1, 2);

	/*if (!this->exec())
	{
		//cancelled by the user
	}*/
}

//LINE CHART FOR CUMULATIVE DISPLACEMENT
//add save as below
QChart* ATMDialog::createLineChart(float* data, int* id, int n) const
{
    QChart* chart = new QChart();
    chart->setTitle("Cumulative displacement relative to position on transect");
    QLineSeries* series = new QLineSeries(chart);
    //QString name("ID #");

    QCategoryAxis* axisX = new QCategoryAxis;
    QValueAxis* axisY = new QValueAxis;
    axisX->setTitleText("Position on transect (y)");
    axisY->setTitleText("Cumulative fault displacement (m)"); //not working??
    for (int i = 0; i < n; i++)
    {
        series->append(id[i], data[i]);
		qDebug() << "y, x" << data[i] << id[i];
        //series->setName(name + QString::number(id[i]));
        //axisX->append("ID #" + QString::number(m_processors[i]->getProfileID()), i);
    }
    chart->legend()->hide();
    chart->addSeries(series);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    //chart->createDefaultAxes();
    return chart;
}


void ATMDialog::computeSegmentation()
{
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
		dPPiecewiseLinearRegression* model = new dPPiecewiseLinearRegression(x, y, n, s_p, s_jumps, s_type);
		m_segments = model->computeSegmentation();
		m_segmentList.push_back(m_segments);

		qDebug() << "list segments size" << m_segmentList.size();
		qDebug() << "segmentation model OK";

		outputs.push_back(m_processors[i]->segmentToProfile(m_segments)); // ISSUE HERE invalid vector subscript
		m_transectPos.push_back(m_processors[i]->getTransectPos());
		qDebug() << "outputs OK";
		m_app->addToDB(outputs[i]);

		/*
		if (outputs.empty())
			return;

		//we only export 'temporary' objects
		unsigned exportCount = outputs.size();

		if (!exportCount)
		{
			//nothing to do
			ccLog::Warning("[qThrowMeasurement] All segments are already in DB");
			return;
		}

		//ccHObject* destEntity = ccHObject::New("ATMPlugin", "0", "Segmentation results"); //default group -> ID = 0
		//New(const QString & pluginId, const QString & classId, const char* name = nullptr);
		//assert(destEntity);

		ccHObject toSave("Segmentation results");

		QMainWindow* mainWin = m_app->getMainWindow();

		//export entites
		for (auto& output : outputs)
		{

				//destEntity->addChild(output);
			toSave.addChild(output);
			//output->setDisplay_recursive(toSave->getDisplay());
				//output.isInDB = true;
				//output->setDisplay_recursive(destEntity->getDisplay());
				m_app->addToDB(output, false, false);

		}

		ccLog::Print(QString("[qThrowMeasurement] %1 segmentation(s) computed").arg(exportCount));
		*/
	}
	//compute displacement
	computeThrowMeasurement();
	//compute cumulative displacement
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
	//float df, alpha, tExp, tThr, sd1, sd2;
	float cumulSlope = 0.;
	float* y = new float[m_profiles.size()];
	int* x = new int[m_profiles.size()];
	for (int i = 0; i < m_profiles.size(); i++) x[i] = i;

	for (int i = 0; i < m_profiles.size(); i++)
	{
		std::vector<SegmentLinearRegression*> currentProfile;
		currentProfile.reserve(m_segmentList[i].size());
		//currentProfile = m_segmentList[i];

	
		float* xX, * yY;
		int idx = 0;
		int idxP = m_profiles[i]->size();
		xX = new float[m_profiles[i]->size()];
		yY = new float[m_profiles[i]->size()];
		//xX = new float[1513];
		//yY = new float[1513];
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
		
		
		//profile 3 test 2
		currentProfile.push_back(new SegmentLinearRegression(0, 159, xX, yY));
		currentProfile[0]->setSlope(0.47);
		currentProfile.push_back(new SegmentLinearRegression(159, 189, xX, yY));
		currentProfile[1]->setSlope(1.69);
		currentProfile.push_back(new SegmentLinearRegression(189, 298, xX, yY));
		currentProfile[2]->setSlope(0.86);
		

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
		for (int j = 0; j < currentProfile.size(); j++)
		{
			std::vector<double> xVal, yVal;
			qDebug() << "current segment size" << currentProfile[j]->getSize();
			qDebug() << "start, end" << currentProfile[j]->getStartIndex() << currentProfile[j]->getEndIndex();
			//for (int k = currentProfile[j]->getStartIndex(); k < currentProfile[j]->getEndIndex(); k++)
			for (int k = 0; k < currentProfile[j]->getSize()-1 + s_jumps; k++) //-1 if start = end
			{
				xVal.push_back(static_cast<double> (currentProfile[j]->getPoint(k)->x()));
				yVal.push_back(static_cast<double> (currentProfile[j]->getPoint(k)->y()));
			}

			//qDebug() << "nb" << nb;
			LinearRegression* lr = new LinearRegression(xVal, yVal);
			listLR.push_back(lr);
		}

		//for (int k = 0; k < xVal.size(); k++) qDebug() << "xVal, yVal" << xVal[k] << yVal[k];

		// compute HAC
		std::vector<std::vector<double>> matrix(matrixDistance(listLR));

		/*
		for (int a = 0; a < matrix.size(); a++)
		{
			for (int b = 0; b < matrix[a].size(); b++)
				qDebug() << "matrix values" << matrix[a][b];
		}
		*/

		qDebug() << "listLR size" << listLR.size();
		qDebug() << "matrix size" << matrix.size();

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
		double threshold = max + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (currentNode->getGap() - max)));

		//if (leftNode == nullptr && rightNode == nullptr) //it's a leaf
		Clusters* cluster = new Clusters(HAC_av, threshold);

		qDebug() << "clustering ok";

		int clusterNb = cluster->get_number_clusters();
		std::vector<std::vector<TreeNode*>> tnClusters;
		tnClusters.reserve(clusterNb);
		std::vector<std::vector<SegmentLinearRegression*>> sgClusters;
		sgClusters.reserve(clusterNb);

		float* slopes = new float[clusterNb];

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
			}

			slopes[l] = slope / sgList.size(); //average slope for each cluster
			if (slopes[l] > maxSlope)
			{
				maxSlope = slopes[l];
				maxL = l;
				qDebug() << "maxL" << maxL;
			}

			qDebug() << "cluster nb " << l << "average slope" << slopes[l];

			qDebug() << "sgList size" << sgList.size();
			sgClusters.push_back(sgList);
		}

		y[i] = static_cast<float> (maxSlope);
		qDebug() << "y" << y[i];
		qDebug() << "maxSlope" << maxSlope;

		std::vector<int> listStart, listEnd;
		listStart.reserve(sgClusters[maxL].size());
		listEnd.reserve(sgClusters[maxL].size());

		for (int i = 0; i < sgClusters[maxL].size(); i++)
		{
			//qDebug() << "sgList maxL size" << sgClusters[maxL].size();
			int startIndex = sgClusters[maxL][i]->getStartIndex();
			int endIndex = sgClusters[maxL][i]->getEndIndex();
			listStart.push_back(startIndex);
			listEnd.push_back(endIndex);
			qDebug() << "start Idx" << startIndex;
		}

		m_startIdx.push_back(listStart);
		m_endIdx.push_back(listEnd);
		qDebug() << "point list size" << m_startIdx[i].size();
		//full profile is covered in green

	}
	//gets throw value
	//compute cumulative throw along somewhat curvilinear abscissa

	m_chart = createLineChart(y, x, m_profiles.size());
	m_chartView->setChart(m_chart);
	m_chartView->update();
}

void ATMDialog::exportDataAsTxt()
{
	
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
}

void ATMDialog::displayProfilesDlg()
{
	ATMDisplayProfilesDlg* ATMDPDlg = new ATMDisplayProfilesDlg(m_segmentList, m_startIdx, m_endIdx, m_transectPos);
	//ATMDPDlg->displayProfile();
	//ATMDPDlg->displayChart();
}

/*
void ATMDialog::exportDataAsImg()
{

    QPixmap p( chartView->size() );
    chartView->render( &p );
    p.save("a.png", "PNG");

    //open file saving dialog
    QString outputFilename = QFileDialog::getSaveFileName(nullptr, "Select destination", destinationPathLineEdit->text(), saveFileFilter);

    if (outputFilename.isEmpty())
        return;

    destinationPathLineEdit->setText(outputFilename);

    //FacetsExportDlg fDlg(FacetsExportDlg::SHAPE_FILE_IO, m_app->getMainWindow()); //gets what's above

    //persistent settings (default export path)
    QSettings settings;
    settings.beginGroup("qFacets");
    QString facetsSavePath = settings.value("exportPath", ccFileUtils::defaultDocPath()).toString();
    fDlg.destinationPathLineEdit->setText(facetsSavePath + QString("/facets.shp"));

    if (!fDlg.exec())
        return;

    QString filename = fDlg.destinationPathLineEdit->text();

    //save current export path to persistent settings
    settings.setValue("exportPath", QFileInfo(filename).absolutePath());

    if (QFile(filename).exists())
    {
        //if the file already exists, ask for confirmation!
        if (QMessageBox::warning(m_app->getMainWindow(), "File already exists!", "File already exists! Are you sure you want to overwrite it?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
            return;
    }


            std::vector<GenericDBFField*> fields;
        fields.push_back(&facetIndex);
        fields.push_back(&facetBarycenter);
        fields.push_back(&facetNormal);
        fields.push_back(&facetRMS);
        fields.push_back(&horizExtension);
        fields.push_back(&vertExtension);
        fields.push_back(&surfaceExtension);
        fields.push_back(&facetSurface);
        fields.push_back(&facetDipDir);
        fields.push_back(&facetDip);
        fields.push_back(&familyIndex);
        fields.push_back(&subfamilyIndex);
        ShpFilter filter;
        filter.treatClosedPolylinesAsPolygons(true);
        ShpFilter::SaveParameters params;
        params.alwaysDisplaySaveDialog = false;
        if (filter.saveToFile(&toSave, fields, filename, params) == CC_FERR_NO_ERROR)
        {
            m_app->dispToConsole(QString("[qFacets] File '%1' successfully saved").arg(filename), ccMainAppInterface::STD_CONSOLE_MESSAGE);
        }
        else
        {
            m_app->dispToConsole(QString("[qFacets] Failed to save file '%1'!").arg(filename), ccMainAppInterface::WRN_CONSOLE_MESSAGE);
        }
}


*/

/*
ATMDialog::~ATMDialog()
{
    delete ui;
}
*/
