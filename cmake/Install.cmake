
install(TARGETS sos EXPORT SosTargets
	INCLUDES DESTINATION include
)
install(EXPORT SosTargets
	FILE SosTargets.cmake
	NAMESPACE Sos::
	DESTINATION cmake
)
install(DIRECTORY include/sos DESTINATION include)

include(CMakePackageConfigHelpers)
write_basic_package_version_file("SosConfigVersion.cmake"
	VERSION ${Sos_VERSION}
	COMPATIBILITY SameMajorVersion
)

install(FILES "cmake/SosConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/SosConfigVersion.cmake"
	DESTINATION cmake
)

INCLUDE(CPack)