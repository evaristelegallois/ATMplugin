
target_sources( ${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_LIST_DIR}/qThrowMeasurement.cpp
		${CMAKE_CURRENT_LIST_DIR}/atmdialog.cpp
		${CMAKE_CURRENT_LIST_DIR}/profileprocessor.cpp
		${CMAKE_CURRENT_LIST_DIR}/segmentlinearregression.cpp
		${CMAKE_CURRENT_LIST_DIR}/dppiecewiselinearregression.cpp
		${CMAKE_CURRENT_LIST_DIR}/atmselectentitiesdlg.cpp
		${CMAKE_CURRENT_LIST_DIR}/atmdisplayprofilesdlg.cpp

		${CMAKE_CURRENT_LIST_DIR}/HAC_Average.cpp
		${CMAKE_CURRENT_LIST_DIR}/LinearRegression.cpp
		${CMAKE_CURRENT_LIST_DIR}/MatrixDistance.cpp
		${CMAKE_CURRENT_LIST_DIR}/StatisticalTools.cpp
		${CMAKE_CURRENT_LIST_DIR}/TreeNode.cpp
		${CMAKE_CURRENT_LIST_DIR}/Clusters.cpp
)
