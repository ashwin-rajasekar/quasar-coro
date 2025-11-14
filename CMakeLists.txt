cmake_minimum_required(VERSION 3.28)

project(quasar-coro VERSION 0.1.3)

add_library(coro INTERFACE)
target_sources(coro INTERFACE
	FILE_SET HEADERS
	BASE_DIRS include
	FILES include/quasar/coro
)

add_library(quasar::coro ALIAS coro)
include(cmake/install.cmake)
