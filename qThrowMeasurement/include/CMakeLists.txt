
target_sources( ${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_LIST_DIR}/ActionA.h
		${CMAKE_CURRENT_LIST_DIR}/qThrowMeasurement.h
		${CMAKE_CURRENT_LIST_DIR}/atmdialog.h
		${CMAKE_CURRENT_LIST_DIR}/envelopeextractor.h
		${CMAKE_CURRENT_LIST_DIR}/envelopeextractordlg.h
)

target_include_directories( ${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)
