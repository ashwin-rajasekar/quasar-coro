include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/config.cmake
    ${PROJECT_BINARY_DIR}/quasar-coro-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
	${PROJECT_BINARY_DIR}/quasar-coro-config-version.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMinorVersion
)

install(
	FILES
		${PROJECT_BINARY_DIR}/quasar-coro-config.cmake
		${PROJECT_BINARY_DIR}/quasar-coro-config-version.cmake
	DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}
)

install(
	TARGETS coro
	EXPORT quasar-coro-targets
    BUNDLE        DESTINATION ${CMAKE_INSTALL_BINDIR}     COMPONENT Runtime
    RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}     COMPONENT Runtime
	LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}     COMPONENT Runtime
    ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}     COMPONENT Development
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT Development
)

install(
	EXPORT quasar-coro-targets
	NAMESPACE quasar::
	DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}
)

install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/include/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
