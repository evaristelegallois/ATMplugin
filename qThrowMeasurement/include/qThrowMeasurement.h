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

#pragma once

#include "ccStdPluginInterface.h"

//ATM Plug-in
#include "dppiecewiselinearregression.h"
#include "profileprocessor.h"
#include "atmdialog.h"

/// 
/// The ATM (Automatic Throw Measurement) plug-in has been designed to automatically compute
/// geological throws from:
/// 
///		1) 2D profiles, obtained through sectionning of 3D geological data along a fault.
///		2) 3D geological data, by first automatically locating the fault along which the 
///		sectionning is done.
/// 
/// The detection method is still in the work in progress phase. It relies on the use of the qFacet 
/// plug-in (T. Dewez) as well as the computation of geometrical features (planarity, verticality).
/// The computation method relies on a piecewise linear regression (courtesy of S. Viseur, 
/// inspired by the "dpseg" R package from Machne & Stadler), followed by the throw computation
/// itself (see Puliti et al, 2020).
/// 
class qThrowMeasurement : public QObject, public ccStdPluginInterface
{
	Q_OBJECT
	Q_INTERFACES( ccPluginInterface ccStdPluginInterface )

        Q_PLUGIN_METADATA( IID "cccorp.cloudcompare.plugin.qThrowMeasurement" FILE "../info.json" )

public:
    explicit qThrowMeasurement( QObject *parent = nullptr );
    ~qThrowMeasurement() override = default;

	//// 2D FUNCTIONNALITY
	
	/// <summary>
	/// Opens the main plug-in dialog.
	/// </summary>
	void computeThrowMeasurement2D();

	//// 3D FUNCTIONNALITY; all functions have to be relocated to main 3D functionnality window
	//// this plug-in needs facets to work; use qFacets plug-in beforehand for better results
	
	/// <summary>
	/// Computes difference |theta1 - theta2|, with theta given as inputs.
	/// </summary>
	/// <param name="theta1">Facet 1 angle (computed from verticality), in rad</param>
	/// <param name="theta2">Facet 2 angle (computed from verticality), in rad</param>
	/// <returns>|theta1 - theta2| in rad (float)</returns>
	ScalarType computeAngularDifference(double theta1, double theta2);

	/// <summary>
	/// INCOMPLETE FUNCTION: should return an entire scalar field instead of a scalar value
	/// Computes verticality (angle alpha) in order to obtain theta = acos(1 - alpha)
	/// </summary>
	/// <param name="cloud">Selected point cloud</param>
	/// <returns>Scalar value theta, in rad (float)</returns>
	ScalarType getAngleFromVerticality(ccPointCloud* cloud);

	/// <summary>
	/// INCOMPLETE FUNCTION: not all the scalar field is displayed, restraining radius
	/// might be an issue?
	/// Compare all facets in a the selected facetSet. Comparison is restrained to facets
	/// close to each other (max radius = 200) to save time/memory.
	/// </summary>
	/// <param name="FacetSet">Selected facets</param>
	void compareFacetsGroup(std::vector<ccFacet*> FacetSet);

	/// <summary>
	/// Computes distance between facet1 & facet2 using their centers and normals.
	/// </summary>
	/// <param name="f1">Facet 1</param>
	/// <param name="f2">Facet 2</param>
	/// <returns>PointCoordinateType = float</returns>
	static PointCoordinateType ComputeHDistBetweenFacets(const ccFacet* f1, const ccFacet* f2);
	void computeThrowMeasurement3D();

	// Inherited from ccStdPluginInterface
	void onNewSelection( const ccHObject::Container &selectedEntities ) override;
	QList<QAction *> getActions() override;

protected:

	//! Associated (MDI) window
	ccGLWindow* m_associatedWin;
	//! Associated interface
	ccMainAppInterface* m_mainAppInterface;
	//! main 2D plug-in dialog
	ATMDialog* m_atmDlg;

private:
	//! 2D functionnality
	QAction* m_computeThrowMeasurement;
	//! 3D functionnality
	QAction* m_computeAngularDifference;

	//! neighboring sphere's radius (for verticality computation)
	double m_radius;
	//! selected cloud
	ccPointCloud* m_cloud;
};
