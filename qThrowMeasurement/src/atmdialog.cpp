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

	//connect(step, static_cast<void (QDoubleSpinBox::*)(double)> (&QDoubleSpinBox::valueChanged), this, &ATMDialog::onStepChanged);
}


/*
ATMDialog::~ATMDialog()
{
    delete ui;
}
*/
