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

//Qt
#include <QMainWindow>

//#include "../../../../../qCC/ccEnvelopeExtractor.h"
//#include "../../../../../qCC/ccSectionExtractionSubDlg.h"
//#include "../../../../../qCC/mainwindow.h"


//default export groups
static unsigned s_polyExportGroupID = 0;
static unsigned s_profileExportGroupID = 0;
static unsigned s_cloudExportGroupID = 0;

//default arrow size
static const PointCoordinateType s_defaultArrowSize = 20;


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
			createOrthoSections( m_app );
		});
	}

	return { sectionExtraction };
}

//needs to add edition fonctions, also a way to update the view in real time
void qThrowMeasurement::createOrthoSections(ccMainAppInterface* appInterface)
{
	if (appInterface == nullptr)
	{
		// The application interface should have already been initialized when the plugin is loaded
		Q_ASSERT(false);

		return;
	}

	m_mainAppInterface = appInterface;

	/*** HERE STARTS THE ACTION ***/

	//////generate ortho sections

	//get the selected polyline
	const ccHObject::Container& selectedEntities = appInterface->getSelectedEntities();
	ccPolyline* poly = ccHObjectCaster::ToPolyline(selectedEntities[0]);
	m_selectedPolyline = poly;

	//initialize relevant parameters
	static double s_orthoSectionWidth = -1.0;
	static double s_orthoSectionStep = -1.0;

	//compute polyline length
	unsigned vertCount = (poly ? poly->size() : 0);
	if (vertCount < 2)
	{
		ccLog::Warning("[qThrowMeasurementPlugin] Invalid polyline");
		return;
	}

	PointCoordinateType length = poly->computeLength();

	//display dialog
	ATMDialog atmDlg(poly, appInterface);

	atmDlg.setPathLength(length);
	if (s_orthoSectionWidth > 0.0)
		atmDlg.setSectionsWidth(s_orthoSectionWidth);
	if (s_orthoSectionStep > 0.0)
		atmDlg.setGenerationStep(s_orthoSectionStep);
	//osgDlg.setAutoSaveAndRemove(s_autoSaveAndRemoveGeneratrix);

	if (atmDlg.exec())
	{
		//now generate the orthogonal sections
		s_orthoSectionStep = atmDlg.getGenerationStep();
		s_orthoSectionWidth = atmDlg.getSectionsWidth();
		//s_autoSaveAndRemoveGeneratrix = atmDlg.autoSaveAndRemove();

		/*
		if (s_autoSaveAndRemoveGeneratrix)
		{
			//save
			if (!m_selectedPoly->isInDB)
			{
				ccHObject* destEntity = getExportGroup(s_polyExportGroupID, "Exported sections");
				assert(destEntity);
				destEntity->addChild(m_selectedPoly->entity);
				m_selectedPoly->isInDB = true;
				m_selectedPoly->entity->setDisplay_recursive(destEntity->getDisplay());
				MainWindow::TheInstance()->addToDB(m_selectedPoly->entity, false, false);
			}
			//and remove
			deleteSelectedPolyline();
		}
		*/

		//set new 'undo' step
		//addUndoStep();

		//normal to the plane
		CCVector3 N(0, 0, 0);
		int vertDim = 0; //0, 1 or 2 in original plugin
		assert(vertDim >= 0 && vertDim < 3);
		{
			N.u[vertDim] = 1.0;
		}

		//curvilinear position
		double s = 0;
		//current length
		double l = 0;
		unsigned maxCount = vertCount;
		if (!poly->isClosed())
			maxCount--;
		unsigned polyIndex = 0;
		for (unsigned i = 0; i < maxCount; ++i)
		{
			const CCVector3* A = poly->getPoint(i);
			const CCVector3* B = poly->getPoint((i + 1) % vertCount);
			CCVector3 AB = (*B - *A);
			AB.u[vertDim] = 0;
			CCVector3 nAB = AB.cross(N);
			nAB.normalize();

			double lAB = (*B - *A).norm();
			while (s < l + lAB)
			{
				double s_local = s - l;
				assert(s_local < lAB);

				//create orhogonal polyline
				ccPointCloud* vertices = new ccPointCloud("vertices");
				ccPolyline* orthoPoly = new ccPolyline(vertices);
				orthoPoly->addChild(vertices);
				if (vertices->reserve(2) && orthoPoly->reserve(2))
				{
					//intersection point
					CCVector3 I = *A + AB * (s_local / lAB);
					CCVector3 I1 = I + nAB * static_cast<PointCoordinateType>(s_orthoSectionWidth / 2);
					CCVector3 I2 = I - nAB * static_cast<PointCoordinateType>(s_orthoSectionWidth / 2);

					vertices->addPoint(I1);
					orthoPoly->addPointIndex(0);
					vertices->addPoint(I2);
					orthoPoly->addPointIndex(1);

					orthoPoly->setClosed(false);
					orthoPoly->set2DMode(false);
					orthoPoly->copyGlobalShiftAndScale(*poly);

					//set default display style
					vertices->setEnabled(false);
					orthoPoly->showColors(true);
					orthoPoly->setColor(ccColor::magenta);
					orthoPoly->setWidth(1);
					
					//orthoPoly->setDisplay_recursive(m_associatedWin); //set the same 'default' display as the cloud

					m_app->addToDB(orthoPoly);

					orthoPoly->setName(QString("%1.%2").arg(poly->getName()).arg(++polyIndex));

					//add meta data (for Mascaret export)
					{
						orthoPoly->setMetaData(ccPolyline::MetaKeyUpDir(), QVariant(vertDim));
						orthoPoly->setMetaData(ccPolyline::MetaKeyAbscissa(), QVariant(s));
						orthoPoly->setMetaData(ccPolyline::MetaKeyPrefixCenter() + ".x", QVariant(static_cast<double>(I.x)));
						orthoPoly->setMetaData(ccPolyline::MetaKeyPrefixCenter() + ".y", QVariant(static_cast<double>(I.y)));
						orthoPoly->setMetaData(ccPolyline::MetaKeyPrefixCenter() + ".z", QVariant(static_cast<double>(I.z)));
						orthoPoly->setMetaData(ccPolyline::MetaKeyPrefixDirection() + ".x", QVariant(static_cast<double>(nAB.x)));
						orthoPoly->setMetaData(ccPolyline::MetaKeyPrefixDirection() + ".y", QVariant(static_cast<double>(nAB.y)));
						orthoPoly->setMetaData(ccPolyline::MetaKeyPrefixDirection() + ".z", QVariant(static_cast<double>(nAB.z)));
					}

					/*
					if (!addPolyline(orthoPoly, false))
					{
						delete orthoPoly;
						orthoPoly = nullptr;
					}
					*/
				}
				else
				{
					delete orthoPoly;
					orthoPoly = nullptr;
					ccLog::Error("Not enough memory!");
					//early stop
					i = maxCount;
					break;
				}

				s += s_orthoSectionStep;
			}

			l += lAB;
		}
	}

	m_app->redrawAll();
	extractPointsFromSections();
}

void qThrowMeasurement::extractPointsFromSections()
{
	static double s_defaultSectionThickness = -1.0;
	static double s_envelopeMaxEdgeLength = 0;
	static bool s_extractSectionsAsClouds = false;
	static bool s_extractSectionsAsEnvelopes = true;
	static bool s_multiPass = false;
	static bool s_splitEnvelope = false;
	static EnvelopeExtractor::EnvelopeType s_extractSectionsType = EnvelopeExtractor::LOWER;

	//number of eligible sections
	unsigned sectionCount = 0;
	{
		for (auto& section : m_sections)
		{
			if (section.entity && section.entity->size() > 1)
				++sectionCount;
		}
	}
	if (sectionCount == 0)
	{
		ccLog::Error("No (valid) section!");
		return;
	}

	//compute loaded clouds bounding-box
	ccBBox box;
	unsigned pointCount = 0;

	for (auto& cloud : m_clouds)
	{
		if (cloud.entity)
		{
			box += cloud.entity->getOwnBB();
			pointCount += cloud.entity->size();
		}
	}

	if (s_defaultSectionThickness <= 0)
	{
		s_defaultSectionThickness = box.getMaxBoxDim() / 500.0;
	}
	if (s_envelopeMaxEdgeLength <= 0)
	{
		s_envelopeMaxEdgeLength = box.getMaxBoxDim() / 500.0;
	}

	//display dialog
	ATMDialog sesDlg(m_selectedPolyline, m_mainAppInterface);

	sesDlg.setActiveSectionCount(sectionCount);
	sesDlg.setSectionThickness(s_defaultSectionThickness);
	sesDlg.setMaxEdgeLength(s_envelopeMaxEdgeLength);
	sesDlg.doExtractClouds(s_extractSectionsAsClouds);
	sesDlg.doExtractEnvelopes(s_extractSectionsAsEnvelopes, s_extractSectionsType);
	sesDlg.doUseMultiPass(s_multiPass);
	sesDlg.doSplitEnvelopes(s_splitEnvelope);

	if (!sesDlg.exec())
		return;

	s_defaultSectionThickness = sesDlg.getSectionThickness();
	s_envelopeMaxEdgeLength = sesDlg.getMaxEdgeLength();
	s_extractSectionsAsClouds = sesDlg.extractClouds();
	s_extractSectionsAsEnvelopes = sesDlg.extractEnvelopes();
	s_extractSectionsType = sesDlg.getEnvelopeType();
	s_multiPass = sesDlg.useMultiPass();
	s_splitEnvelope = sesDlg.splitEnvelopes();
	bool visualDebugMode = sesDlg.visualDebugMode();

	//progress dialog
	ccProgressDialog pdlg(true);
	CCCoreLib::NormalizedProgress nprogress(&pdlg, static_cast<unsigned>(sectionCount));
	if (!visualDebugMode)
	{
		pdlg.setMethodTitle(tr("Extract sections"));
		pdlg.setInfo(tr("Number of sections: %1\nNumber of points: %2").arg(sectionCount).arg(pointCount));
		pdlg.start();
		QCoreApplication::processEvents();
	}

	int vertDim = 0; //can be 0, 1 or 2
	int xDim = (vertDim < 2 ? vertDim + 1 : 0);
	int yDim = (xDim < 2 ? xDim + 1 : 0);

	//we consider half of the total thickness as points can be on both sides!
	double sectionThicknessSq = std::pow(s_defaultSectionThickness / 2.0, 2.0);
	bool error = false;

	unsigned generatedEnvelopes = 0;
	unsigned generatedClouds = 0;

	try
	{
		//for each slice
		for (int s = 0; s < m_sections.size(); ++s)
		{
			ccPolyline* poly = m_sections[s].entity;
			if (poly)
			{
				unsigned polyVertCount = poly->size();
				if (polyVertCount < 2)
				{
					assert(false);
					continue;
				}
				unsigned polySegmentCount = poly->isClosed() ? polyVertCount : polyVertCount - 1;

				//project the section in '2D'
				std::vector<Segment2D> polySegments2D;
				{
					polySegments2D.reserve(polySegmentCount);
					PointCoordinateType s = 0;
					for (unsigned j = 0; j < polySegmentCount; ++j)
					{
						Segment2D seg2D;
						const CCVector3* A = poly->getPoint(j);
						const CCVector3* B = poly->getPoint((j + 1) % polyVertCount);
						seg2D.A = CCVector2(A->u[xDim], A->u[yDim]);
						seg2D.B = CCVector2(B->u[xDim], B->u[yDim]);
						seg2D.uAB = seg2D.B - seg2D.A; //(unit) direction
						seg2D.lAB = seg2D.uAB.norm(); //length
						seg2D.s = s;
						s += seg2D.lAB;

						if (CCCoreLib::LessThanEpsilon(seg2D.lAB))
						{
							//ignore too small segments
							continue;
						}

						seg2D.uAB /= seg2D.lAB;
						polySegments2D.push_back(seg2D);
					}

					if (polySegments2D.empty())
					{
						assert(false);
						continue;
					}
					polySegments2D.shrink_to_fit();
				}

				int cloudCount = m_clouds.size();
				std::vector<CCCoreLib::ReferenceCloud*> refClouds;
				if (s_extractSectionsAsClouds)
				{
					refClouds.resize(cloudCount, nullptr);
				}

				//for envelope extraction as a polyline
				ccPointCloud* originalSlicePoints = nullptr;
				ccPointCloud* unrolledSlicePoints = nullptr;
				if (s_extractSectionsAsEnvelopes)
				{
					originalSlicePoints = new ccPointCloud("section.orig");
					unrolledSlicePoints = new ccPointCloud("section.unroll");

					//assign them the default (first!) global shift & scale info
					assert(!m_clouds.empty());
					ccGenericPointCloud* cloud = m_clouds.front().entity;
					originalSlicePoints->copyGlobalShiftAndScale(*cloud);
				}

				//for each cloud
				for (int c = 0; c < cloudCount; ++c)
				{
					ccGenericPointCloud* cloud = m_clouds[c].entity;
					if (cloud)
					{
						//for envelope extraction as a cloud
						CCCoreLib::ReferenceCloud* refCloud = nullptr;
						if (s_extractSectionsAsClouds)
						{
							refCloud = new CCCoreLib::ReferenceCloud(cloud);
						}

						//compute the distance of each point to the current polyline segment
						for (unsigned i = 0; i < cloud->size(); ++i)
						{
							const CCVector3* P = cloud->getPoint(i);
							CCVector2 P2D(P->u[xDim], P->u[yDim]);

							//for each vertex
							PointCoordinateType minSquareDist = -CCCoreLib::PC_ONE;
							PointCoordinateType curvilinearPos = 0.0;
							size_t minIndex = 0;
							for (size_t j = 0; j < polySegments2D.size(); ++j)
							{
								const Segment2D& seg2D = polySegments2D[j];
								CCVector2 AP2D = P2D - seg2D.A;

								//square distance to the polyline
								PointCoordinateType squareDist = 0;

								//longitudinal 'distance'
								double dotprod = seg2D.uAB.dot(AP2D);
								if (dotprod < 0)
								{
									if (j == 0 && !poly->isClosed())
										continue;
									squareDist = AP2D.norm2();
								}
								else if (dotprod > seg2D.lAB)
								{
									if (j + 1 == polySegments2D.size() && !poly->isClosed())
										continue;
									squareDist = (P2D - seg2D.B).norm2();
								}
								else
								{
									//orthogonal distance
									squareDist = (AP2D - seg2D.uAB * dotprod).norm2();
								}

								if (minSquareDist < 0 || squareDist < minSquareDist)
								{
									minSquareDist = squareDist;
									curvilinearPos = dotprod;
									minIndex = j;
								}
							}

							//elligible point?
							if (minSquareDist >= 0 && minSquareDist < sectionThicknessSq)
							{
								//if we extract the section as cloud(s), we add the point to the (current) ref. cloud
								if (s_extractSectionsAsClouds)
								{
									assert(refCloud);
									unsigned refCloudSize = refCloud->size();
									if (refCloudSize == refCloud->capacity())
									{
										refCloudSize += (refCloudSize / 2 + 1);
										if (!refCloud->reserve(refCloudSize))
										{
											//not enough memory
											ccLog::Warning("[ccSectionExtractionTool] Not enough memory");
											error = true;
											break;
										}
									}
									refCloud->addPointIndex(i);
								}

								//if we extract the section as envelope(s), we add it to the 2D points set
								if (s_extractSectionsAsEnvelopes)
								{
									assert(originalSlicePoints && unrolledSlicePoints);
									assert(originalSlicePoints->size() == unrolledSlicePoints->size());

									unsigned cloudSize = originalSlicePoints->size();
									if (cloudSize == originalSlicePoints->capacity())
									{
										cloudSize += (cloudSize / 2 + 1);
										if (!originalSlicePoints->reserve(cloudSize)
											|| !unrolledSlicePoints->reserve(cloudSize))
										{
											//not enough memory
											ccLog::Warning("[ccSectionExtractionTool] Not enough memory");
											error = true;
											break;
										}
									}

									const Segment2D& seg2D = polySegments2D[minIndex];

									//we project the 'real' 3D point in the section plane
									CCVector3 Pproj3D;
									{
										Pproj3D.u[xDim] = seg2D.A.x + seg2D.uAB.x * curvilinearPos;
										Pproj3D.u[yDim] = seg2D.A.y + seg2D.uAB.y * curvilinearPos;
										Pproj3D.u[vertDim] = P->u[vertDim];
									}
									originalSlicePoints->addPoint(Pproj3D);
									unrolledSlicePoints->addPoint(CCVector3(seg2D.s + curvilinearPos, P->u[vertDim], 0));
								}
							}

							if (error)
							{
								break;
							}

						} //for each point

						if (refCloud)
						{
							assert(s_extractSectionsAsClouds);
							if (error || refCloud->size() == 0)
							{
								delete refCloud;
								refCloud = nullptr;
							}
							else
							{
								refClouds[c] = refCloud;
							}
						}

					}

					if (error)
					{
						break;
					}

				} //for each cloud

				if (!error)
				{
					//Extract sections as (polyline) envelopes
					if (/*!error && */s_extractSectionsAsEnvelopes)
					{
						assert(originalSlicePoints && unrolledSlicePoints);
						bool envelopeGenerated = false;
						error = !extractSectionToEnvelope(poly,
							originalSlicePoints,
							unrolledSlicePoints,
							s + 1,
							s_extractSectionsType,
							s_envelopeMaxEdgeLength,
							s_multiPass,
							s_splitEnvelope,
							envelopeGenerated,
							visualDebugMode);

						if (envelopeGenerated)
						{
							++generatedEnvelopes;
						}
					}

					//Extract sections as clouds
					if (!error && s_extractSectionsAsClouds)
					{
						assert(static_cast<int>(refClouds.size()) == cloudCount);
						bool cloudGenerated = false;
						error = !extractSectionToCloud(refClouds, s + 1, cloudGenerated);
						if (cloudGenerated)
							++generatedClouds;
					}
				}

				//release memory
				for (auto& refCloud : refClouds)
				{
					delete refCloud;
					refCloud = nullptr;
				}

				delete originalSlicePoints;
				originalSlicePoints = nullptr;

				delete unrolledSlicePoints;
				unrolledSlicePoints = nullptr;
			} //if (poly)

			if (!nprogress.oneStep())
			{
				ccLog::Warning("[ccSectionExtractionTool] Canceled by user");
				error = true;
			}

			if (error)
				break;
		} //for (int s=0; s<m_sections.size(); ++s)
	}
	catch (const std::bad_alloc&)
	{
		error = true;
	}

	if (error)
	{
		ccLog::Error("An error occurred (see console)");
	}
	else
	{
		ccLog::Print(QString("[ccSectionExtractionTool] Job done (%1 envelope(s) and %2 cloud(s) were generated)").arg(generatedEnvelopes).arg(generatedClouds));
	}
}

bool qThrowMeasurement::extractSectionToEnvelope(const ccPolyline* originalSection,
	const ccPointCloud* originalSectionCloud,
	ccPointCloud* unrolledSectionCloud, //'2D' cloud with Z = 0
	unsigned sectionIndex,
	EnvelopeExtractor::EnvelopeType envelopeType,
	PointCoordinateType maxEdgeLength,
	bool multiPass,
	bool splitEnvelope,
	bool& envelopeGenerated,
	bool visualDebugMode)
{
	envelopeGenerated = false;

	if (!originalSectionCloud || !unrolledSectionCloud)
	{
		ccLog::Warning("[ccSectionExtractionTool][extract envelope] Internal error: invalid input parameter(s)");
		return false;
	}

	if (originalSectionCloud->size() < 2)
	{
		//nothing to do
		ccLog::Warning(QString("[ccSectionExtractionTool][extract envelope] Section #%1 contains less than 2 points and will be ignored").arg(sectionIndex));
		return true;
	}

	//by default, the points in 'unrolledSectionCloud' are 2D (X = curvilinear coordinate, Y = height, Z = 0)
	CCVector3 N(0, 0, 1);
	CCVector3 Y(0, 1, 0);

	std::vector<unsigned> vertIndexes;
	ccPolyline* envelope = EnvelopeExtractor::ExtractFlatEnvelope(unrolledSectionCloud,
		multiPass,
		maxEdgeLength,
		N.u,
		Y.u,
		envelopeType,
		&vertIndexes,
		visualDebugMode);

	if (envelope)
	{
		//update vertices (to replace 'unrolled' points by 'original' ones
		{
			CCCoreLib::GenericIndexedCloud* vertices = envelope->getAssociatedCloud();
			if (vertIndexes.size() == static_cast<size_t>(vertices->size()))
			{
				for (unsigned i = 0; i < vertices->size(); ++i)
				{
					const CCVector3* P = vertices->getPoint(i);
					assert(vertIndexes[i] < originalSectionCloud->size());
					*const_cast<CCVector3*>(P) = *originalSectionCloud->getPoint(vertIndexes[i]);
				}

				ccPointCloud* verticesAsPC = dynamic_cast<ccPointCloud*>(vertices);
				if (verticesAsPC)
					verticesAsPC->refreshBB();
			}
			else
			{
				ccLog::Warning("[ccSectionExtractionTool][extract envelope] Internal error (couldn't fetch original points indexes?!)");
				delete envelope;
				return false;
			}
		}

		std::vector<ccPolyline*> parts;
		if (splitEnvelope)
		{
#ifdef QT_DEBUG
			//compute some stats on the envelope
			{
				double minLength = 0;
				double maxLength = 0;
				double sumLength = 0;
				unsigned count = envelope->size();
				if (!envelope->isClosed())
					--count;
				for (unsigned i = 0; i < count; ++i)
				{
					const CCVector3* A = envelope->getPoint(i);
					const CCVector3* B = envelope->getPoint((i + 1) % envelope->size());
					CCVector3 e = *B - *A;
					double l = e.norm();
					if (i != 0)
					{
						minLength = std::min(minLength, l);
						maxLength = std::max(maxLength, l);
						sumLength += l;
					}
					else
					{
						minLength = maxLength = sumLength = l;
					}
				}
				ccLog::PrintDebug(QString("Envelope: min = %1 / avg = %2 / max = %3").arg(minLength).arg(sumLength / count).arg(maxLength));
			}
#endif

			/*bool success = */envelope->split(maxEdgeLength, parts);
			delete envelope;
			envelope = nullptr;
		}
		else
		{
			parts.push_back(envelope);
		}

		//create output group if necessary
		ccHObject* destEntity = getSectionExportGroup(s_profileExportGroupID, "Extracted profiles");
		assert(destEntity);

		for (size_t p = 0; p < parts.size(); ++p)
		{
			ccPolyline* envelopePart = parts[p];
			QString name = QString("Section envelope #%1").arg(sectionIndex);
			if (parts.size() > 1)
				name += QString("(part %1/%2)").arg(p + 1).arg(parts.size());
			envelopePart->setName(name);
			envelopePart->copyGlobalShiftAndScale(*originalSectionCloud);
			envelopePart->setColor(ccColor::green);
			envelopePart->showColors(true);
			//copy meta-data (import for Mascaret export!)
			{
				const QVariantMap& metaData = originalSection->metaData();
				for (QVariantMap::const_iterator it = metaData.begin(); it != metaData.end(); ++it)
				{
					envelopePart->setMetaData(it.key(), it.value());
				}
			}

			//add to main DB
			destEntity->addChild(envelopePart);
			envelopePart->setDisplay_recursive(destEntity->getDisplay());
			//MainWindow::TheInstance()->addToDB(envelopePart, false, false);
		}

		envelopeGenerated = true;
	}

	return true;
}

bool qThrowMeasurement::extractSectionToCloud(const std::vector<CCCoreLib::ReferenceCloud*>& refClouds,
	unsigned sectionIndex,
	bool& cloudGenerated)
{
	cloudGenerated = false;

	ccPointCloud* sectionCloud = nullptr;
	for (int i = 0; i < static_cast<int>(refClouds.size()); ++i)
	{
		if (!refClouds[i])
			continue;
		assert(m_clouds[i].entity); //a valid ref. cloud must have a valid counterpart!

		//extract part/section from each cloud
		ccPointCloud* part = nullptr;

		//if the cloud is a ccPointCloud, we can keep a lot more information
		//when extracting the section cloud
		ccPointCloud* pc = dynamic_cast<ccPointCloud*>(m_clouds[i].entity);
		if (pc)
		{
			part = pc->partialClone(refClouds[i]);
		}
		else
		{
			part = ccPointCloud::From(refClouds[i], m_clouds[i].entity);
		}

		if (part)
		{
			if (i == 0)
			{
				//we simply use this 'part' cloud as the section cloud
				sectionCloud = part;
			}
			else
			{
				//fuse it with the global cloud
				unsigned cloudSizeBefore = sectionCloud->size();
				unsigned partSize = part->size();
				sectionCloud->append(part, cloudSizeBefore, true);

				//don't need it anymore
				delete part;
				part = nullptr;
				//check that it actually worked!
				if (sectionCloud->size() != cloudSizeBefore + partSize)
				{
					//not enough memory
					ccLog::Warning("[ccSectionExtractionTool][extract cloud] Not enough memory");
					delete sectionCloud;
					return false;
				}
			}
		}
		else
		{
			//not enough memory
			ccLog::Warning("[ccSectionExtractionTool][extract cloud] Not enough memory");
			delete sectionCloud;
			return false;
		}
	}

	if (sectionCloud)
	{
		//create output group if necessary
		ccHObject* destEntity = getSectionExportGroup(s_cloudExportGroupID, "Extracted section clouds");
		assert(destEntity);

		sectionCloud->setName(QString("Section cloud #%1").arg(sectionIndex));
		sectionCloud->setDisplay(destEntity->getDisplay());

		//add to main DB
		destEntity->addChild(sectionCloud);
		//MainWindow::TheInstance()->addToDB(sectionCloud, false, false);

		cloudGenerated = true;
	}

	return true;
}


ccHObject* qThrowMeasurement::getSectionExportGroup(unsigned& defaultGroupID, const QString& defaultName)
{
	ccHObject* root = m_app ? m_app->dbRootObject() : nullptr;
	if (!root)
	{
		ccLog::Warning("Internal error (no MainWindow or DB?!)");
		assert(false);
		return nullptr;
	}

	ccHObject* destEntity = (defaultGroupID != 0 ? root->find(defaultGroupID) : nullptr);
	if (!destEntity)
	{
		destEntity = new ccHObject(defaultName);
		//assign default display
		for (auto& cloud : m_clouds)
		{
			if (cloud.entity)
			{
				destEntity->setDisplay_recursive(cloud.originalDisplay);
				break;
			}
		}
		m_app->addToDB(destEntity);
		defaultGroupID = destEntity->getUniqueID();
	}
	return destEntity;
}

