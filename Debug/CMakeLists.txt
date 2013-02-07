cmake_minimum_required(VERSION 2.8)

SET(CMAKE_BUILD_TYPE Debug)

include_directories(
	 ${CGoGN_ROOT_DIR}/include
	 ${COMMON_INCLUDES}
	 ${CMAKE_CURRENT_SOURCE_DIR}
	 ${CMAKE_CURRENT_BINARY_DIR}
   ../include/
)

link_directories( ${CGoGN_ROOT_DIR}/lib/Debug/ ${CGoGN_ROOT_DIR}/lib/Release ${Boost_LIBRARY_DIRS})

QT4_WRAP_UI( VDPMesh_App_ui ${CMAKE_SOURCE_DIR}/src/VDPMesh_App.ui )
QT4_WRAP_CPP( VDPMesh_App_moc ${CMAKE_SOURCE_DIR}/include/VDPMesh_App.h )

file ( GLOB_RECURSE 
   source_files
   ../src/*)

add_executable( VDPMesh_AppD ${source_files}
	 ${VDPMesh_App_moc}
	 ${VDPMesh_App_ui} )

target_link_libraries( VDPMesh_AppD ${CGoGN_LIBS_D} ${COMMON_LIBS})
