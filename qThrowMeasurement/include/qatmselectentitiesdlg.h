#ifndef QATMSELECTENTITIESDLG_H
#define QATMSELECTENTITIESDLG_H

#include <QDialog>
#include <ui_qATMSelectEntitiesDlg.h>

//qCC_db
#include <ccHObject.h>

class qATMSelectEntitiesDlg : public QDialog, public Ui::qATMSelectEntitiesDlg
{
    Q_OBJECT

public:
	qATMSelectEntitiesDlg(
		QWidget* parent = 0,
		QString itemName = "entities",
		QString label = QString());
    //~qATMSelectEntitiesDlg();

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

#endif // QATMSELECTENTITIESDLG_H
