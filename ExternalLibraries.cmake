#
# ExternalLibraries.cmake
#

CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

SET(FETCHCONTENT_QUIET ON)
SET(FETCHCONTENT_TRY_FIND_PACKAGE_MODE NEVER)
SET(FETCHCONTENT_UPDATES_DISCONNECTED ON)
INCLUDE(FetchContent)

FetchContent_Declare(json
	URL http://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz
	URL_HASH SHA256=344BE97B757A36C5B180F1C8162F6C5F6EBD760B117F6E64B77866E97B217280
	DOWNLOAD_EXTRACT_TIMESTAMP ON
)

FetchContent_MakeAvailable(json)
