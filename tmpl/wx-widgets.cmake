#
# wxWidget - imported libraries
#

CMAKE_MINIMUM_REQUIRED(VERSION 3.23)

SET(WX_PREFIX_LIB_STATIC ${CMAKE_STATIC_LIBRARY_PREFIX}wx)
SET(WX_PREFIX_LIB_DYNAMIC ${CMAKE_DYNAMIC_LIBRARY_PREFIX}wx)
SET(WX_SUFFIX_LIB_DYNAMIC _${WXWIDGETS_DLL_SUFFIX}_x64${CMAKE_SHARED_LIBRARY_SUFFIX})

FUNCTION(WxConfigureStaticTarget Tgt)
	TARGET_INCLUDE_DIRECTORIES(WX_${Tgt} INTERFACE
		${wxWidgets_LIB_DIR}/$<IF:$<CONFIG:Debug,RelWithDebInfo>,mswud,mswu>
		${wxWidgets_ROOT_DIR}/include
	)
ENDFUNCTION()

FUNCTION(WxConfigureDynamicTarget Tgt)
	TARGET_COMPILE_DEFINITIONS(WX_${Tgt} INTERFACE WXUSINGDLL)
	TARGET_INCLUDE_DIRECTORIES(WX_${Tgt} INTERFACE
		${wxWidgets_LIB_DIR}/$<IF:$<CONFIG:Debug,RelWithDebInfo>,mswud,mswu>
		${wxWidgets_ROOT_DIR}/include
	)
ENDFUNCTION()

MACRO(WxAddImportedUnversionedStaticLibrary LibName LibPath LibPathDbg)
	IF(NOT EXISTS ${LibPath})
		MESSAGE(WARNING "WX_${LibName} - static library not found")
	ENDIF()
	IF(NOT EXISTS ${LibPathDbg})
		MESSAGE(WARNING "WX_${LibName} - debug static library not found")
	ENDIF()
	
	MESSAGE(VERBOSE "Adding static library WX_${LibName}")
	ADD_LIBRARY(WX_${LibName} STATIC IMPORTED)
	SET_TARGET_PROPERTIES(WX_${LibName} PROPERTIES
		IMPORTED_NO_SYSTEM ON
		IMPORTED_LOCATION ${LibPath}
		IMPORTED_LOCATION_DEBUG ${LibPathDbg}
	)
	WxConfigureStaticTarget(${LibName})
ENDMACRO()

MACRO(WxAddImportedUnversionedStaticLibraryEx LibName)
	WxAddImportedUnversionedStaticLibrary(${LibName}
		${wxWidgets_LIB_DIR}/${WX_PREFIX_LIB_STATIC}${LibName}${CMAKE_STATIC_LIBRARY_SUFFIX}
		${wxWidgets_LIB_DIR}/${WX_PREFIX_LIB_STATIC}${LibName}d${CMAKE_STATIC_LIBRARY_SUFFIX}
	)
ENDMACRO()

MACRO(WxAddImportedSharedLibrary LibName ImpLib ImpLibDbg DllPath DllPathDbg)
	IF(NOT EXISTS ${ImpLib})
		MESSAGE(WARNING "WX_${LibName} - implib not found")
	ENDIF()
	IF(NOT EXISTS ${ImpLibDbg})
		MESSAGE(WARNING "WX_${LibName} - debug implib not found")
	ENDIF()
	IF(NOT EXISTS ${DllPath})
		MESSAGE(WARNING "WX_${LibName} - dynamic library not found")
	ENDIF()
	IF(NOT EXISTS ${DllPathDbg})
		MESSAGE(WARNING "WX_${LibName} - debug dynamic library not found")
	ENDIF()
	
	MESSAGE(VERBOSE "Adding shared library WX_${LibName}")
	ADD_LIBRARY(WX_${LibName} SHARED IMPORTED)
	SET_TARGET_PROPERTIES(WX_${LibName} PROPERTIES
		IMPORTED_NO_SYSTEM ON
		IMPORTED_IMPLIB ${ImpLib}
		IMPORTED_IMPLIB_DEBUG ${ImpLibDbg}
		IMPORTED_LOCATION ${DllPath}
		IMPORTED_LOCATION_DEBUG ${DllPathDbg}
	)
	WxConfigureDynamicTarget(${LibName})
ENDMACRO()

MACRO(WxAddImportedSharedLibraryEx LibName ImpLib ImpLibDbg DllPath DllPathDbg)
	WxAddImportedSharedLibrary(${LibName}
		${wxWidgets_LIB_DIR}/${WX_PREFIX_LIB_STATIC}${ImpLib}${CMAKE_STATIC_LIBRARY_SUFFIX}
		${wxWidgets_LIB_DIR}/${WX_PREFIX_LIB_STATIC}${ImpLibDbg}${CMAKE_STATIC_LIBRARY_SUFFIX}
		${wxWidgets_LIB_DIR}/${WX_PREFIX_LIB_DYNAMIC}${DllPath}${WX_SUFFIX_LIB_DYNAMIC}
		${wxWidgets_LIB_DIR}/${WX_PREFIX_LIB_DYNAMIC}${DllPathDbg}${WX_SUFFIX_LIB_DYNAMIC}
	)
ENDMACRO()

MACRO(WxAddImportedVersionedSharedLibrary LibName)
	WxAddImportedSharedLibraryEx(${LibName}
		${LibName}${WXWIDGETS_VER_COMPACT}u
		${LibName}${WXWIDGETS_VER_COMPACT}ud
		${LibName}${WXWIDGETS_VER_COMPACT}u
		${LibName}${WXWIDGETS_VER_COMPACT}ud
	)
ENDMACRO()

MACRO(WxAddImportedCategorizedSharedLibrary LibName LibCategory)
	WxAddImportedSharedLibraryEx(${LibName}
		${LibCategory}${WXWIDGETS_VER_COMPACT}u_${LibName}
		${LibCategory}${WXWIDGETS_VER_COMPACT}ud_${LibName}
		${LibCategory}${WXWIDGETS_VER_COMPACT}u_${LibName}
		${LibCategory}${WXWIDGETS_VER_COMPACT}ud_${LibName}
	)
ENDMACRO()

MACRO(WxAddImportedBaseSharedLibrary LibName)
	WxAddImportedCategorizedSharedLibrary(${LibName} base)
ENDMACRO()

MACRO(WxAddImportedMswSharedLibrary LibName)
	WxAddImportedCategorizedSharedLibrary(${LibName} msw)
ENDMACRO()

# ---------------------------------------------------------------------
# static libs
# ---------------------------------------------------------------------

SET(WX_LIBS_STATIC jpeg expat png regexu scintilla tiff zlib)
FOREACH(L IN LISTS WX_LIBS_STATIC)
	WxAddImportedUnversionedStaticLibraryEx(${L})
ENDFOREACH()

# ---------------------------------------------------------------------
# dynamic libs - base
# ---------------------------------------------------------------------

SET(WX_LIBS_BASE net xml)

WxAddImportedVersionedSharedLibrary(base)
FOREACH(L IN LISTS WX_LIBS_BASE)
	WxAddImportedBaseSharedLibrary(${L})
ENDFOREACH()

# ---------------------------------------------------------------------
# dynamic libs - msw
# ---------------------------------------------------------------------

SET(WX_LIBS_MSW adv core gl html media propgrid ribbon richtext stc webview xrc)
FOREACH(L IN LISTS WX_LIBS_MSW)
	WxAddImportedMswSharedLibrary(${L})
ENDFOREACH()

# ---------------------------------------------------------------------
# cleanup
# ---------------------------------------------------------------------

UNSET(WX_PREFIX_LIB_STATIC)
UNSET(WX_PREFIX_LIB_DYNAMIC)

UNSET(WX_LIBS_STATIC)
UNSET(WX_LIBS_BASE)
UNSET(WX_LIBS_MSW)

# ---------------------------------------------------------------------
# done
# ---------------------------------------------------------------------
