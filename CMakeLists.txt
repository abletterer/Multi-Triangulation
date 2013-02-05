cmake_minimum_required(VERSION 2.8)

project( VDPMesh_App )

SET(CGoGN_ROOT_DIR ${CMAKE_SOURCE_DIR}/../../CGoGN CACHE STRING "CGoGN root dir")
include(${CGoGN_ROOT_DIR}/apps_cmake.txt)

add_subdirectory(${CMAKE_SOURCE_DIR}/Release Release)
IF (NOT WIN32)
	 add_subdirectory(${CMAKE_SOURCE_DIR}/Debug Debug)
ENDIF (NOT WIN32)
