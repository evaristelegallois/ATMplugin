#include "atmdisplayprofilesdlg.h"
#include "ui_atmdisplayprofilesdlg.h"

#include "qatmselectentitiesdlg.h"

#include <QtCharts>

using namespace QtCharts;

ATMDisplayProfilesDlg::ATMDisplayProfilesDlg(std::vector<std::vector<SegmentLinearRegression*>> entities, QWidget *parent) :
    QDialog(parent),
    Ui::ATMDisplayProfilesDlg(),
	m_entities(entities)
{
    setupUi(this);

	connect(saveAsTxtBtn, &QPushButton::released, this, &ATMDisplayProfilesDlg::exportDataAsTxt);
	connect(saveAllAsTxtBtn, &QPushButton::released, this, &ATMDisplayProfilesDlg::exportAllDataAsTxt);
	//connect(saveAsImgBtn, &QPushButton::released, this, ATMDialog::exportDataAsImg);


	float* data = new float[10];
	m_chart = createLineChart(data, data, 10);
	m_chartView = new QChartView(m_chart);
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

//LIST ISNT CLEARED SO NEED TO ADD UNIQUE ID AND P IN CASE PEOPLE WANT TO COMPARE BETWEEN SEGMENTATION RESULTS
void ATMDisplayProfilesDlg::displayProfile(int selectedIndex/*=0*/,
	QWidget* parent/*=0*/)
{
	ATMDisplayProfilesDlg ATMDPDlg(m_entities, parent);
	ATMDPDlg.setModal(false);

	QStringList items;
	for (size_t i = 0; i < m_entities.size(); ++i)
	{
		//add one line per entity
		items << QString("Profile #%1 (ID=%2)").arg(i).arg(i/*entities[i]->getUniqueID() */); //to do
	}
	ATMDPDlg.setItems(items, selectedIndex);

	for (size_t i = 0; i < m_entities.size(); ++i)
	{
		int currentIdx = ATMDPDlg.getSelectedIndex();
		int size = m_entities[i][currentIdx]->getSize();
		float* x = new float[size];
		float* y = new float[size];
		for (int j = 0; j < size; j++)
		{
			x[j] = m_entities[i][currentIdx]->getPoint(j)->x();
			y[j] = m_entities[i][currentIdx]->getPoint(j)->y();
		}

		m_chart->removeAllSeries();
		m_chart = createLineChart(x, y, size);
		m_chartView->setChart(m_chart);
		baseLayout->addWidget(m_chartView, 1, 2);
		//m_charts << m_chartView;
	}

	m_chartView->update();
	//m_chartView->repaint();

	if (!ATMDPDlg.exec())
	{
		//cancelled by the user
	}
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

void ATMDisplayProfilesDlg::exportDataAsTxt()
{
	int idx = getSelectedIndex();

	//open file saving dialog
	QString path = QString("profile#%1_coordinates").arg(idx);
	QString outputFilename = QFileDialog::getSaveFileName(this, "Select destination", path, tr("Text files(*.txt)"));

	if (outputFilename.isEmpty())
		return;

	//coordinates
	QFile file(outputFilename);
	qDebug() << outputFilename;
	if (file.open(QIODevice::ReadWrite)) {
		QTextStream stream(&file);
		stream << "curvilinear abscissa" << "\t" << "z" << "\n";

		for (int i = 0; i < m_entities.size(); i++)
		{
			for (int j = 0; j < m_entities[i].size(); j++)
			{
				for (int k = 0; k < m_entities[i][j]->getSize(); k++) stream << m_entities[i][j]->getPoint(j)->x()
					<< "\t" << m_entities[i][j]->getPoint(j)->y() << "\n";
			}

		}
		//when done
		file.close();
	}

	//segmentation related data
	QString path2 = QString("profile#%1_segmentationData").arg(idx);
	QString outputFilename2 = QFileDialog::getSaveFileName(this,
		"Select destination", path2, tr("Text files(*.txt)"));

	if (outputFilename2.isEmpty())
		return;

	QFile file2(outputFilename2);
	if (file2.open(QIODevice::ReadWrite)) {
		QTextStream stream(&file2);
		stream << "start" << "\t" << "end" << "\t" << "intercept"
			<< "\t" << "slope" << "\t" << "r2" << "\t" << "var" << "\n";

		for (int i = 0; i < m_entities.size(); i++)
		{
			for (int j = 0; j < m_entities[i].size(); j++)
			{
				stream << m_entities[i][j]->getStartIndex() << "\t"
					<< m_entities[i][j]->getEndIndex() << "\t" << m_entities[i][j]->getIntercept()
					<< "\t" << m_entities[i][j]->getSlope() << "\t"
					<< m_entities[i][j]->getRSquare() << "\t" << m_entities[i][j]->getVar() << "\n";
			}
		}
		//when done
		file2.close();
	}
}

void ATMDisplayProfilesDlg::exportAllDataAsTxt()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"));
	for (int l = 0; l < m_entities.size(); l++)
	{
		//open file saving dialog
		QString path = dir + QString("/profile#%1_coordinates.txt").arg(l);

		//coordinates
		QFile file(path);
		if (file.open(QIODevice::ReadWrite)) {
			QTextStream stream(&file);
			stream << "curvilinear abscissa" << "\t" << "z" << "\n";

			for (int i = 0; i < m_entities.size(); i++)
			{
				for (int j = 0; j < m_entities[i].size(); j++)
				{
					for (int k = 0; k < m_entities[i][j]->getSize(); k++) stream << m_entities[i][j]->getPoint(k)->x()
						<< "\t" << m_entities[i][j]->getPoint(k)->y() << "\n";
				}

			}
			//when done
			file.close();
		}

		//segmentation related data
		QString path2 = dir + QString("/profile#%1_segmentationData.txt").arg(l);

		QFile file2(path2);
		if (file2.open(QIODevice::ReadWrite)) {
			QTextStream stream(&file2);
			stream << "start" << "\t" << "end" << "\t" << "intercept" << "\t" << "slope" << "\t" << "r2" << "\t" << "var" << "\n";

			for (int i = 0; i < m_entities.size(); i++)
			{
				for (int j = 0; j < m_entities[i].size(); j++)
				{
					stream << m_entities[i][j]->getStartIndex() << "\t"
						<< m_entities[i][j]->getEndIndex() << "\t" << m_entities[i][j]->getIntercept()
						<< "\t" << m_entities[i][j]->getSlope() << "\t"
						<< m_entities[i][j]->getRSquare() << "\t" << m_entities[i][j]->getVar() << "\n";
				}
			}
			//when done
			file2.close();
		}
	}
}


/*
ATMDisplayProfilesDlg::~ATMDisplayProfilesDlg()
{
	delete ui;
}
*/
