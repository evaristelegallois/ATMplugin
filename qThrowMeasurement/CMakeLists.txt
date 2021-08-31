option( PLUGIN_ATM_STANDARD "Install ATM plugin" ON )

if ( PLUGIN_ATM_STANDARD )
	project( qThrowMeasurement )
	 
	AddPlugin( NAME ${PROJECT_NAME} )
		
	add_subdirectory( include )
	add_subdirectory( src )
	add_subdirectory( ui )

	find_package(Qt5Charts)
	target_link_libraries(${PROJECT_NAME} Qt5::Charts)
endif()
