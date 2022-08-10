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

CMAKE_PATH(APPEND IMG2PDF_WORKDIR download OUTPUT_VARIABLE CUE2MKC_DLDIR)

SET(URL_WXWIDGETS "http://github.com/wxWidgets/wxWidgets/releases/download/v${WXWIDGETS_VERSION}")
DownloadPkgSha1(${URL_WXWIDGETS} wxWidgets-${WXWIDGETS_VERSION}-headers.7z 75b5271d1a3f08f32557c7a8ca1782310ee279b4 "wxWidgets headers")
DownloadPkgSha1(${URL_WXWIDGETS} wxWidgets-${WXWIDGETS_VERSION}-docs-html.tar.bz2 9c9caa3b3ce30b7f8b1e30b7a6cc70353b21761d "wxWidgets docs")

IF(INSTALL_MSVC)
	DownloadPkgSha1(${URL_WXWIDGETS} wxMSW-${WXWIDGETS_VERSION}_vc14x_x64_Dev.7z 65ad095d125dea942b9a74339e0476d14a53e6af "wxWidgets dev libraries [MSVC x64]")
	DownloadPkgSha1(${URL_WXWIDGETS} wxMSW-${WXWIDGETS_VERSION}_vc14x_x64_ReleaseDLL.7z c5164b52771aac9973123cfdbeecc5fe8606faba "wxWidgets libraries [MSVC x64]")
	DownloadPkgSha1(${URL_WXWIDGETS} wxMSW-${WXWIDGETS_VERSION}_vc14x_x64_ReleasePDB.7z 61549c7f12ec7c3398160be3445476a812987f9f "wxWidgets PDBs [MSVC x64]")
ENDIF()

IF(INSTALL_MINGW64)
	SET(URL_MINGW64 "http://github.com/niXman/mingw-builds-binaries/releases/download/12.1.0-rt_v10-rev3")
	DownloadPkgSha1(${URL_MINGW64} x86_64-12.1.0-release-win32-seh-rt_v10-rev3.7z 3618baf9bb90c7c4d7b2bb419bc680995531d9cc "MinGW64 runtime")
	DownloadPkgSha1(${URL_WXWIDGETS} wxMSW-${WXWIDGETS_VERSION}_gcc1210_x64_Dev.7z 26a58b3dc1135163921910b69e0ac94f2cbd18a0 "wxWidgets dev libraries [MinGW64]")
	DownloadPkgSha1(${URL_WXWIDGETS} wxMSW-${WXWIDGETS_VERSION}_gcc1210_x64_ReleaseDLL.7z ca15f4ddc1d9ebf68a6a7764eda60a6369285b2d "wxWidgets libraries [MinGW64]")
	
	SET(URL_GCC "http://gcc.gnu.org/onlinedocs/gcc-12.1.0")
	DownloadPkgSha1(${URL_GCC} gcc.pdf 366da27b705374d4f07b508bdc642b6c5d5b4ca7 "GCC documentation - PDF")
	DownloadPkgSha1(${URL_GCC} gcc-html.tar.gz 8fc02ede5b52e8abe4b67a6884e19a62899d809e "GCC documentation - HTML")
ENDIF()

SET(URL_NINJA "http://github.com/ninja-build/ninja/releases/download/v1.11.0")
DownloadPkgSha1(${URL_NINJA} ninja-win.zip 31c7b577d3e5be57eb8acca527f73a484ace7d8c "Ninja builder")

SET(PANDOC_VERSION 2.18)
SET(URL_PANDOC "http://github.com/jgm/pandoc/releases/download/${PANDOC_VERSION}")
DownloadPkgSha1(${URL_PANDOC} pandoc-${PANDOC_VERSION}-windows-x86_64.zip 7db15a14081b52c851a42a5a69a6fd52958ab110 "Pandoc")

# extracting

CMAKE_PATH(APPEND IMG2PDF_WORKDIR wx-widgets OUTPUT_VARIABLE WXWIDGETS_DIR)

CMAKE_PATH(APPEND WXWIDGETS_DIR include msvc wx setup.h OUTPUT_VARIABLE WX_TEST_FILE)
IF(NOT EXISTS ${WX_TEST_FILE})
	MESSAGE(STATUS "[EXR] wxWidgets headers")
	FILE(ARCHIVE_EXTRACT INPUT ${CUE2MKC_DLDIR}/wxWidgets-${WXWIDGETS_VERSION}-headers.7z DESTINATION ${WXWIDGETS_DIR})
ENDIF()

CMAKE_PATH(APPEND WXWIDGETS_DIR docs index.html OUTPUT_VARIABLE WX_TEST_FILE)
IF(NOT EXISTS ${WX_TEST_FILE})
	MESSAGE(STATUS "[EXR] wxWidgets docs")
	FILE(ARCHIVE_EXTRACT INPUT ${CUE2MKC_DLDIR}/wxWidgets-${WXWIDGETS_VERSION}-docs-html.tar.bz2 DESTINATION ${WXWIDGETS_DIR})
	FILE(RENAME ${WXWIDGETS_DIR}/wxWidgets-${WXWIDGETS_VERSION}-docs-html ${WXWIDGETS_DIR}/docs NO_REPLACE)
ENDIF()

IF(INSTALL_MSVC)
	SET(WX_MSVC_DLL_SUFFIX vc14x)
	CMAKE_PATH(APPEND WXWIDGETS_DIR lib ${WX_MSVC_DLL_SUFFIX}_x64_dll wxmsw${WX_VER_COMPACT}ud_core.lib OUTPUT_VARIABLE MSVC_TEST_FILE)
	IF(NOT EXISTS ${MSVC_TEST_FILE})
		MESSAGE(STATUS "[EXR] wxWidgets debug libraries [MSVC x64]")
		FILE(ARCHIVE_EXTRACT INPUT ${CUE2MKC_DLDIR}/wxMSW-${WXWIDGETS_VERSION}_vc14x_x64_Dev.7z DESTINATION ${WXWIDGETS_DIR})
	ENDIF()
	
	CMAKE_PATH(APPEND WXWIDGETS_DIR lib ${WX_MSVC_DLL_SUFFIX}_x64_dll wxmsw${WX_VER_COMPACT}u_core_vc14x_x64.dll OUTPUT_VARIABLE MSVC_TEST_FILE)
	IF(NOT EXISTS ${MSVC_TEST_FILE})
		MESSAGE(STATUS "[EXR] wxWidgets libraries [MSVC x64]")
		FILE(ARCHIVE_EXTRACT INPUT ${CUE2MKC_DLDIR}/wxMSW-${WXWIDGETS_VERSION}_vc14x_x64_ReleaseDLL.7z DESTINATION ${WXWIDGETS_DIR})
	ENDIF()
	
	CMAKE_PATH(APPEND WXWIDGETS_DIR lib ${WX_MSVC_DLL_SUFFIX}_x64_dll wxmsw${WX_VER_COMPACT}u_core_vc14x_x64.pdb OUTPUT_VARIABLE MSVC_TEST_FILE)
	IF(NOT EXISTS ${MSVC_TEST_FILE})
		MESSAGE(STATUS "[EXR] wxWidgets PDBs [MSVC x64]")
		FILE(ARCHIVE_EXTRACT INPUT ${CUE2MKC_DLDIR}/wxMSW-${WXWIDGETS_VERSION}_vc14x_x64_ReleasePDB.7z DESTINATION ${WXWIDGETS_DIR})
	ENDIF()	
ENDIF()

IF(INSTALL_MINGW64)
	CMAKE_PATH(APPEND IMG2PDF_WORKDIR mingw64 OUTPUT_VARIABLE MINGW_DIR)
	IF(NOT EXISTS ${MINGW_DIR})
		MESSAGE(STATUS "[EXR] MinGW64 runtime")
		FILE(ARCHIVE_EXTRACT INPUT ${CUE2MKC_DLDIR}/x86_64-12.1.0-release-win32-seh-rt_v10-rev3.7z DESTINATION ${IMG2PDF_WORKDIR})
	ENDIF()
	
	CMAKE_PATH(APPEND MINGW_DIR doc OUTPUT_VARIABLE MINGW_DOC_DIR)
	IF(NOT EXISTS ${MINGW_DOC_DIR})
		MESSAGE(STATUS "[EXR] GCC documentation - HTML")
		FILE(ARCHIVE_EXTRACT INPUT ${CUE2MKC_DLDIR}/gcc-html.tar.gz DESTINATION ${MINGW_DIR})
		CMAKE_PATH(APPEND MINGW_DIR gcc OUTPUT_VARIABLE MINGW_GCC_DIR)
		FILE(RENAME ${MINGW_GCC_DIR} ${MINGW_DOC_DIR})
	ENDIF()
	
	CMAKE_PATH(APPEND MINGW_DIR gcc.pdf OUTPUT_VARIABLE MINGW_GCC_PDF)
	IF(NOT EXISTS ${MINGW_GCC_PDF})
		MESSAGE(STATUS "[EXR] GCC documentation - PDF")
		FILE(CREATE_LINK ${CUE2MKC_DLDIR}/gcc.pdf ${MINGW_GCC_PDF} COPY_ON_ERROR)
	ENDIF()
	
	SET(WX_MW64_DLL_SUFFIX gcc1210)
	CMAKE_PATH(APPEND WXWIDGETS_DIR lib ${WX_MW64_DLL_SUFFIX}_x64_dll libwxbase${WX_VER_COMPACT}ud.a OUTPUT_VARIABLE MW64_TEST_FILE)
	IF(NOT EXISTS ${MW64_TEST_FILE})
		MESSAGE(STATUS "[EXR] wxWidgets libraries [MinGW64]")
		FILE(ARCHIVE_EXTRACT INPUT ${CUE2MKC_DLDIR}/wxMSW-${WXWIDGETS_VERSION}_gcc1210_x64_Dev.7z DESTINATION ${WXWIDGETS_DIR})
	ENDIF()
	
	MESSAGE(DEBUG "[EXR] wxWidgets libraries #1 [MinGW64]")
	FILE(ARCHIVE_EXTRACT INPUT ${CUE2MKC_DLDIR}/wxMSW-${WXWIDGETS_VERSION}_gcc1210_x64_ReleaseDLL.7z DESTINATION ${WXWIDGETS_DIR})
ENDIF()

CMAKE_PATH(APPEND IMG2PDF_WORKDIR ninja ninja.exe OUTPUT_VARIABLE NINJA_BINARY)
IF(NOT EXISTS ${NINJA_BINARY})
	MESSAGE(STATUS "[EXR] Ninja builder")
	FILE(ARCHIVE_EXTRACT INPUT ${CUE2MKC_DLDIR}/ninja-win.zip DESTINATION ${IMG2PDF_WORKDIR}/ninja)
ENDIF()

CMAKE_PATH(APPEND IMG2PDF_WORKDIR pandoc pandoc.exe OUTPUT_VARIABLE PANDOC_BINARY)
IF(NOT EXISTS ${PANDOC_BINARY})
	MESSAGE(STATUS "[EXR] Pandoc")
	FILE(ARCHIVE_EXTRACT INPUT ${CUE2MKC_DLDIR}/pandoc-${PANDOC_VERSION}-windows-x86_64.zip DESTINATION ${IMG2PDF_WORKDIR})
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
