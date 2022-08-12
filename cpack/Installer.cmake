#
# Installer.cmake
#

# installer code - create pkg-desc.json

STRING(CONFIGURE [=[
SET(_PKG_DESC "{}")
STRING(JSON _PKG_DESC SET "${_PKG_DESC}" buildConfig "\"$<CONFIG>\"")
STRING(JSON _PKG_DESC SET "${_PKG_DESC}" componentName "\"img2pdf\"")
STRING(JSON _PKG_DESC SET "${_PKG_DESC}" packageName "\"wximg2pdf\"")
CMAKE_PATH(APPEND CMAKE_INSTALL_PREFIX pkg-desc.json OUTPUT_VARIABLE _PKG_DESC_PATH)
FILE(WRITE "${_PKG_DESC_PATH}" "${_PKG_DESC}")
]=] INSTALL_SCRIPT @ONLY)
INSTALL(CODE "${INSTALL_SCRIPT}" COMPONENT img2pdf EXCLUDE_FROM_ALL)

# install & strip DLLs
IF(MINGW)
	FIND_PROGRAM(STRIP_EXE "strip")
	IF(NOT STRIP_EXE)
		MESSAGE(FATAL_ERROR "Strip utility not found")
	ENDIF()
	STRING(CONFIGURE [=[
FOREACH(LIBPATH $<TARGET_RUNTIME_DLLS:wxImg2PdfGui>)
	CMAKE_PATH(GET LIBPATH FILENAME LIBNAME)
	EXECUTE_PROCESS(COMMAND "@STRIP_EXE@" --preserve-dates $<IF:$<CONFIG:Debug,RelWithDebInfo>,--only-keep-debug,--strip-unneeded> "${LIBPATH}" -o ${LIBNAME} ENCODING UTF-8 WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/img2pdf COMMAND_ERROR_IS_FATAL ANY)
ENDFOREACH()
]=] INSTALL_SCRIPT @ONLY)
	INSTALL(CODE "${INSTALL_SCRIPT}" COMPONENT img2pdf EXCLUDE_FROM_ALL)
ENDIF()

# CPack

SET(CPACK_PACKAGE_NAME wximg2pdf)
SET(CPACK_PACKAGE_VENDOR "Edmunt Pienkowsky")
SET(CPACK_STRIP_FILES ON)
LIST(APPEND CPACK_GENERATOR External)
LIST(APPEND CPACK_SOURCE_GENERATOR ZIP)
LIST(APPEND CPACK_SOURCE_IGNORE_FILES "/\.editorconfig")
LIST(APPEND CPACK_SOURCE_IGNORE_FILES "/\.gitignore")
LIST(APPEND CPACK_SOURCE_IGNORE_FILES "/\.git/")
LIST(APPEND CPACK_SOURCE_IGNORE_FILES "/doc/")
LIST(APPEND CPACK_SOURCE_IGNORE_FILES "/cue/")
LIST(APPEND CPACK_SOURCE_IGNORE_FILES "/CMakeUserPresets.*\.json")
SET(CPACK_EXTERNAL_ENABLE_STAGING ON)
SET(CPACK_PACKAGE_CHECKSUM SHA1)
SET(CPACK_COMPONENTS_ALL img2pdf)
SET(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${CMAKE_PROJECT_NAME};img2pdf;/")
CMAKE_PATH(APPEND CMAKE_SOURCE_DIR cpack InnoSetup.cmake OUTPUT_VARIABLE CPACK_EXTERNAL_PACKAGE_SCRIPT)

INCLUDE(CPack)
