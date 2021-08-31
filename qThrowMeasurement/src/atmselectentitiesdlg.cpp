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
//#  This class has been copied from ccEntitySelectionDlg and thus the     #
//#  credit goes to D. Girardeau. It has been used under the terms of the  #
//#  GNU General Public License v2 and can be removed if deemed necessary. #
//#                                                                        #
//#               COPYRIGHT: Gabriel Parel, Daniel Girardeau               #
//#                                                                        #
//##########################################################################

#include "atmselectentitiesdlg.h"
#include "ui_atmselectentitiesdlg.h"

//Qt
#include <QDialog>
#include <QListWidgetItem>

ATMSelectEntitiesDlg::ATMSelectEntitiesDlg(
	QWidget* parent/*=0*/,
	QString itemName/*="entities"*/,
	QString labelStr/*=QString()*/)
	: QDialog(parent, Qt::Tool)
	, Ui::ATMSelectEntitiesDlg()
{
	setupUi(this);

	label->setText(tr("Please select one %1").arg(itemName));
}


int ATMSelectEntitiesDlg::SelectEntity(const ccHObject::Container& entities,
	int selectedIndex/*=0*/,
	QWidget* parent/*=0*/,
	QString label/*=QString()*/)
{
	ATMSelectEntitiesDlg ATMSEDlg(parent, tr("polyline"), label);

	QStringList items;
	for (size_t i = 0; i < entities.size(); ++i)
	{
		//add one line per entity
		items << QString("%1 (ID=%2)").arg(entities[i]->getName()).arg(entities[i]->getUniqueID());
	}
	ATMSEDlg.setItems(items, selectedIndex);

	if (!ATMSEDlg.exec())
	{
		//if cancelled, return first item 
		return 0;
	}

	return ATMSEDlg.getSelectedIndex();
}

int ATMSelectEntitiesDlg::getSelectedIndex() const
{
	//get selected items
	QList<QListWidgetItem*> list = listWidget->selectedItems();
	return list.empty() ? -1 : listWidget->row(list.front());
}

void ATMSelectEntitiesDlg::setItems(const QStringList& items, int defaultSelectedIndex)
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
