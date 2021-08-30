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
	std::vector<std::vector<int>> startIdx, std::vector<std::vector<int>> endIdx, QWidget *parent) :
    QDialog(parent),
    Ui::ATMDisplayProfilesDlg(),
	m_entities(entities),
	m_startIdx(startIdx),
	m_endIdx(endIdx)
{
    setupUi(this);

	connect(saveAsTxtBtn, &QPushButton::released, this, &ATMDisplayProfilesDlg::exportDataAsTxt);
	connect(saveAllAsTxtBtn, &QPushButton::released, this, &ATMDisplayProfilesDlg::exportAllDataAsTxt);
	connect(saveAsImgBtn, &QPushButton::released, this, &ATMDisplayProfilesDlg::exportDataAsImg);
	connect(saveAllAsImgBtn, &QPushButton::released, this, &ATMDisplayProfilesDlg::exportAllDataAsImg);
	connect(profileList, &QListWidget::currentRowChanged, this, &ATMDisplayProfilesDlg::displayChart);
	connect(genCheckBox, &QCheckBox::stateChanged, this, &ATMDisplayProfilesDlg::displayChart);
	connect(clusterCheckBox, &QCheckBox::stateChanged, this, &ATMDisplayProfilesDlg::displayChart);

	this->setModal(false);

	QStringList items;
	for (size_t i = 0; i < m_entities.size(); ++i)
	{
		//add one line per entity
		items << QString("Profile #ID=%1 (p=%2)").arg(m_entities[i][0]->getUniqueSharedID()).arg(m_entities[i][0]->getAssociatedP());
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

//LINE CHART FOR SEGMENTATION & clustering RESULTS
QChart* ATMDisplayProfilesDlg::createLineChart(float* x, float* y, int n) const
{
	QChart* chart = new QChart();
	chart->setTitle(QString("Scarp height (in m) relative to curvilinear abscissa (Profile ID#%1, segmentation parameter p=%2)")
		.arg(m_entities[getSelectedIndex()][0]->getUniqueSharedID()).arg(m_entities[getSelectedIndex()][0]->getAssociatedP()));
	QLineSeries* series = new QLineSeries(chart);

	QScatterSeries* clusterPos = new QScatterSeries(chart);
	clusterPos->setMarkerShape(QScatterSeries::MarkerShapeCircle);
	clusterPos->setColor(QColor(0, 255, 0));
	clusterPos->setBorderColor(QColor(0, 255, 0));
	clusterPos->setMarkerSize(5.0);

	std::vector<QLineSeries*> segmentPos;
	//for (int i = 0; i < m_sStartIdx[getSelectedIndex()].size(); i++)
	for (int i = 0; i < m_entities[getSelectedIndex()].size(); i++)
	{
		QLineSeries* serie = new QLineSeries(chart);
		segmentPos.push_back(serie);
		QPen pen = segmentPos[i]->pen();
		pen.setWidth(5);
		const QColor color = QColor(rand() % 255 + 0, rand() % 255 + 0, rand() % 255 + 0);
		pen.setColor(color);
		segmentPos[i]->setPen(pen);
	}

	QScatterSeries* transectPos = new QScatterSeries(chart);
	transectPos->setMarkerShape(QScatterSeries::MarkerShapeCircle);
	transectPos->setColor(QColor(255,0,0));
	transectPos->setMarkerSize(10.0);

	QValueAxis* axisX = new QValueAxis;
	QValueAxis* axisY = new QValueAxis;
	axisX->setTitleText("Curvilinear abscissa");
	axisY->setTitleText("Height (m)");

	for (int i = 0; i < n; i++) series->append(x[i], y[i]);
	chart->legend()->hide();
	chart->addSeries(series);

	if (clusterCheckBox->isChecked())
	{
		for (int j = 0; j < m_startIdx[getSelectedIndex()].size(); j++)
		{
			for (int k = m_startIdx[getSelectedIndex()][j]; k <= m_endIdx[getSelectedIndex()][j]; k++)
				clusterPos->append(x[k], y[k]);
		}
		chart->addSeries(clusterPos);
	}
	else {
		/*for (int j = 0; j < m_sStartIdx[getSelectedIndex()].size(); j++)
		{
			for (int k = m_sStartIdx[getSelectedIndex()][j]; k <= m_sEndIdx[getSelectedIndex()][j]; k++)
				segmentPos[j]->append(x[k], y[k]);
			chart->addSeries(segmentPos[j]);
		}*/

		for (int j = 0; j < m_entities[getSelectedIndex()].size(); j++)
		{
			for (int k = m_entities[getSelectedIndex()][j]->getStartIndex(); k <= m_entities[getSelectedIndex()][j]->getEndIndex(); k++)
				segmentPos[j]->append(x[k], y[k]);
			chart->addSeries(segmentPos[j]);
		}
	}

	if (genCheckBox->isChecked())
	{
		if (m_entities[getSelectedIndex()][0]->getTransectPosition() == 0) transectPos->append(-1, -1);
		else transectPos->append(x[m_entities[getSelectedIndex()][0]->getTransectPosition()],
			y[m_entities[getSelectedIndex()][0]->getTransectPosition()]); //generatrix position
		chart->addSeries(transectPos);
	}

	chart->addAxis(axisX, Qt::AlignBottom);
	chart->addAxis(axisY, Qt::AlignLeft);
	series->attachAxis(axisX);
	series->attachAxis(axisY);
	if (clusterCheckBox->isChecked())
	{
		clusterPos->attachAxis(axisX);
		clusterPos->attachAxis(axisY);
	}
	else {
		for (int j = 0; j < m_entities[getSelectedIndex()].size(); j++)
		{
			segmentPos[j]->attachAxis(axisX);
			segmentPos[j]->attachAxis(axisY);
		}
	}
	if (genCheckBox->isChecked())
	{
		transectPos->attachAxis(axisX);
		transectPos->attachAxis(axisY);
	}


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
		int currentIdx = getSelectedIndex();
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

	int size = 0;
	for (int k = 0; k < m_entities[idx].size(); k++) size += m_entities[idx][k]->getSize();

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

		//header
		stream << "profile #ID" << m_entities[idx][0]->getUniqueSharedID() << "\n" <<
			"nb of points" << (size - m_entities[idx].size()) << "\n";

		stream << "curvilinear abscissa" << "\t" << "z" << "\n";

			for (int j = 0; j < m_entities[idx].size(); j++)
			{
				for (int k = 0; k < m_entities[idx][j]->getSize() - 1; k++) stream << 
					m_entities[idx][j]->getPoint(k)->x() << "\t" << 
					m_entities[idx][j]->getPoint(k)->y() << "\n";
			}
		//when done
		file.close();
		ccLog::Print(QString("[qATM] File '%1' successfully saved.").arg(path));
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
		
		//header
			stream << "profile #ID" << m_entities[idx][0]->getUniqueSharedID() << "\n" <<
				"nb of points" << (size - m_entities[idx].size()) << "\n" <<
				"p value" << m_entities[idx][0]->getAssociatedP() << "\n";

		stream << "start" << "\t" << "end" << "\t" << "intercept"
			<< "\t" << "slope" << "\t" << "r2" << "\t" << "var" << "\n";

			for (int j = 0; j < m_entities[idx].size(); j++)
			{
				stream << m_entities[idx][j]->getStartIndex() << "\t"
					<< m_entities[idx][j]->getEndIndex() << "\t" << m_entities[idx][j]->getIntercept()
					<< "\t" << m_entities[idx][j]->getSlope() << "\t"
					<< m_entities[idx][j]->getRSquare() << "\t" << m_entities[idx][j]->getVar() << "\n";
			}
		//when done
		file2.close();
		ccLog::Print(QString("[qATM] File '%1' successfully saved.").arg(path2));
	}
}

void ATMDisplayProfilesDlg::exportAllDataAsTxt()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"));
	for (int l = 0; l < m_entities.size(); l++)
	{
		//open file saving dialog
		QString path = dir + QString("/profile#%1_coordinates.txt").arg(l);

		int size = 0; //profile size (nb of points)
		for (int k = 0; k < m_entities[l].size(); k++) size += m_entities[l][k]->getSize();
		//coordinates
		QFile file(path);
		if (file.open(QIODevice::ReadWrite)) {
			QTextStream stream(&file);

			stream << "profile #ID " << m_entities[l][0]->getUniqueSharedID() << "\n"
				<< "nb of points " << (size - m_entities[l].size()) << "\n";

			stream << "curvilinear abscissa" << "\t" << "z" << "\n";

				for (int j = 0; j < m_entities[l].size() ; j++)
				{
					for (int k = 0; k < m_entities[l][j]->getSize() - 1; k++) stream << 
						m_entities[l][j]->getPoint(k)->x()  << "\t" << 
						m_entities[l][j]->getPoint(k)->y() << "\n";
				}

			//when done
			file.close();
			ccLog::Print(QString("[qATM] File '%1' successfully saved.").arg(path));
		}

		//segmentation related data
		QString path2 = dir + QString("/profile#%1_segmentationData.txt").arg(l);

		QFile file2(path2);
		if (file2.open(QIODevice::ReadWrite)) {
			QTextStream stream(&file2);

			//header
				stream << "profile #ID " << m_entities[l][0]->getUniqueSharedID() << "\n"
					<< "nb of points " << (size - m_entities[l].size()) << "\n"
					<< "p value " << m_entities[l][0]->getAssociatedP() << "\n";

			stream << "start" << "\t" << "end" << "\t" << "intercept" << "\t" << "slope" 
				<< "\t" << "r2" << "\t" << "var" << "\n";

				for (int j = 0; j < m_entities[l].size(); j++)
				{
					stream << m_entities[l][j]->getStartIndex() << "\t"
						<< m_entities[l][j]->getEndIndex() << "\t" << m_entities[l][j]->getIntercept()
						<< "\t" << m_entities[l][j]->getSlope() << "\t"
						<< m_entities[l][j]->getRSquare() << "\t" << m_entities[l][j]->getVar() << "\n";
				}
			//when done
			file2.close();
			ccLog::Print(QString("[qATM] File '%1' successfully saved.").arg(path2));
		}

		//clustering related data
		QString path3 = dir + QString("/profile#%1_clusteringData.txt").arg(l);

		QFile file3(path3);
		if (file3.open(QIODevice::ReadWrite)) {
			QTextStream stream(&file3);

			//header
			stream << "profile #ID " << m_entities[l][0]->getUniqueSharedID() << "\n"
				<< "nb of points " << (size - m_entities[l].size()) << "\n"
				<< "nb of clusters " << m_startIdx.size() << "\n";

			stream << "start" << "\t" << "end" << "\t" << "intercept" << "\t" << "slope"
				<< "\t" << "r2" << "\t" << "var" << "\n";

			for (int j = 0; j < m_startIdx[l].size(); j++)
			{
				stream << m_startIdx[l][j] << "\t"
					<< m_endIdx[l][j] << "\t" << m_entities[l][j]->getIntercept()
					<< "\t" << m_entities[l][j]->getSlope() << "\t"
					<< m_entities[l][j]->getRSquare() << "\t" << m_entities[l][j]->getVar() << "\n";
			}
			//when done
			file3.close();
			ccLog::Print(QString("[qATM] File '%1' successfully saved.").arg(path3));
		}
	}
}

void ATMDisplayProfilesDlg::exportDataAsImg()
{
	QString path = QString("/profile#%1_segmentationData.png").arg(getSelectedIndex());
	QString outputFilename = QFileDialog::getSaveFileName(this, "Select destination", path, tr("Images(*.png)"));

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

void ATMDisplayProfilesDlg::exportAllDataAsImg()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Select destination"));
	for (int l = 0; l < m_entities.size(); l++)
	{
		profileList->setCurrentRow(l);
		displayChart();

		QString outputFilename;
		if (clusterCheckBox->isChecked()) outputFilename = dir + QString("/profile#%1_segmentationData.png").arg(l);
		else outputFilename = dir + QString("/profile#%1_clusteringData.png").arg(l);

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
}