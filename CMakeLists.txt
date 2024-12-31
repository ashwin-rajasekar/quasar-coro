cmake_minimum_required(VERSION 3.11)

project(quasar-coro VERSION 0.1.0)

add_library(coro INTERFACE)
add_library(quasar::coro ALIAS coro)
target_include_directories(coro INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

if(${PROJECT_IS_TOP_LEVEL})
	include(cmake/install.cmake)
endif()
