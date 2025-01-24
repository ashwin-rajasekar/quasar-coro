include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Set Architecture-Independence flag as appropriate
if(${QUASAR_ARCH_INDEPENDENT})
	set(QUASAR_ARCH_INDEPENDENT ARCH_INDEPENDENT)
else()
	set(QUASAR_ARCH_INDEPENDENT)
endif()

# Generate Package Config
configure_package_config_file(
	${PROJECT_SOURCE_DIR}/cmake/config.cmake
	${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}
)

# Generate Package Version Config
write_basic_package_version_file(
	${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
	COMPATIBILITY SameMinorVersion
	${QUASAR_ARCH_INDEPENDENT}
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
	DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

# Create Installation Target Set
install(
	TARGETS ${QUASAR_INSTALL_TARGETS}
	EXPORT ${PROJECT_NAME}-targets
	RUNTIME_DEPENDENCIES
	BUNDLE        DESTINATION ${CMAKE_INSTALL_BINDIR}
	RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
	FRAMEWORK     DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	INCLUDES      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install Targets & Set Namespacing
install(
	EXPORT ${PROJECT_NAME}-targets
	NAMESPACE quasar::
	DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}
)

# Install Interface Headers
install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/include/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
