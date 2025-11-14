cmake_minimum_required(VERSION 3.28)

project(quasar-coro VERSION 0.1.3)

add_library(coro INTERFACE)
add_library(quasar::coro ALIAS coro)

target_sources(coro INTERFACE
	FILE_SET HEADERS
	BASE_DIRS include
	FILES include/quasar/coro
)

if(QUASAR_CORO_MODULES)
	add_library(coro-module)
	add_library(quasar::coro-module ALIAS coro-module)

	target_link_libraries(coro-module PUBLIC coro)
	target_compile_features(coro-module PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
	target_compile_definitions(coro-module PUBLIC QUASAR_CORO_MODULES)
	set_target_properties(coro-module PROPERTIES LINKER_LANGUAGE CXX)
	target_sources(coro-module PUBLIC
		FILE_SET CXX_MODULES
		BASE_DIRS src
		FILES src/quasar.coro.cpp
	)
endif()

include(CTest)
if(BUILD_TESTING)
	add_subdirectory(test)
endif()

include(cmake/install.cmake)
