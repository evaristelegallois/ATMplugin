#include "qatmselectentitiesdlg.h"
#include "ui_qatmselectentitiesdlg.h"

#include <QDialog>
#include <QListWidgetItem>

/*qATMSelectEntitiesDlg::qATMSelectEntitiesDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::qATMSelectEntitiesDlg)
{
    ui->setupUi(this);
}

qATMSelectEntitiesDlg::~qATMSelectEntitiesDlg()
{
    delete ui;
}*/

qATMSelectEntitiesDlg::qATMSelectEntitiesDlg(
	QWidget* parent/*=0*/,
	QString itemName/*="entities"*/,
	QString labelStr/*=QString()*/)
	: QDialog(parent, Qt::Tool)
	, Ui::qATMSelectEntitiesDlg()
{
	setupUi(this);

	label->setText(tr("Please select one %1").arg(itemName));

}

int qATMSelectEntitiesDlg::SelectEntity(const ccHObject::Container& entities,
	int selectedIndex/*=0*/,
	QWidget* parent/*=0*/,
	QString label/*=QString()*/)
{
	qATMSelectEntitiesDlg ATMSEDlg(parent, tr("entity"), label);

	QStringList items;
	for (size_t i = 0; i < entities.size(); ++i)
	{
		//add one line per entity
		items << QString("%1 (ID=%2)").arg(entities[i]->getName()).arg(entities[i]->getUniqueID());
	}
	ATMSEDlg.setItems(items, selectedIndex);

	if (!ATMSEDlg.exec())
	{
		//cancelled by the user
		return -1;
	}

	return ATMSEDlg.getSelectedIndex();
}

int qATMSelectEntitiesDlg::getSelectedIndex() const
{
	//get selected items
	QList<QListWidgetItem*> list = listWidget->selectedItems();
	return list.empty() ? -1 : listWidget->row(list.front());
}

void qATMSelectEntitiesDlg::setItems(const QStringList& items, int defaultSelectedIndex)
{
	for (int i = 0; i < items.size(); ++i)
	{
		//add one line per entity in the combo-box
		listWidget->insertItem(static_cast<int>(i), new QListWidgetItem(items[i]));
	}

	//default selection
	if (defaultSelectedIndex >= 0 && defaultSelectedIndex < items.size())
	{
		listWidget->setItemSelected(listWidget->item(defaultSelectedIndex), true);
	}
}
