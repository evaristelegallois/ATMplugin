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

// First:
//	Replace all occurrences of 'ExamplePlugin' by your own plugin class name in this file.
//	This includes the resource path to info.json in the constructor.

// Second:
//	Open ExamplePlugin.qrc, change the "prefix" and the icon filename for your plugin.
//	Change the name of the file to <yourPluginName>.qrc

// Third:
//	Open the info.json file and fill in the information about the plugin.
//	 "type" should be one of: "Standard", "GL", or "I/O" (required)
//	 "name" is the name of the plugin (required)
//	 "icon" is the Qt resource path to the plugin's icon (from the .qrc file)
//	 "description" is used as a tootip if the plugin has actions and is displayed in the plugin dialog
//	 "authors", "maintainers", and "references" show up in the plugin dialog as well

#include <QtGui>

//qCC_db
#include <ccGenericPointCloud.h>
#include <ccHObjectCaster.h>
#include <ccLog.h>
#include <ccPointCloud.h>
#include <ccPolyline.h>
#include <ccProgressDialog.h>

#include "qThrowMeasurement.h"
#include "envelopeextractor.h"
#include "ccMainAppInterface.h"

#include "ActionA.h"
#include "ActionA.cpp"

#include "ccFacet.h"

//Qt
#include <QMainWindow>
#include <QVector2D>
#include <GeometricalAnalysisTools.h>
#include <Jacobi.h>

using namespace CCCoreLib;

// Default constructor:
//	- pass the Qt resource path to the info.json file (from <yourPluginName>.qrc file) 
//  - constructor should mainly be used to initialize actions and other members
qThrowMeasurement::qThrowMeasurement( QObject *parent )
	: QObject( parent )
    , ccStdPluginInterface( ":/CC/plugin/qThrowMeasurement/info.json" )
	, sectionExtraction( nullptr )
	, m_associatedWin (nullptr)
{
}

struct Segment2D
{
	Segment2D() : s(0) {}

	CCVector2 A, B, uAB;
	PointCoordinateType lAB;
	PointCoordinateType s; //curvilinear coordinate
};

// This method should enable or disable your plugin actions
// depending on the currently selected entities ('selectedEntities').
void qThrowMeasurement::onNewSelection( const ccHObject::Container &selectedEntities )
{
	if ( sectionExtraction == nullptr )
	{
		return;
	}
	
	// If you need to check for a specific type of object, you can use the methods
	// in ccHObjectCaster.h or loop and check the objects' classIDs like this:
	//
	//	for ( ccHObject *object : selectedEntities )
	//	{
	//		if ( object->getClassID() == CC_TYPES::VIEWPORT_2D_OBJECT )
	//		{
	//			// ... do something with the viewports
	//		}
	//	}
	
	// For example - only enable our action if something is selected.
	if (sectionExtraction)
	{
		//classification: only one point cloud
		sectionExtraction->setEnabled(selectedEntities.size() == 1 && selectedEntities[0]->isA(CC_TYPES::POLY_LINE));
	}

}

// This method returns all the 'actions' your plugin can perform.
// getActions() will be called only once, when plugin is loaded.
QList<QAction *> qThrowMeasurement::getActions()
{
	// default action (if it has not been already created, this is the moment to do it)
	if ( !sectionExtraction)
	{
		// Here we use the default plugin name, description, and icon,
		// but each action should have its own.
		sectionExtraction = new QAction( getName(), this );
		sectionExtraction->setToolTip( getDescription() );
		sectionExtraction->setIcon( getIcon() );
		
		// Connect appropriate signal
		connect(sectionExtraction, &QAction::triggered, this, [this]()
		{
			//createOrthoSections( m_app );
		});
	}

	return { sectionExtraction };
}


ScalarType qThrowMeasurement::computeAngularDifference(double theta1, double theta2)
{
	QVector2D v1 = QVector2D(sin(theta1), cos(theta1));
	QVector2D v2 = QVector2D(sin(theta2), cos(theta2));
	QVector2D v3;

	double result = v3.dotProduct(v1, v2);
	return acos(result); //returns the difference between theta1 & theta2
}

ScalarType qThrowMeasurement::getAngleFromVerticality(ccPointCloud* cloud)
{
	ScalarType theta;

	if (!cloud)
	{
		//return nullptr;
	}

	//ccFacet* facet = ccFacet::Create(cloud, static_cast<PointCoordinateType>(10), true);

	//computes verticality specifically, with a neighbouring sphere radius of 20
	GeometricalAnalysisTools::ComputeCharactersitic(
		static_cast<GeometricalAnalysisTools::GeomCharacteristic>(0), 10, cloud, 20.0);

	int index = cloud->getScalarFieldIndexByName("Verticality (20.0)");
	int index2 = cloud->addScalarField("Angle");
	cloud->setCurrentOutScalarField(index);
	cloud->setCurrentInScalarField(index2);

	for (int i = 0; i < cloud->size(); i++) {
		theta = acos(1 - (cloud->getPointScalarValue(i))); //computes theta from verticality
		cloud->setPointScalarValue(i, theta); //assigns value to each point
	}

	//scalarField->computeMinAndMax();
}

void qThrowMeasurement::compute(ccMainAppInterface* m_app)
{
	// TO PUT A THE BEGINING OF PLUGIN
	/*assert(m_app);
	if (!m_app)
	return;*/

/*
	//we expect a facet group
	const ccHObject::Container& selectedEntities = m_app->getSelectedEntities();
	if (!m_app->haveOneSelection() || !selectedEntities.back()->isA(CC_TYPES::HIERARCHY_OBJECT))
	{
		m_app->dispToConsole("Select a group of facets!");
	}
}


static PointCoordinateType qThrowMeasurement::ComputeHDistBetweenFacets(const ccFacet* f1, const ccFacet* f2)
{
	CCVector3 AB = f1->getCenter() - f2->getCenter();
	return std::min(std::abs(AB.dot(f1->getNormal())), std::abs(AB.dot(f2->getNormal())));
}


void qThrowMeasurement::compareFacetsGroup(std::vector<ccFacet*> FacetSet)
{
	ScalarType theta1, theta2, deltaTheta;
	double minDist = 0;
	double maxDist = 200;
	double curDist = 0;

	int index = cloud->getScalarFieldIndexByName("Verticality (20.0)");
	int index2 = cloud->addScalarField("Angle");
	cloud->setCurrentOutScalarField(index);
	cloud->setCurrentInScalarField(index2);

	for (int i = 0; i < cloud->size(); i++) {
		theta = acos(1 - (cloud->getPointScalarValue(i))); //computes theta from verticality
		cloud->setPointScalarValue(i, theta); //assigns value to each point
	}

	//scalarField->computeMinAndMax();
	for (int i = 0; i < FacetSet.size() - 1; i++)
	{
		curDist = abs(ComputeHDistBetweenFacets(FacetSet[i], FacetSet[i + 1]));
		if (curDist < maxDist)
		{
			theta1 = getAngleFromVerticality(FacetSet[i]->getOriginPoints());
			theta2 = getAngleFromverticality(FacetSet[i + 1]->getOriginPoints());
			deltaTheta = computeAngularDifference(theta1, theta2);
		}
	}
}
*/

