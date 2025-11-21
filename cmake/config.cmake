include(${CMAKE_CURRENT_LIST_DIR}/@PROJECT_NAME@-targets.cmake)

if(CMAKE_CXX_STANDARD)
	target_compile_features(quasar::coro INTERFACE cxx_std_${CMAKE_CXX_STANDARD})
else()
	target_compile_features(quasar::coro INTERFACE cxx_std_20)
endif()
