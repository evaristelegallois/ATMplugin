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


ATMDialog::ATMDialog(ccPolyline* polyline, ccMainAppInterface* app) :
    QDialog(app ? app->getMainWindow() : nullptr),
    Ui::ATMDialog(),
	app(app)
{
    setupUi(this);

	connect(step, static_cast<void (QDoubleSpinBox::*)(double)> (&QDoubleSpinBox::valueChanged), this, &ATMDialog::onStepChanged);
}

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
/*
ATMDialog::~ATMDialog()
{
    delete ui;
}
*/
