if(WIN32 AND NOT CYGWIN)
    set(DEF_INSTALL_CMAKE_DIR CMake)
else()
    set(DEF_INSTALL_CMAKE_DIR lib/cmake/date)
endif()

install( TARGETS sos EXPORT SosConfig )
install( EXPORT SosConfig
	FILE SosConfig.cmake
	NAMESPACE Sos::
	DESTINATION ${DEF_INSTALL_CMAKE_DIR}
)
install( DIRECTORY include/ DESTINATION include/ )

include(CMakePackageConfigHelpers)
write_basic_package_version_file("SosConfigVersion.cmake"
	VERSION ${Sos_VERSION}
	COMPATIBILITY SameMajorVersion
)

install(
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/SosConfigVersion.cmake"
		"LICENSE"
	DESTINATION
		${DEF_INSTALL_CMAKE_DIR}
)



INCLUDE(CPack)
set(CPACK_RESOURCE_FILE_LICENSE LICENSE)
