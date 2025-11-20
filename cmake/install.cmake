include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Generate Package Version Config
write_basic_package_version_file(
	${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
	COMPATIBILITY SameMinorVersion
	ARCH_INDEPENDENT
)

# Install Package Config & Version Config
install(
	FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
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
	install(
		TARGETS coro
		EXPORT ${PROJECT_NAME}-config
		FILE_SET HEADERS
		FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)
else()
	install(
		TARGETS coro
		EXPORT ${PROJECT_NAME}-config
		FILE_SET HEADERS
	)
endif()

# Install Targets & Set Namespacing
install(
	EXPORT ${PROJECT_NAME}-config
	NAMESPACE quasar::
	DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}
)
