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
#include "atmdialog.h"

#include "ActionA.h"
#include "ActionA.cpp"

#include "ccFacet.h"

//Qt
#include <QMainWindow>
#include <QVector2D>
#include <GeometricalAnalysisTools.h>
#include <Jacobi.h>

using namespace CCCoreLib;


//semi-persistent dialog values
static float s_p = 1.00;
static const char* s_type = "var";
static int s_size, s_jumps = 0;


// Default constructor:
//	- pass the Qt resource path to the info.json file (from <yourPluginName>.qrc file) 
//  - constructor should mainly be used to initialize actions and other members
qThrowMeasurement::qThrowMeasurement( QObject *parent )
	: QObject( parent )
    , ccStdPluginInterface( ":/CC/plugin/qThrowMeasurement/info.json" )
	, m_computeThrowMeasurement(nullptr)
	, m_computeAngularDifference(nullptr)
	, m_associatedWin (nullptr)
{
}

// This method should enable or disable your plugin actions
// depending on the currently selected entities ('selectedEntities').
void qThrowMeasurement::onNewSelection( const ccHObject::Container &selectedEntities )
{
	if (m_computeThrowMeasurement)
		m_computeThrowMeasurement->setEnabled(selectedEntities.size() >= 1 && 
			(selectedEntities.back()->isA(CC_TYPES::HIERARCHY_OBJECT) || selectedEntities.back()->isA(CC_TYPES::POLY_LINE)));
	
	if (m_computeAngularDifference)
		m_computeAngularDifference->setEnabled(selectedEntities.size() == 1 && 
			selectedEntities.back()->isA(CC_TYPES::POINT_CLOUD));

}

// This method returns all the 'actions' your plugin can perform.
// getActions() will be called only once, when plugin is loaded.
QList<QAction *> qThrowMeasurement::getActions()
{
	// default action (if it has not been already created, this is the moment to do it)
	if (!m_computeThrowMeasurement)
	{
		// Here we use the default plugin name, description, and icon,
		// but each action should have its own.
		m_computeThrowMeasurement = new QAction("", this );
		m_computeThrowMeasurement->setToolTip( "" );
		m_computeThrowMeasurement->setIcon(QIcon(QString::fromUtf8(":/CC/plugin/qThrowMeasurement/img/angularDiff.png")));
		
		// Connect appropriate signal
		connect(m_computeThrowMeasurement, &QAction::triggered, this, &qThrowMeasurement::computeThrowMeasurement);
	}

	if (!m_computeAngularDifference)
	{
		m_computeAngularDifference = new QAction("", this);
		m_computeAngularDifference->setToolTip("");
		m_computeAngularDifference->setIcon(QIcon(QString::fromUtf8(":/CC/plugin/qThrowMeasurement/img/angularDiff.png")));
		//connect signal
		//connect(m_computeAngularDifference, &QAction::triggered, this, [this]);
	}

	return QList<QAction*>{
		    m_computeThrowMeasurement,
			m_computeAngularDifference,
	};
}

////2D FUNCTIONNALITY
void qThrowMeasurement::computeThrowMeasurement()
{
	std::vector<ccPolyline*> profiles;
	for (int i = 0; i < m_app->getSelectedEntities().size(); i++)
	{
		if (m_app->getSelectedEntities().back()->isA(CC_TYPES::POLY_LINE))
		{
			ccPolyline * polyline = ccHObjectCaster::ToPolyline(m_app->getSelectedEntities().back());
			profiles[i] = polyline;
		}
	}
	computeSegmentation(profiles);
}

void qThrowMeasurement::computeSegmentation(std::vector<ccPolyline*> profiles)
{
	assert(m_app);
	if (!m_app)
		return;

	ATMDialog atmDlg(m_app);


	atmDlg.pDoubleSpinBox->setValue(s_p);
	atmDlg.jSpinBox->setChecked(s_jumps);
	//atmDlg.sizeSpinBox->setValue(s_size);
	//atmDlg.

	if (!atmDlg.exec())
		return;

	s_p = atmDlg.pDoubleSpinBox->value();
	s_jumps = atmDlg.jSpinBox->value();
	//s_type = atmDlg.jSpinBox-> ;


	/*fusionDlg.octreeLevelSpinBox->setValue(s_octreeLevel);
	fusionDlg.useRetroProjectionCheckBox->setChecked(s_fmUseRetroProjectionError);
	fusionDlg.minPointsPerFacetSpinBox->setValue(s_minPointsPerFacet);
	//"no normal" warning
	fusionDlg.noNormalWarningLabel->setVisible(!pc->hasNormals());

	if (!fusionDlg.exec())
		return;

	s_octreeLevel = fusionDlg.octreeLevelSpinBox->value();
	s_fmUseRetroProjectionError = fusionDlg.useRetroProjectionCheckBox->isChecked();
	s_minPointsPerFacet = fusionDlg.minPointsPerFacetSpinBox->value();
	s_errorMeasureType = fusionDlg.errorMeasureComboBox->currentIndex();
	s_errorMaxPerFacet = fusionDlg.maxRMSDoubleSpinBox->value();
	s_kdTreeFusionMaxAngle_deg = fusionDlg.maxAngleDoubleSpinBox->value();
	s_kdTreeFusionMaxRelativeDistance = fusionDlg.maxRelativeDistDoubleSpinBox->value();
	s_maxEdgeLength = fusionDlg.maxEdgeLengthDoubleSpinBox->value();*/

	/*//create scalar field to host the fusion result
	const char c_defaultSFName[] = "facet indexes";
	int sfIdx = pc->getScalarFieldIndexByName(c_defaultSFName);
	if (sfIdx < 0)
		sfIdx = pc->addScalarField(c_defaultSFName);
	if (sfIdx < 0)
	{
		m_app->dispToConsole("Couldn't allocate a new scalar field for computing fusion labels! 
		Try to free some memory ...", ccMainAppInterface::ERR_CONSOLE_MESSAGE);
		return;
	}
	pc->setCurrentScalarField(sfIdx);*/


	std::vector<QVector<QVector2D>> inputs;
	std::vector<ccPolyline*> outputs;
	for (int i = 0; i < profiles.size(); i++)
	{
		profileProcessor* processor = new profileProcessor(profiles[i]);
		inputs[i] = processor->profileToXY();
	}

	int n = inputs[0].size();
	int k = 0; //remove if size given by user
	float* x, * y;

	for (int i = 0; i < profiles.size(); i++)
	{
		for (int j = 0; j < n; j++)
		{
			x[j] = inputs[i][j][0];
			y[j] = inputs[i][j][1];
			k++;
		}

		//get linear regression parameters here
		dPPiecewiseLinearRegression* model = new dPPiecewiseLinearRegression(x, y, s_size, s_p, s_jumps, s_type);
		std::vector<SegmentLinearRegression>* segments = model->computeSegmentation();

		for (int i = 0; i < profiles.size(); i++)
		{
			profileProcessor* processor = new profileProcessor(profiles[i]);
			outputs[i] = processor->segmentToProfile();
		}
	}
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

//void qThrowMeasurement::compute(ccMainAppInterface* m_app)
//{
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

