include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Generate Package Config
configure_package_config_file(
	${CMAKE_CURRENT_LIST_DIR}/config.cmake
	${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
	INSTALL_DESTINATION . # unused but required
)

# Generate Package Version Config
write_basic_package_version_file(
	${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
	COMPATIBILITY SameMinorVersion
	ARCH_INDEPENDENT
)

# Install Package Config & Version Config
install(
	FILES
		${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
		${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
	DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}
)

# Install Licenses
install(
	FILES
		${PROJECT_SOURCE_DIR}/COPYING
		${PROJECT_SOURCE_DIR}/COPYING.LESSER
	TYPE DOC
)

# Create Installation Target Set
if(QUASAR_CORO_MODULES)
	set(QUASR_CORO_MODULES_ARTIFACTS FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

install(
	TARGETS coro
	EXPORT ${PROJECT_NAME}-targets
	FILE_SET HEADERS
	${QUASR_CORO_MODULES_ARTIFACTS}
)

# Install Targets & Set Namespacing
install(
	EXPORT ${PROJECT_NAME}-targets
	NAMESPACE quasar::
	DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}
)
