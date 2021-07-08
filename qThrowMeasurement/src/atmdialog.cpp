#include "atmdialog.h"
#include "ui_atmdialog.h"

#include "qThrowMeasurement.h"

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

ATMDialog::ATMDialog(ccMainAppInterface* app) :
    QDialog(app ? app->getMainWindow() : nullptr),
    Ui::ATMDialog(),
	app(app)
{
    setupUi(this);

	//connect(step, static_cast<void (QDoubleSpinBox::*)(double)> (&QDoubleSpinBox::valueChanged), this, &ATMDialog::onStepChanged);
    
    //connect(computeMain, &QPushButton::released, this, ATMDialog::computeSegmentation);
    //connect(displayProfilesBtn, &QPushButton::released, this, ATMDialog::displayProfilesDlg);
    //connect(genFromDBBtn, &QPushButton::released, this, ATMDialog::importGeneratrixFromDB);
    //connect(genFromTxtBtn, &QPushButton::released, this, ATMDialog::importGeneratrixFromTxt);
    //connect(saveAsTxtBtn, &QPushButton::released, this, ATMDialog::exportDataAsTxt);
    //connect(saveAsImgBtn, &QPushButton::released, this, ATMDialog::exportDataAsImg);

	QChartView* chartView;

    float* data = new float[10];
    int* id = new int[10];
	chartView = new QChartView(createLineChart(data, id, 10));
	baseLayout->addWidget(chartView, 1, 2);
	m_charts << chartView;


    //can get m_app as a parameter from qThrowMeasurement!!!!!! or use app??

    app->dbRootObject(); //nice
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

/*void ATMDialog::computeSegmentation()
{
    qThrowMeasurement::computeSegmentation();
}
*/

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
