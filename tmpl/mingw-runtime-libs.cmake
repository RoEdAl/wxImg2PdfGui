#
# MinGW Runtime libraries
#

MACRO(GnuAddImportedSharedLibrary LibName ImpLib DllPath)
	IF(NOT EXISTS ${ImpLib})
		MESSAGE(WARNING "GNU_${LibName} - implib library not found - ${ImpLib}")
	ENDIF()
	IF(NOT EXISTS ${DllPath})
		MESSAGE(WARNING "GNU_${LibName} - dynamic library not found")
	ENDIF()
	
	MESSAGE(VERBOSE "Adding shared library GNU_${LibName}")
	ADD_LIBRARY(GNU_${LibName} SHARED IMPORTED)
	SET_TARGET_PROPERTIES(GNU_${LibName} PROPERTIES
		IMPORTED_NO_SYSTEM ON
		IMPORTED_IMPLIB ${ImpLib}
		IMPORTED_LOCATION ${DllPath}
	)
ENDMACRO()

MACRO(GnuAddImportedSharedLibraryEx LibName ImpLib DllName)
	GnuAddImportedSharedLibrary(${LibName}
		${MINGW_DIR}/x86_64-w64-mingw32/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${ImpLib}${CMAKE_STATIC_LIBRARY_SUFFIX}
		${MINGW_DIR}/x86_64-w64-mingw32/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${DllName}${CMAKE_SHARED_LIBRARY_SUFFIX}
	)
ENDMACRO()

GnuAddImportedSharedLibrary(stdcxx
	${MINGW_DIR}/lib/gcc/x86_64-w64-mingw32/12.1.0/${CMAKE_STATIC_LIBRARY_PREFIX}stdc++${CMAKE_SHARED_LIBRARY_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}
	${MINGW_DIR}/x86_64-w64-mingw32/lib/${CMAKE_SHARED_LIBRARY_PREFIX}stdc++-6${CMAKE_SHARED_LIBRARY_SUFFIX}
)
#GnuAddImportedSharedLibraryEx(stdcxx stdc++ stdc++-6)
GnuAddImportedSharedLibraryEx(gcc gcc_s gcc_s_seh-1)
