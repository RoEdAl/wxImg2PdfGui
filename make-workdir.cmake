﻿#
# make-workdir.cmake
#

CMAKE_MINIMUM_REQUIRED(VERSION 3.23)
INCLUDE(ProcessorCount)
INCLUDE(tmpl/utils.cmake)

# currently hardcoded configuration

ProcessorCount(PARALLEL_LEVEL)
SET(WXWIDGETS_VERSION "3.2.0")
SET(WX_VER_COMPACT 32)
SET(INSTALL_MSVC ON)
SET(INSTALL_MINGW64 ON)
SET(INSTALL_PANDOC ON)
SET(MSVC_GENERATOR "Visual Studio 17 2022")
SET(MSVC_TOOLSET "v143,host=x64")

SET(IMG2PDF_WORKDIR ${CMAKE_SOURCE_DIR}/..)
CMAKE_PATH(ABSOLUTE_PATH IMG2PDF_WORKDIR NORMALIZE)
RemoveLastChar(${IMG2PDF_WORKDIR})

# show configuration

MESSAGE(STATUS "[CFG] wxWidgets version: ${WXWIDGETS_VERSION}")
MESSAGE(STATUS "[CFG] Workdir: ${IMG2PDF_WORKDIR}")
MESSAGE(STATUS "[CFG] Install MSVC: ${INSTALL_MSVC}")
MESSAGE(STATUS "[CFG] Install MinGW64: ${INSTALL_MINGW64}")
MESSAGE(STATUS "[CFG] Jobs: ${PARALLEL_LEVEL}")

# downloading

CMAKE_PATH(APPEND IMG2PDF_WORKDIR download OUTPUT_VARIABLE IMG2PDF_DLDIR)

SET(URL_WXWIDGETS "https://github.com/wxWidgets/wxWidgets/releases/download/v${WXWIDGETS_VERSION}")
DownloadPkgSha1(${URL_WXWIDGETS} wxWidgets-${WXWIDGETS_VERSION}-docs-html.tar.bz2 9c9caa3b3ce30b7f8b1e30b7a6cc70353b21761d "wxWidgets docs")

SET(URL_WXWIDGETS "https://github.com/RoEdAl/wxwidgets-builds/releases/download/v${WXWIDGETS_VERSION}-build4")
IF(INSTALL_MSVC)
	DownloadPkgSha1(${URL_WXWIDGETS} wxWidgets-vc-dbg-${WXWIDGETS_VERSION}-win64.7z 0685eaf45e983fe634c2c45768dd562d6d97f2d9 "wxWidgets dev libraries [MSVC x64]")
	DownloadPkgSha1(${URL_WXWIDGETS} wxWidgets-vc-${WXWIDGETS_VERSION}-win64.7z 90303347291babffe1cd8c7434d2c8df99d1123e "wxWidgets libraries [MSVC x64]")
ENDIF()

IF(INSTALL_MINGW64)
	DownloadPkgSha1(${URL_WXWIDGETS} wxWidgets-gcc-dbg-${WXWIDGETS_VERSION}-win64.7z c85cd8597441b093127560f6fe1bbf034e3c0362 "wxWidgets dev libraries [MinGW64]")
	DownloadPkgSha1(${URL_WXWIDGETS} wxWidgets-gcc-${WXWIDGETS_VERSION}-win64.7z 0f6f43a1cd5bf1752b4a764b412b8eef0fae340b "wxWidgets libraries [MinGW64]")
	
	SET(URL_MINGW64 "https://github.com/RoEdAl/ucrt-mingw-builds/releases/download/v12.2.0-rt10-ucrt2")
	DownloadPkgSha1(${URL_MINGW64} x86_64-12.2.0-release-win32-seh-rt_v10-rev0.7z 5de70c97911ba10a591cbf6290e75f336ab0f635 "MinGW64 runtime")
		
	SET(URL_GCC "http://gcc.gnu.org/onlinedocs/gcc-12.2.0")
	DownloadPkgSha1(${URL_GCC} gcc.pdf becbd022de78a4f818d53d3229a19f9edb03f88e "GCC documentation - PDF")
	DownloadPkgSha1(${URL_GCC} gcc-html.tar.gz e3ef867a3803961b01fbd57e7c5d19bc36757573 "GCC documentation - HTML")
ENDIF()

SET(URL_NINJA "http://github.com/ninja-build/ninja/releases/download/v1.11.0")
DownloadPkgSha1(${URL_NINJA} ninja-win.zip 31c7b577d3e5be57eb8acca527f73a484ace7d8c "Ninja builder")

SET(PANDOC_VERSION 2.19)
SET(URL_PANDOC "http://github.com/jgm/pandoc/releases/download/${PANDOC_VERSION}")
DownloadPkgSha1(${URL_PANDOC} pandoc-${PANDOC_VERSION}-windows-x86_64.zip 7a581c2efd02252e7eaa830cc813f3147c01f691 Pandoc)

# extracting

FUNCTION(ExtractWxArchive ar_name)
	SET(ARCHIVE_NAME wxWidgets-${ar_name}-${WXWIDGETS_VERSION}-win64)
	FILE(ARCHIVE_EXTRACT INPUT ${IMG2PDF_DLDIR}/${ARCHIVE_NAME}.7z DESTINATION ${WXWIDGETS_DIR})
	FILE(COPY ${WXWIDGETS_DIR}/${ARCHIVE_NAME}/include DESTINATION ${WXWIDGETS_DIR})
	FILE(COPY ${WXWIDGETS_DIR}/${ARCHIVE_NAME}/lib DESTINATION ${WXWIDGETS_DIR})
	FILE(REMOVE_RECURSE ${WXWIDGETS_DIR}/${ARCHIVE_NAME})
ENDFUNCTION()

CMAKE_PATH(APPEND IMG2PDF_WORKDIR wx-widgets OUTPUT_VARIABLE WXWIDGETS_DIR)

CMAKE_PATH(APPEND WXWIDGETS_DIR docs index.html OUTPUT_VARIABLE WX_TEST_FILE)
IF(NOT EXISTS ${WX_TEST_FILE})
	MESSAGE(STATUS "[EXTR] wxWidgets docs")
	FILE(ARCHIVE_EXTRACT INPUT ${IMG2PDF_DLDIR}/wxWidgets-${WXWIDGETS_VERSION}-docs-html.tar.bz2 DESTINATION ${WXWIDGETS_DIR})
	FILE(RENAME ${WXWIDGETS_DIR}/wxWidgets-${WXWIDGETS_VERSION}-docs-html ${WXWIDGETS_DIR}/docs NO_REPLACE)
ENDIF()

IF(INSTALL_MSVC)
	SET(WX_MSVC_DLL_SUFFIX vc)
	CMAKE_PATH(APPEND WXWIDGETS_DIR lib ${WX_MSVC_DLL_SUFFIX}_x64_dll wxmsw${WX_VER_COMPACT}ud_core.lib OUTPUT_VARIABLE MSVC_TEST_FILE)
	IF(NOT EXISTS ${MSVC_TEST_FILE})
		MESSAGE(STATUS "[EXTR] wxWidgets dev libraries [MSVC x64]")
		ExtractWxArchive(vc-dbg)
	ENDIF()
	
	CMAKE_PATH(APPEND WXWIDGETS_DIR lib ${WX_MSVC_DLL_SUFFIX}_x64_dll wxmsw${WX_VER_COMPACT}u_core.lib OUTPUT_VARIABLE MSVC_TEST_FILE)
	IF(NOT EXISTS ${MSVC_TEST_FILE})
		MESSAGE(STATUS "[EXTR] wxWidgets libraries [MSVC x64]")
		ExtractWxArchive(vc)
	ENDIF()
ENDIF()

IF(INSTALL_MINGW64)
	CMAKE_PATH(APPEND IMG2PDF_WORKDIR mingw64 OUTPUT_VARIABLE MINGW_DIR)
	IF(NOT EXISTS ${MINGW_DIR})
		MESSAGE(STATUS "[EXTR] MinGW64 runtime")
		FILE(ARCHIVE_EXTRACT INPUT ${IMG2PDF_DLDIR}/x86_64-12.2.0-release-win32-seh-rt_v10-rev0.7z DESTINATION ${IMG2PDF_WORKDIR})
	ENDIF()
	
	CMAKE_PATH(APPEND MINGW_DIR doc OUTPUT_VARIABLE MINGW_DOC_DIR)
	IF(NOT EXISTS ${MINGW_DOC_DIR})
		MESSAGE(STATUS "[EXTR] GCC documentation - HTML")
		FILE(ARCHIVE_EXTRACT INPUT ${IMG2PDF_DLDIR}/gcc-html.tar.gz DESTINATION ${MINGW_DIR})
		CMAKE_PATH(APPEND MINGW_DIR gcc OUTPUT_VARIABLE MINGW_GCC_DIR)
		FILE(RENAME ${MINGW_GCC_DIR} ${MINGW_DOC_DIR})
	ENDIF()
	
	CMAKE_PATH(APPEND MINGW_DIR gcc.pdf OUTPUT_VARIABLE MINGW_GCC_PDF)
	IF(NOT EXISTS ${MINGW_GCC_PDF})
		MESSAGE(STATUS "[EXTR] GCC documentation - PDF")
		FILE(CREATE_LINK ${IMG2PDF_DLDIR}/gcc.pdf ${MINGW_GCC_PDF} COPY_ON_ERROR)
	ENDIF()
	
	SET(WX_MW64_DLL_SUFFIX gcc)
	CMAKE_PATH(APPEND WXWIDGETS_DIR lib ${WX_MW64_DLL_SUFFIX}_x64_dll libwxbase${WX_VER_COMPACT}ud.a OUTPUT_VARIABLE MW64_TEST_FILE)
	IF(NOT EXISTS ${MW64_TEST_FILE})
		MESSAGE(STATUS "[EXTR] wxWidgets dev libraries [MinGW64]")
		ExtractWxArchive(gcc-dbg)
	ENDIF()
	
	CMAKE_PATH(APPEND WXWIDGETS_DIR lib ${WX_MW64_DLL_SUFFIX}_x64_dll libwxbase${WX_VER_COMPACT}u.a OUTPUT_VARIABLE MW64_TEST_FILE)
	IF(NOT EXISTS ${MW64_TEST_FILE})
		MESSAGE(STATUS "[EXTR] wxWidgets libraries [MinGW64]")
		ExtractWxArchive(gcc)
	ENDIF()
ENDIF()

CMAKE_PATH(APPEND IMG2PDF_WORKDIR ninja ninja.exe OUTPUT_VARIABLE NINJA_BINARY)
IF(NOT EXISTS ${NINJA_BINARY})
	MESSAGE(STATUS "[EXTR] Ninja builder")
	FILE(ARCHIVE_EXTRACT INPUT ${IMG2PDF_DLDIR}/ninja-win.zip DESTINATION ${IMG2PDF_WORKDIR}/ninja)
ENDIF()

CMAKE_PATH(APPEND IMG2PDF_WORKDIR pandoc pandoc.exe OUTPUT_VARIABLE PANDOC_BINARY)
IF(NOT EXISTS ${PANDOC_BINARY})
	MESSAGE(STATUS "[EXTR] Pandoc")
	FILE(ARCHIVE_EXTRACT INPUT ${IMG2PDF_DLDIR}/pandoc-${PANDOC_VERSION}-windows-x86_64.zip DESTINATION ${IMG2PDF_WORKDIR})
	FILE(RENAME ${IMG2PDF_WORKDIR}/pandoc-${PANDOC_VERSION} ${IMG2PDF_WORKDIR}/pandoc)
ENDIF()

# MINGW toolchain

IF(INSTALL_MINGW64)
	CMAKE_PATH(APPEND MINGW_DIR mingw-toolchain.cmake OUTPUT_VARIABLE MW64_TOOLCHAIN_FILE)
	IF(NOT EXISTS ${MW64_TOOLCHAIN_FILE})
		MESSAGE(STATUS "[CFGF] MinGW toolchain: ${MW64_TOOLCHAIN_FILE}")
		CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/tmpl/mingw-toolchain.cmake ${MW64_TOOLCHAIN_FILE} NO_SOURCE_PERMISSIONS @ONLY)
	ENDIF()
	
	CMAKE_PATH(APPEND MINGW_DIR mingw-config.cmake OUTPUT_VARIABLE MW64_CFG_FILE)
	IF(NOT EXISTS ${MW64_CFG_FILE})
		MESSAGE(STATUS "[CFGF] MinGW config: ${MW64_CFG_FILE}")
		CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/tmpl/mingw-runtime-libs.cmake ${MW64_CFG_FILE} NO_SOURCE_PERMISSIONS @ONLY)
	ENDIF()	
ENDIF()

# wxWidgets configuration

CMAKE_PATH(APPEND WXWIDGETS_DIR wx-widgets.cmake OUTPUT_VARIABLE MSVC_CFG_USE_FILE)
IF(NOT EXISTS ${MSVC_CFG_USE_FILE})
	MESSAGE(STATUS "[CFGF] MSVC ${MSVC_CFG_USE_FILE}")
	FILE(COPY ${CMAKE_SOURCE_DIR}/tmpl/wx-widgets.cmake DESTINATION ${WXWIDGETS_DIR} NO_SOURCE_PERMISSIONS)
ENDIF()

IF(INSTALL_MSVC)
	CMAKE_PATH(APPEND WXWIDGETS_DIR msvc-config.cmake OUTPUT_VARIABLE MSVC_CFG_USE_FILE)
	IF(NOT EXISTS ${MSVC_CFG_USE_FILE})
		MESSAGE(STATUS "[CFGF] MSVC ${MSVC_CFG_USE_FILE}")
		CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/tmpl/msvc-config.cmake ${MSVC_CFG_USE_FILE} NO_SOURCE_PERMISSIONS @ONLY)
	ENDIF()
ENDIF()

IF(INSTALL_MINGW64)
	CMAKE_PATH(APPEND WXWIDGETS_DIR mingw64-config.cmake OUTPUT_VARIABLE MW64_CFG_USE_FILE)
	IF(NOT EXISTS ${MW64_CFG_USE_FILE})
		MESSAGE(STATUS "[CFGF] MinGW64 ${MW64_CFG_USE_FILE}")
		CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/tmpl/mingw64-config.cmake ${MW64_CFG_USE_FILE} NO_SOURCE_PERMISSIONS @ONLY)
	ENDIF()
ENDIF()

# presets file: CMakeUserPresets.json

CMAKE_PATH(APPEND CMAKE_SOURCE_DIR CMakeUserPresets.json OUTPUT_VARIABLE PRESETS_FILE)
IF(NOT EXISTS ${PRESETS_FILE})
	MESSAGE(STATUS "[CFGF] Preset ${PRESETS_FILE}")
	CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/tmpl/CMakePresets.json ${PRESETS_FILE} NO_SOURCE_PERMISSIONS @ONLY)
ENDIF()
