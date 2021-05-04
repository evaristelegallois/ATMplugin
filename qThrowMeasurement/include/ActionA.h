// Example of a plugin action

#pragma once

//Local
#include "ccOverlayDialog.h"
#include "ATMDialog.h"

//qCC
#include <ccStdPluginInterface.h>

//qCC_db
#include <ccHObject.h>

class ccMainAppInterface;

namespace Ui
{
	class ATMDialog;
}

//! Section extraction tool
class ActionA
{

public:
	//! Default constructor
	//explicit ActionA(QWidget* parent);
	//void createOrthoSections(ccMainAppInterface* appInterface);


private: //members

	//! Associated UI
	Ui::ATMDialog* m_UI;
};