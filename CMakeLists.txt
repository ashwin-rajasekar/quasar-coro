cmake_minimum_required(VERSION 3.14)

project(quasar-coro VERSION 0.1.2)

add_library(coro INTERFACE)
add_library(quasar::coro ALIAS coro)
target_include_directories(coro INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

set(QUASAR_INSTALL_TARGETS coro)
set(QUASAR_ARCH_INDEPENDENT true)
include(cmake/install.cmake)
