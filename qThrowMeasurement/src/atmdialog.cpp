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

	chartView = new QChartView(createLineChart());
	baseLayout->addWidget(chartView, 1, 2);
	m_charts << chartView;

}

QChart* ATMDialog::createLineChart() const
{
    QChart* chart = new QChart();
    chart->setTitle("Line chart");

    QString name("Series ");
    int nameIndex = 0;
    /*for (const DataList& list : m_dataTable) {
        QLineSeries* series = new QLineSeries(chart);
        for (const Data& data : list)
            series->append(data.first);
        series->setName(name + QString::number(nameIndex));
        nameIndex++;
        chart->addSeries(series);
    }*/
    chart->createDefaultAxes();

    return chart;
}


/*
ATMDialog::~ATMDialog()
{
    delete ui;
}
*/
