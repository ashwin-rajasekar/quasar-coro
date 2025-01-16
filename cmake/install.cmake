include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(${QUASAR_ARCH_INDEPENDENT})
	set(QUASAR_ARCH_INDEPENDENT ARCH_INDEPENDENT)
else()
	set(QUASAR_ARCH_INDEPENDENT)
endif()

configure_package_config_file(
	${PROJECT_SOURCE_DIR}/cmake/config.cmake
	${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
	${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMinorVersion
	${QUASAR_ARCH_INDEPENDENT}
)

install(
	FILES
		${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
		${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
	DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}
)

install(
	FILES
		${PROJECT_SOURCE_DIR}/COPYING
		${PROJECT_SOURCE_DIR}/COPYING.LESSER
	DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

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

install(
	EXPORT ${PROJECT_NAME}-targets
	NAMESPACE quasar::
	DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}
)

install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/include/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
