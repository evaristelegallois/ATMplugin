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

#ifndef ATMSELECTENTITIESDLG_H
#define ATMSELECTENTITIESDLG_H

#include <ui_ATMSelectEntitiesDlg.h>

//Qt
#include <QDialog>

//qCC_db
#include <ccHObject.h>

/// <summary>
/// Convenient class to easily select one polyline directly from DB.
/// </summary>
class ATMSelectEntitiesDlg : public QDialog, public Ui::ATMSelectEntitiesDlg
{
    Q_OBJECT

public:
	ATMSelectEntitiesDlg(
		QWidget* parent = 0,
		QString itemName = "entities",
		QString label = QString());
    //~ATMSelectEntitiesDlg();

public: //static shortcuts

	static int SelectEntity(const ccHObject::Container& entities,
		int defaultSelectedIndex = 0,
		QWidget* parent = 0,
		QString label = QString());

public:

	//! Sets the list of items
	void setItems(const QStringList& items, int defaultSelectedIndex = 0);

	//! Returns selected index (unique selection mode)
	int getSelectedIndex() const;

};

#endif // ATMSELECTENTITIESDLG_H
