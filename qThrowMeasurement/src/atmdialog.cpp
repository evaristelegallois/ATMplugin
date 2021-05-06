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

//system
#include <assert.h>


ATMDialog::ATMDialog(ccPolyline* polyline, ccMainAppInterface* app) :
    QDialog(app ? app->getMainWindow() : nullptr),
    Ui::ATMDialog(),
	app(app)
{
    setupUi(this);

	connect(step, static_cast<void (QDoubleSpinBox::*)(double)> (&QDoubleSpinBox::valueChanged), this, &ATMDialog::onStepChanged);
}


///// orthosectiontool dialog
void ATMDialog::setPathLength(double l)
{
	m_pathLength = l;
	step->setValue(l / 9);
	secLength->setValue(l / 5);

	qDebug() << "length" << m_pathLength;
	qDebug() << "step" << step->value();
}

void ATMDialog::setGenerationStep(double s)
{
	step->setValue(s);
}

void ATMDialog::setSectionsWidth(double w)
{
	secLength->setValue(w);
}

double ATMDialog::getGenerationStep() const
{
	return step->value();
}

double ATMDialog::getSectionsWidth() const
{
	return secLength->value();
}

void ATMDialog::onStepChanged(double step)
{
	if (step < 0)
		return;

	unsigned count = step < 1.0e-6 ? 1 : 1 + static_cast<unsigned>(std::floor(m_pathLength / step));
	//sectionCountLineEdit->setText(QString::number(count));
}

///// sectionextractiondlg
void ATMDialog::setActiveSectionCount(int count)
{
	activeSectionsLabel_3->setText(QString::number(count));
}

void ATMDialog::setSectionThickness(double t)
{
	thicknessDoubleSpinBox_3->setValue(t);
}

double ATMDialog::getSectionThickness() const
{
	return thicknessDoubleSpinBox_3->value();
}

double ATMDialog::getMaxEdgeLength() const
{
	return maxEdgeLengthDoubleSpinBox_3->value();
}

void ATMDialog::setMaxEdgeLength(double l)
{
	maxEdgeLengthDoubleSpinBox_3->setValue(l);
}

bool ATMDialog::extractClouds() const
{
	return extractCloudsGroupBox->isChecked();
}

void ATMDialog::doExtractClouds(bool state)
{
	extractCloudsGroupBox->setChecked(state);
}

bool ATMDialog::extractEnvelopes() const
{
	return extractEnvelopesGroupBox->isChecked();
}

void ATMDialog::doExtractEnvelopes(bool state, EnvelopeExtractor::EnvelopeType type)
{
	extractEnvelopesGroupBox->setChecked(state);

	switch (type)
	{
	case EnvelopeExtractor::LOWER:
		envelopeTypeComboBox_3->setCurrentIndex(0);
		break;
	case EnvelopeExtractor::UPPER:
		envelopeTypeComboBox_3->setCurrentIndex(1);
		break;
	case EnvelopeExtractor::FULL:
		envelopeTypeComboBox_3->setCurrentIndex(2);
		break;
	default:
		assert(false);
		break;
	}
}

bool ATMDialog::splitEnvelopes() const
{
	return splitEnvelopeCheckBox_3->isChecked();
}

void ATMDialog::doSplitEnvelopes(bool state)
{
	splitEnvelopeCheckBox_3->setChecked(state);
}

bool ATMDialog::useMultiPass() const
{
	return multiPassCheckBox_3->isChecked();
}

void ATMDialog::doUseMultiPass(bool state)
{
	multiPassCheckBox_3->setChecked(state);
}

EnvelopeExtractor::EnvelopeType ATMDialog::getEnvelopeType() const
{
	switch (envelopeTypeComboBox_3->currentIndex())
	{
	case 0:
		return EnvelopeExtractor::LOWER;
	case 1:
		return EnvelopeExtractor::UPPER;
	case 2:
		return EnvelopeExtractor::FULL;
	default:
		assert(false);
		break;
	}

	return EnvelopeExtractor::FULL;
}

bool ATMDialog::visualDebugMode() const
{
	return debugModeCheckBox_3->isChecked();
}

/*
ATMDialog::~ATMDialog()
{
    delete ui;
}
*/
