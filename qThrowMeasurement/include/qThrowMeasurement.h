//##########################################################################
//#                                                                        #
//#                CLOUDCOMPARE PLUGIN: ExamplePlugin                      #
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
//#                             COPYRIGHT: XXX                             #
//#                                                                        #
//##########################################################################

#pragma once

#include "ccStdPluginInterface.h"
#include "Neighbourhood.h"
#include "dppiecewiselinearregression.h"
#include "profileprocessor.h"
#include "atmdialog.h"

//! Example qCC plugin
/** Replace 'ExamplePlugin' by your own plugin class name throughout and then
	check 'ExamplePlugin.cpp' for more directions.

	Each plugin requires an info.json file to provide information about itself -
	the name, authors, maintainers, icon, etc..

	The one method you are required to implement is 'getActions'. This should
	return all actions (QAction objects) for the plugin. CloudCompare will
	automatically add these with their icons in the plugin toolbar and to the
	plugin menu. If	your plugin returns	several actions, CC will create a
	dedicated toolbar and a	sub-menu for your plugin. You are responsible for
	connecting these actions to	methods in your plugin.

	Use the ccStdPluginInterface::m_app variable for access to most of the CC
	components (database, 3D views, console, etc.) - see the ccMainAppInterface
	class in ccMainAppInterface.h.
**/
class qThrowMeasurement : public QObject, public ccStdPluginInterface
{
	Q_OBJECT
	Q_INTERFACES( ccPluginInterface ccStdPluginInterface )

	// Replace "Example" by your plugin name (IID should be unique - let's hope your plugin name is unique ;)
	// The info.json file provides information about the plugin to the loading system and
	// it is displayed in the plugin information dialog.
        Q_PLUGIN_METADATA( IID "cccorp.cloudcompare.plugin.qThrowMeasurement" FILE "../info.json" )

public:
    explicit qThrowMeasurement( QObject *parent = nullptr );
    ~qThrowMeasurement() override = default;

	//// 2D FUNCTIONNALITY
	void computeThrowMeasurement();
	void computeSegmentation(std::vector<ccPolyline*> polylines);
	void displayProfile(ccPointCloud* pc);

	//// 3D FUNCTIONNALITY
	ScalarType computeAngularDifference(double theta1, double theta2);
	ScalarType getAngleFromVerticality(ccPointCloud* cloud);

	void exportData(std::vector<SegmentLinearRegression*> segments, int index);
	ccPolyline* importGeneratrixFromDB();

	// Inherited from ccStdPluginInterface
	void onNewSelection( const ccHObject::Container &selectedEntities ) override;
	QList<QAction *> getActions() override;

protected:

	//! Associated (MDI) window
	ccGLWindow* m_associatedWin;

	//! Associated interface
	ccMainAppInterface* m_mainAppInterface;

	ATMDialog* m_atmDlg;


protected:

	//! Imported entity
	template<class EntityType> struct ImportedEntity
	{
		//! Default constructor
		ImportedEntity()
			: entity(0)
			, originalDisplay(nullptr)
			, isInDB(false)
			, backupColorShown(false)
			, backupWidth(1)
		{}

		//! Copy constructor
		ImportedEntity(const ImportedEntity& section)
			: entity(section.entity)
			, originalDisplay(section.originalDisplay)
			, isInDB(section.isInDB)
			, backupColorShown(section.backupColorShown)
			, backupWidth(section.backupWidth)
		{
			backupColor = section.backupColor;
		}

		//! Constructor from an entity
		ImportedEntity(EntityType* e, bool alreadyInDB)
			: entity(e)
			, originalDisplay(e->getDisplay())
			, isInDB(alreadyInDB)
		{
			//specific case: polylines
			if (e->isA(CC_TYPES::POLY_LINE))
			{
				ccPolyline* poly = reinterpret_cast<ccPolyline*>(e);
				//backup color
				backupColor = poly->getColor();
				backupColorShown = poly->colorsShown();
				//backup thickness
				backupWidth = poly->getWidth();
			}
		}

		bool operator ==(const ImportedEntity& ie) { return entity == ie.entity; }

		EntityType* entity;
		ccGenericGLDisplay* originalDisplay;
		bool isInDB;

		//backup info (for polylines only)
		ccColor::Rgb backupColor;
		bool backupColorShown;
		PointCoordinateType backupWidth;
		ccPolyline cloud;
	};

private:
	//! Default action
	/** You can add as many actions as you want in a plugin.
		Each action will correspond to an icon in the dedicated
		toolbar and an entry in the plugin menu.
	**/
	QAction* m_computeThrowMeasurement;
	QAction* m_computeAngularDifference;

	std::vector<profileProcessor*> m_processors;

};
