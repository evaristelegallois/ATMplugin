#include "atmdialog.h"
#include "ui_atmdialog.h"

#include "qThrowMeasurement.h"
#include "qatmselectentitiesdlg.h"
#include "atmdisplayprofilesdlg.h"

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

	QChartView* chartView;

    float* data = new float[10];
    int* id = new int[10];
	chartView = new QChartView(createLineChart(data, id, 10));
	baseLayout->addWidget(chartView, 1, 2);
	//m_charts << chartView;


    //can get m_app as a parameter from qThrowMeasurement!!!!!! or use app??

    //m_app->dbRootObject(); //nice
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
        series->append(i, i/*data[i]*/);
        //series->setName(name + QString::number(id[i]));
        axisX->append("ID #" + QString::number(/*id[i]*/i), i);
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
	//compute cumulative displacement
	m_app->redrawAll();

	//release memory
	m_profiles.clear();
	m_processors.clear();
	m_segments.clear();
	m_segmentList.clear();
	inputs.clear();
	outputs.clear();
}

void ATMDialog::computeThrowMeasurement()
{
	for (int i = 0; i < m_profiles.size(); i++)
	{
		std::vector<SegmentLinearRegression*> currentProfile;
		currentProfile.reserve(m_segmentList[i].size());
		currentProfile = m_segmentList[i];
	}
	//gets throw value

	//compute cumulative throw along somewhat curvilinear abscissa
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
	ATMDisplayProfilesDlg* ATMDPDlg = new ATMDisplayProfilesDlg(m_segmentList);
	ATMDPDlg->displayProfile();
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
