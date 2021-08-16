
target_sources( ${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_LIST_DIR}/ActionA.h
		${CMAKE_CURRENT_LIST_DIR}/qThrowMeasurement.h
		${CMAKE_CURRENT_LIST_DIR}/atmdialog.h
		${CMAKE_CURRENT_LIST_DIR}/profileprocessor.h
		${CMAKE_CURRENT_LIST_DIR}/segmentlinearregression.h
		${CMAKE_CURRENT_LIST_DIR}/dppiecewiselinearregression.h
		${CMAKE_CURRENT_LIST_DIR}/qatmselectentitiesdlg.h
		${CMAKE_CURRENT_LIST_DIR}/atmdisplayprofilesdlg.h

		${CMAKE_CURRENT_LIST_DIR}/HAC_Average.h
		${CMAKE_CURRENT_LIST_DIR}/LinearRegression.h
		${CMAKE_CURRENT_LIST_DIR}/MatrixDistance.h
		${CMAKE_CURRENT_LIST_DIR}/StatisticalTools.h
		${CMAKE_CURRENT_LIST_DIR}/TreeNode.h
		${CMAKE_CURRENT_LIST_DIR}/HierarchicalAgglomerativeClustering.h
		${CMAKE_CURRENT_LIST_DIR}/Clusters.h
		
)

target_include_directories( ${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)
