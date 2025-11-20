cmake_minimum_required(VERSION 3.28)

project(quasar-coro VERSION 0.2.0)

if(QUASAR_CORO_MODULES)
	add_library(coro)
	target_sources(coro PUBLIC
		FILE_SET HEADERS
		BASE_DIRS include
		FILES include/quasar/coro

		FILE_SET CXX_MODULES
		BASE_DIRS src
		FILES src/quasar.coro.cpp
	)
else()
	add_library(coro INTERFACE)
	target_sources(coro INTERFACE
		FILE_SET HEADERS
		BASE_DIRS include
		FILES include/quasar/coro
	)
endif()

add_library(quasar::coro ALIAS coro)

include(CTest)
if(BUILD_TESTING)
	add_subdirectory(test)
endif()

include(cmake/install.cmake)
