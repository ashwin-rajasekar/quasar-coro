# Validate input tag
set(VERSION_REGEX "[0-9]+\\.[0-9]+\\.[0-9]+")
string(REGEX MATCH "(refs/tags/)?v(${VERSION_REGEX})" MATCH ${VERSION})
if(${MATCH} STREQUAL "")
	message(FATAL_ERROR "'${VERSION}' is not a valid version tag; expected vX.Y.Z")
endif()
set(NEW_VERSION ${CMAKE_MATCH_2})
message(STATUS "new version is ${NEW_VERSION}")

if(${CMAKELISTS_DIRTY})
	message(FATAL_ERROR "CMakeLists.txt has uncommitted changes!")
endif()

# Generate updated CMakeLists.txt
file(READ CMakeLists.txt CMAKELISTS_IN)
string(REGEX REPLACE
	"(project\\s*\\(.* VERSION) ${VERSION_REGEX}"
	"\\1 ${NEW_VERSION}"
	CMAKELISTS_OUT
	"${CMAKELISTS_IN}"
)

# Update CMakeLists.txt & commit
file(WRITE CMakeLists.txt ${CMAKELISTS_OUT})
