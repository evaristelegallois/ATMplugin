#include "atmdisplayprofilesdlg.h"
#include "ui_atmdisplayprofilesdlg.h"

#include "qatmselectentitiesdlg.h"

#include <QtCharts>

using namespace QtCharts;

ATMDisplayProfilesDlg::ATMDisplayProfilesDlg(QWidget *parent) :
    QDialog(parent),
    Ui::ATMDisplayProfilesDlg()
{
    setupUi(this);

	QChart* chart = new QChart;
	//float* data = new float[10];
	m_chartView = new QChartView(chart);
	baseLayout->addWidget(m_chartView, 1, 2);
	//m_charts << m_chartView;

}

//LINE CHART FOR SEGMENTATION RESULTS
QChart* ATMDisplayProfilesDlg::createLineChart(float* x, float* y, int n) const
{
	QChart* chart = new QChart();
	chart->setTitle("Scarp height (in m) relative to curvilinear abscissa");
	QLineSeries* series = new QLineSeries(chart);
	//QString name("ID #");

	QValueAxis* axisX = new QValueAxis;
	QValueAxis* axisY = new QValueAxis;
	axisX->setTitleText("Curvilinear abscissa");
	axisY->setTitleText("Height (m)"); //not working??
	for (int i = 0; i < n; i++)
	{
		series->append(i, i/*data[i]*/);
		//series->setName(name + QString::number(id[i]));
		//axisX->append(QString::number(/*id[i]*/i));
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

void ATMDisplayProfilesDlg::displayProfile(std::vector<SegmentLinearRegression*> entities,
	int selectedIndex/*=0*/,
	QWidget* parent/*=0*/)
{
	ATMDisplayProfilesDlg ATMDPDlg(parent);

	QStringList items;
	for (size_t i = 0; i < entities.size(); ++i)
	{
		//add one line per entity
		items << QString("Profile #%1 (ID=%2)").arg(i).arg(i/*entities[i]->getUniqueID() */ ); //to do
	}
	ATMDPDlg.setItems(items, selectedIndex);

	if (!ATMDPDlg.exec())
	{
		//cancelled by the user
	}

	int currentIdx = ATMDPDlg.getSelectedIndex();
	int size = entities[currentIdx]->getSize();
	float* x = new float[size];
	float* y = new float[size];
	for (int i = 0; i < size; i++)
	{
		x[i] = entities[currentIdx]->getPoint(i)->x();
		y[i] = entities[currentIdx]->getPoint(i)->y();
		qDebug() << "x" << x[i];
	}

	QChart* chart = createLineChart(x,y,size);
	m_chartView = new QChartView(chart);
	baseLayout->addWidget(m_chartView, 1, 2);
	m_charts << m_chartView;

	m_chartView->update();
}

int ATMDisplayProfilesDlg::getSelectedIndex() const
{
	//get selected items
	QList<QListWidgetItem*> list = profileList->selectedItems();
	return list.empty() ? -1 : profileList->row(list.front());
}

void ATMDisplayProfilesDlg::setItems(const QStringList& items, int defaultSelectedIndex)
{
	for (int i = 0; i < items.size(); ++i)
	{
		//add one line per entity in the combo-box
		profileList->insertItem(static_cast<int>(i), new QListWidgetItem(items[i]));
	}

	//default selection
	if (defaultSelectedIndex >= 0 && defaultSelectedIndex < items.size())
	{
		profileList->setItemSelected(profileList->item(defaultSelectedIndex), true);
	}
}

/*
ATMDisplayProfilesDlg::~ATMDisplayProfilesDlg()
{
    delete ui;
}
*/
