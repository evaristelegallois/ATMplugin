#include "atmdisplayprofilesdlg.h"
#include "ui_atmdisplayprofilesdlg.h"

#include "qatmselectentitiesdlg.h"

//qCC_plugins
#include "ccMainAppInterface.h"

//Qt
#include <QSettings>
#include <QMainWindow>
#include <QApplication>
#include <QDebug>
#include <QtCharts>

using namespace QtCharts;

ATMDisplayProfilesDlg::ATMDisplayProfilesDlg(std::vector<std::vector<SegmentLinearRegression*>> entities, 
	std::vector<std::vector<int>> startIdx, std::vector<std::vector<int>> endIdx, 
	std::vector<int> transectPos, QWidget *parent) :
    QDialog(parent),
    Ui::ATMDisplayProfilesDlg(),
	m_entities(entities),
	m_startIdx(startIdx),
	m_endIdx(endIdx),
	m_transectPos(transectPos)
{
    setupUi(this);

	connect(saveAsTxtBtn, &QPushButton::released, this, &ATMDisplayProfilesDlg::exportDataAsTxt);
	connect(saveAllAsTxtBtn, &QPushButton::released, this, &ATMDisplayProfilesDlg::exportAllDataAsTxt);
	connect(saveAsImgBtn, &QPushButton::released, this, &ATMDisplayProfilesDlg::exportDataAsImg);
	connect(saveAllAsImgBtn, &QPushButton::released, this, &ATMDisplayProfilesDlg::exportAllDataAsImg);
	connect(profileList, &QListWidget::currentRowChanged, this, 
		&ATMDisplayProfilesDlg::displayChart);

	this->setModal(false);

	QStringList items;
	for (size_t i = 0; i < m_entities.size(); ++i)
	{
		//add one line per entity
		items << QString("Profile #%1 (ID=%2)").arg(i).arg(m_entities[i][0]->getUniqueSharedID());
	}
	this->setItems(items, 0);

	m_chartView = new QChartView();
	baseLayout->addWidget(m_chartView, 1, 2);

	displayChart();

	if (!this->exec())
	{
		//cancelled by the user
	}
}

//LINE CHART FOR SEGMENTATION RESULTS
QChart* ATMDisplayProfilesDlg::createLineChart(float* x, float* y, int n) const
{
	QChart* chart = new QChart();
	chart->setTitle("Scarp height (in m) relative to curvilinear abscissa");
	QLineSeries* series = new QLineSeries(chart);
	//QString name("ID #");

	QScatterSeries* clusterPos = new QScatterSeries(chart);
	clusterPos->setMarkerShape(QScatterSeries::MarkerShapeCircle);
	clusterPos->setColor(QColor(0, 255, 0));
	clusterPos->setBorderColor(QColor(0, 255, 0));
	clusterPos->setMarkerSize(5.0);

	QScatterSeries* transectPos = new QScatterSeries(chart);
	transectPos->setMarkerShape(QScatterSeries::MarkerShapeCircle);
	transectPos->setColor(QColor(255,0,0));
	transectPos->setMarkerSize(10.0);

	QValueAxis* axisX = new QValueAxis;
	QValueAxis* axisY = new QValueAxis;
	axisX->setTitleText("Curvilinear abscissa");
	axisY->setTitleText("Height (m)");

	for (int i = 0; i < n; i++) series->append(x[i], y[i]);
	if (m_transectPos[getSelectedIndex()] == 0) transectPos->append(-1, -1);
	else transectPos->append(x[m_transectPos[getSelectedIndex()]], 
		y[m_transectPos[getSelectedIndex()]]); //generatrix position
	//qDebug() << "gen pos" << m_transectPos[getSelectedIndex()];
	for (int j = 0; j < m_startIdx[getSelectedIndex()].size(); j++)
	{
		for (int k = m_startIdx[getSelectedIndex()][j]; k <= m_endIdx[getSelectedIndex()][j]; k++)
			clusterPos->append(x[k], y[k]);
	}

	chart->legend()->hide();
	//chart->legend()->setColor(QColor(rand() % 255, rand() % 255, rand() % 255));
	chart->addSeries(series);
	chart->addSeries(clusterPos);
	chart->addSeries(transectPos);

	chart->addAxis(axisX, Qt::AlignBottom);
	chart->addAxis(axisY, Qt::AlignLeft);
	series->attachAxis(axisX);
	series->attachAxis(axisY);
	transectPos->attachAxis(axisX);
	transectPos->attachAxis(axisY);
	clusterPos->attachAxis(axisX);
	clusterPos->attachAxis(axisY);

	//chart->createDefaultAxes();
	return chart;
}

void ATMDisplayProfilesDlg::displayChart()
{
	displayProfile(getSelectedIndex());
	m_chartView->update();
}

//LIST ISNT CLEARED SO NEED TO ADD UNIQUE ID AND P IN CASE PEOPLE WANT TO COMPARE 
//BETWEEN SEGMENTATION RESULTS
void ATMDisplayProfilesDlg::displayProfile(int selectedIndex/*=0*/,
	QWidget* parent/*=0*/)
{
	for (size_t i = 0; i < m_entities.size(); ++i) //this for loop is useless as it is
	{
		int currentIdx = this->getSelectedIndex();
		if (currentIdx < 0) currentIdx = 0;

		int size = m_entities[currentIdx][m_entities[currentIdx].size() - 1]->getEndIndex() + 1;

		float* x = new float[size];
		float* y = new float[size];
		for (int j = 0; j < m_entities[currentIdx].size(); j++)
		{
			SegmentLinearRegression* currentSegment = m_entities[currentIdx][j];
			for (int k = 0; k < currentSegment->getSize(); k++)
			{
				x[currentSegment->getStartIndex() + k] = static_cast<float> 
					(m_entities[currentIdx][j]->getPoint(k)->x());
				y[currentSegment->getStartIndex() + k] = static_cast<float> 
					(m_entities[currentIdx][j]->getPoint(k)->y());
			}
		}
		
		m_chart = createLineChart(x, y, size);
		m_chartView->setChart(m_chart);
		m_chartView->update();
		//m_charts << m_chartView;
		delete[] x;
		delete[] y;
	}
}

int ATMDisplayProfilesDlg::getSelectedIndex() const
{
	//get selected items
	QList<QListWidgetItem*> list = profileList->selectedItems();
	if (list.isEmpty()) return -1; //display error message

	else if (profileList->currentRow() == -1) return 0;
	else return profileList->currentRow();
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
	QString outputFilename = QFileDialog::getSaveFileName(this, "Select destination", 
		path, tr("Text files(*.txt)"));

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
				for (int k = 0; k < m_entities[i][j]->getSize() - 1; k++) stream << 
					m_entities[i][j]->getPoint(k)->x() << "\t" << 
					m_entities[i][j]->getPoint(k)->y() << "\n";
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
					for (int k = 0; k < m_entities[i][j]->getSize(); k++) stream << 
						m_entities[i][j]->getPoint(k)->x()  << "\t" << 
						m_entities[i][j]->getPoint(k)->y() << "\n";
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
			stream << "start" << "\t" << "end" << "\t" << "intercept" << "\t" << "slope" 
				<< "\t" << "r2" << "\t" << "var" << "\n";

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

void ATMDisplayProfilesDlg::exportDataAsImg()
{
	//open file saving dialog
	QString outputFilename = QFileDialog::getSaveFileName(this, "Select destination", tr("Images (*.png *.jpg)"));

	if (outputFilename.isEmpty())
		return;

	QImage* img = new QImage(m_chartView->size().width(), m_chartView->size().height(), QImage::Format_ARGB32_Premultiplied);
	QPainter p(img);
	m_chartView->render(&p);
	p.end();
	img->save(outputFilename);

	/*if (img->save(outputFilename))
		m_app->dispToConsole(QString("[qATM] Image '%1' successfully saved.").arg(outputFilename),
			ccMainAppInterface::STD_CONSOLE_MESSAGE);
	else m_app->dispToConsole(QString("[qATM] Failed to save image '%1'!").arg(outputFilename),
		ccMainAppInterface::WRN_CONSOLE_MESSAGE);*/

}

void ATMDisplayProfilesDlg::exportAllDataAsImg()
{

}