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
//#                COPYRIGHT: Gabriel Parel, Sophie Viseur                 #
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
#include "ccFacet.h"

//qATM
#include "qThrowMeasurement.h"
#include "ccMainAppInterface.h"
#include "qatmselectentitiesdlg.h"
#include "atmdisplayprofilesdlg.h"

//Qt
#include <QMainWindow>
#include <QVector2D>
#include <QVector>
#include <QFile>
#include <QTextStream>

//CCCoreLib
#include <GeometricalAnalysisTools.h>
#include <Jacobi.h>

using namespace CCCoreLib;

// Default constructor
qThrowMeasurement::qThrowMeasurement( QObject *parent )
	: QObject( parent )
    , ccStdPluginInterface( ":/CC/plugin/qThrowMeasurement/info.json" )
	, m_computeThrowMeasurement(nullptr)
	, m_computeAngularDifference(nullptr)
	, m_associatedWin (nullptr)
	, m_mainAppInterface (nullptr)
	, m_atmDlg(nullptr)
	, m_radius (20.0) //temporary; to be replaced by user input
	, m_cloud (nullptr)
{
}

// This method should enable or disable your plugin actions
// depending on the currently selected entities ('selectedEntities').
void qThrowMeasurement::onNewSelection( const ccHObject::Container &selectedEntities )
{
	if (m_computeThrowMeasurement)
		m_computeThrowMeasurement->setEnabled(selectedEntities.size() >= 1 &&
			selectedEntities.back()->isA(CC_TYPES::POLY_LINE));
	
	if (m_computeAngularDifference)
		m_computeAngularDifference->setEnabled(selectedEntities.size() == 1 && 
			selectedEntities.back()->isA(CC_TYPES::HIERARCHY_OBJECT));
}

// This method returns all the 'actions' your plugin can perform.
// getActions() will be called only once, when plugin is loaded.
QList<QAction *> qThrowMeasurement::getActions()
{
	// default action (if it has not been already created, this is the moment to do it)
	if (!m_computeThrowMeasurement)
	{
		m_computeThrowMeasurement = new QAction("Automatic Throw Measurement (2D)", this );
		m_computeThrowMeasurement->setToolTip( "" );
		m_computeThrowMeasurement->setIcon(QIcon(QString::fromUtf8(":/CC/plugin/qThrowMeasurement/img/angularDiff.png")));
		// Connect appropriate signal
		connect(m_computeThrowMeasurement, &QAction::triggered, this, &qThrowMeasurement::computeThrowMeasurement2D);
	}

	if (!m_computeAngularDifference)
	{
		m_computeAngularDifference = new QAction("Automatic Throw Measurement (3D)", this);
		m_computeAngularDifference->setToolTip("");
		m_computeAngularDifference->setIcon(QIcon(QString::fromUtf8(":/CC/plugin/qThrowMeasurement/img/angularDiff.png")));
		// Connect appropriate signal
		connect(m_computeAngularDifference, &QAction::triggered, this, &qThrowMeasurement::computeThrowMeasurement3D);
	}

	return QList<QAction*>{
		    m_computeThrowMeasurement,
			m_computeAngularDifference,
	};
}

////2D FUNCTIONNALITY
void qThrowMeasurement::computeThrowMeasurement2D()
{
	//get the selected entities
	const int size = m_app->getSelectedEntities().size();
	std::vector<ccPolyline*> profiles; //inputs
	profiles.reserve(size);

	for (int i = 0; i < m_app->getSelectedEntities().size(); i++)
	{
		if (m_app->getSelectedEntities()[i]->isA(CC_TYPES::POLY_LINE))
		{
			ccPolyline * polyline = ccHObjectCaster::ToPolyline(m_app->getSelectedEntities()[i]);
			profiles.push_back(polyline);
		}
	}

	ccLog::Print(QString("[qATM] Computation has started."));

	assert(m_app);
	if (!m_app)
		return;

	//opens the 2D plug-in main dialog
	m_atmDlg = new ATMDialog(m_app, profiles);
	m_atmDlg->setModal(false);

	//process dialog here!!!!!!

	if (!m_atmDlg->exec())
		return; //cancellation by user

	ccLog::Print(QString("[qATM] Computation has ended.")); //before
}

////3D FUNCTIONNALITY
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
	ScalarType theta, sum = 0;

	if (!cloud)
	{
		//return nullptr;
		qDebug() << "No cloud selected!";
	}

	//computes verticality specifically, with a neighbouring sphere radius of m_radius
	GeometricalAnalysisTools::ComputeCharactersitic(
		static_cast<GeometricalAnalysisTools::GeomCharacteristic>(0), 10, cloud, m_radius);

	for (int i = 0; i < cloud->size(); i++) {
		double value = (cloud->getPointScalarValue(i));
		if (!isnormal(value)) value = 0;
		sum += acos(1 - value); //computes theta from verticality
		//qDebug() << "point value" << cloud->getPointScalarValue(i);
	}

	theta = sum / cloud->size();

	return theta; //return scalar field instead
}

void qThrowMeasurement::computeThrowMeasurement3D()
{
	//we expect a facet group
	const ccHObject::Container& selectedEntities = m_app->getSelectedEntities();
	if (!m_app->haveOneSelection() || !selectedEntities.back()->isA(CC_TYPES::HIERARCHY_OBJECT))
	{
		m_app->dispToConsole("Select a group of facets!");
	}

	//lets the user choose the initial cloud
	QMainWindow* mainWindow = m_app->getMainWindow();
	if (!mainWindow)
		ccLog::Error("Main window not found!");

	ccHObject* root = m_app->dbRootObject();
	ccHObject::Container clouds;
	if (root) root->filterChildren(clouds, true, CC_TYPES::POINT_CLOUD);

	ccPointCloud* cloud;
	if (!clouds.empty())
	{
		int index = qATMSelectEntitiesDlg::SelectEntity(clouds);
		m_cloud = static_cast<ccPointCloud*>(clouds[index]);
	}
	else ccLog::Error("No point cloud in DB!");

	std::vector<ccFacet*> FacetSet;
	int size = 0;
	for (int i = 0; i < selectedEntities.size(); i++)
	{
		size += selectedEntities[i]->getChildrenNumber();
		qDebug() << "nb of facets" << size;
		FacetSet.reserve(size);
		for (int j = 0; j < selectedEntities[i]->getChildrenNumber(); j++)
		{
			FacetSet.push_back(static_cast<ccFacet*> (selectedEntities[i]->getChild(j)));
		}
	}

	qDebug() << "facetset size" << FacetSet.size();
	//for (int i = 0; i < FacetSet.size(); i++) FacetSet.push_back(static_cast<ccFacet*> (FacetSet[i]));
	compareFacetsGroup(FacetSet);
	
}


PointCoordinateType qThrowMeasurement::ComputeHDistBetweenFacets(const ccFacet* f1, const ccFacet* f2)
{
	CCVector3 AB = f1->getCenter() - f2->getCenter();
	return std::min(std::abs(AB.dot(f1->getNormal())), std::abs(AB.dot(f2->getNormal())));
}

void qThrowMeasurement::compareFacetsGroup(std::vector<ccFacet*> FacetSet)
{
	ScalarType *theta, theta1, theta2, deltaTheta;
	theta = new ScalarType[m_cloud->size()];
	int cloudSize = 0;
	double minDist = 0.;
	double maxDist = 200.;
	double curDist = 0.;

	//creating the point cloud
	ccPointCloud* cloud = new ccPointCloud("Vertices");
	cloud->reserve(m_cloud->size());
	qDebug() << "cloud size" << m_cloud->size();

	//creating the conrresponding scalar field
	int sfIdx = cloud->getScalarFieldIndexByName("Angle");
	if (sfIdx < 0)
		sfIdx = cloud->addScalarField("Angle");
	cloud->getScalarField(sfIdx)->reserve(m_cloud->size());
	cloud->setCurrentScalarField(sfIdx);

	//filling out the scalar field
	for (int i = 0; i < FacetSet.size(); i++) {
		cloud->append(FacetSet[i]->getOriginPoints(), cloudSize);
		theta[i] = static_cast<double> (getAngleFromVerticality(FacetSet[i]->getOriginPoints())); //computes theta from verticality
		cloudSize += FacetSet[i]->getOriginPoints()->size(); //not equal to cloud size??
		qDebug() << "theta" << theta[i];
	}

	qDebug() << "cloudSize" << cloudSize;
	qDebug() << "theta ok";

	for (int i = 0; i < FacetSet.size() - 1; i++)
	{
		for (int j = 0; j < FacetSet.size() - 1; j++)
		{
			//we restrain the computation to facets close to each other
			curDist = abs(ComputeHDistBetweenFacets(FacetSet[i], FacetSet[j]));
			if (curDist < maxDist)
			{
				deltaTheta = static_cast<double> (computeAngularDifference(theta[i], theta[j]));
				cloud->setPointScalarValue(i, deltaTheta); //assigns value to each point
				cloud->getScalarField(sfIdx)->addElement(deltaTheta);
			}
		}
	}

	qDebug() << "cloud ok";

	cloud->setCurrentScalarField(sfIdx);
	cloud->getScalarField(sfIdx)->computeMinAndMax();
	cloud->setCurrentDisplayedScalarField(sfIdx);
	cloud->showSF(true);

	m_app->addToDB(cloud);
	m_app->redrawAll();
	//only a part of the scalar field is displayed???

	qDebug() << "compute ok";
}

