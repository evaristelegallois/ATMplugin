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

	// Inherited from ccStdPluginInterface
	void onNewSelection( const ccHObject::Container &selectedEntities ) override;
	QList<QAction *> getActions() override;

protected:

	//! Associated (MDI) window
	ccGLWindow* m_associatedWin;

	double angle; //to compute non-ortho sections as well

	void createOrthoSections(ccMainAppInterface* appInterface);
	void extractPointsFromSections();

	//! Convert one or several ReferenceCloud instances to a single cloud and add it to the main DB
	bool extractSectionToCloud(const std::vector<CCCoreLib::ReferenceCloud*>& refClouds,
		unsigned sectionIndex,
		bool& cloudGenerated);

	//! Extract the envelope from a set of 2D points and add it to the main DB
	bool extractSectionToEnvelope(const ccPolyline* originalSection,
		const ccPointCloud* originalSectionCloud,
		ccPointCloud* unrolledSectionCloud, //'2D' cloud with Z = 0
		unsigned sectionIndex,
		ccEnvelopeExtractor::EnvelopeType type,
		PointCoordinateType maxEdgeLength,
		bool multiPass,
		bool splitEnvelope,
		bool& envelopeGenerated,
		bool visualDebugMode = false);

protected:

	//! Creates (if necessary) and returns a group to store entities in the main DB
	ccHObject* getSectionExportGroup(unsigned& defaultGroupID, const QString& defaultName);

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
	};

	//! Section
	using Section = ImportedEntity<ccPolyline>;

	//! Cloud
	using Cloud = ImportedEntity<ccGenericPointCloud>;

	//! Type of the pool of active sections
	using SectionPool = QList<Section>;

	//! Type of the pool of clouds
	using CloudPool = QList<Cloud>;

private:
	//! Default action
	/** You can add as many actions as you want in a plugin.
		Each action will correspond to an icon in the dedicated
		toolbar and an entry in the plugin menu.
	**/
	QAction* sectionExtraction;

	//! Pool of active sections
	SectionPool m_sections;

	//! Selected polyline (if any)
	Section* m_selectedPoly;

	//! Pool of clouds
	CloudPool m_clouds;

};
