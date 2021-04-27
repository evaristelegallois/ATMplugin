#include "atmdialog.h"
#include "ui_atmdialog.h"

//qCC_plugins
#include "ccMainAppInterface.h"

//qCC_db
#include <ccPolyline.h>

//Qt
#include <QSettings>
#include <QMainWindow>
#include <QComboBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QPushButton>
#include <QApplication>
#include <QThread>


ATMDialog::ATMDialog(ccPolyline* polyline, ccMainAppInterface* app) :
    QDialog(app ? app->getMainWindow() : nullptr),
    Ui::ATMDialog()
{
    setupUi(this);
}

/*
ATMDialog::~ATMDialog()
{
    delete ui;
}
*/
