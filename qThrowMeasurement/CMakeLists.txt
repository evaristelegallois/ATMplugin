# CloudCompare example for standard plugins

# REPLACE ALL 'ExamplePlugin' OCCURENCES BY YOUR PLUGIN NAME
# AND ADAPT THE CODE BELOW TO YOUR OWN NEEDS!

# Add an option to CMake to control whether we build this plugin or not
option( PLUGIN_ATM_STANDARD "Install example plugin" OFF )

if ( PLUGIN_ATM_STANDARD )
	project( qThrowMeasurement )
	 
	AddPlugin( NAME ${PROJECT_NAME} )
		
	add_subdirectory( include )
	add_subdirectory( src )
	add_subdirectory( ui )
	
	# set dependencies to necessary libraries
	# target_link_libraries( ${PROJECT_NAME} LIB1 )

	find_package(Qt5Charts)
	target_link_libraries(${PROJECT_NAME} Qt5::Charts)
endif()
