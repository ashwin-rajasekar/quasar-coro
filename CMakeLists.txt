cmake_minimum_required(VERSION 3.11)

project(quasar-coro VERSION 0.0.1)

add_library(quasar-coro INTERFACE)
target_include_directories(quasar-coro INTERFACE include)

add_library(quasar::coro ALIAS quasar-coro)
