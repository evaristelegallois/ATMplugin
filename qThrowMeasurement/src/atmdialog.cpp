#include "atmdialog.h"
#include "ui_atmdialog.h"

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

//system
#include <assert.h>

using namespace QtCharts;


ATMDialog::ATMDialog(ccMainAppInterface* app) :
    QDialog(app ? app->getMainWindow() : nullptr),
    Ui::ATMDialog(),
	app(app)
{
    setupUi(this);

	//connect(step, static_cast<void (QDoubleSpinBox::*)(double)> (&QDoubleSpinBox::valueChanged), this, &ATMDialog::onStepChanged);

	QChartView* chartView;

    float* data = new float[10];
    int* id = new int[10];
	chartView = new QChartView(createLineChart(data, id, 10));
	baseLayout->addWidget(chartView, 1, 2);
	m_charts << chartView;

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


//LINE CHART FOR SEGMENTATION DISPLAY
//add save as below + save as data set maybe 

/*
ATMDialog::~ATMDialog()
{
    delete ui;
}
*/
