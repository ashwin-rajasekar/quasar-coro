cmake_minimum_required(VERSION 3.28)

project(quasar-coro VERSION 0.2.0)

if(QUASAR_CORO_MODULES)
	add_library(coro)
	set(visibility PUBLIC)

	target_compile_features(coro PUBLIC cxx_std_20)
	target_compile_definitions(coro PRIVATE QUASAR_CORO_MODULES)
	set_target_properties(coro PROPERTIES LINKER_LANGUAGE CXX)
	target_sources(coro PUBLIC
		FILE_SET CXX_MODULES
		BASE_DIRS src
		FILES src/quasar.coro.cpp
	)
else()
	add_library(coro INTERFACE)
	set(visibility INTERFACE)
endif()

add_library(quasar::coro ALIAS coro)

target_sources(coro ${visibility}
	FILE_SET HEADERS
	BASE_DIRS include
	FILES include/quasar/coro
)

if(${CMAKE_CXX_STANDARD} LESS 23)
	target_compile_definitions(coro ${visibility} QUASAR_CORO_NO_EXPLICIT_OBJECT)
endif()

include(CTest)
if(BUILD_TESTING)
	add_subdirectory(test)
endif()

include(cmake/install.cmake)
