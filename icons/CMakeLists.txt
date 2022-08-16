#
# material-design-icons library
#

SET(SRC_FILES material-design-icons.rc)
IF(MINGW)
	LIST(APPEND SRC_FILES material-design-icons.c)
ENDIF()

ADD_LIBRARY(material-design-icons SHARED ${SRC_FILES})

IF(MSVC)
	TARGET_LINK_OPTIONS(material-design-icons PRIVATE "/NOENTRY" "/MANIFEST:NO")
ENDIF()

IF(MINGW)
	TARGET_LINK_OPTIONS(material-design-icons PRIVATE
		-nostartfiles
		-nodefaultlibs
		-nolibc
		-nostdlib
		-s
	)
	SET_PROPERTY(TARGET material-design-icons PROPERTY PREFIX "")
	SET_PROPERTY(TARGET material-design-icons PROPERTY IMPORT_PREFIX "")
ENDIF()

ADD_DEPENDENCIES(wxImg2PdfGui material-design-icons)

ADD_CUSTOM_COMMAND(TARGET material-design-icons POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:material-design-icons> $<TARGET_FILE_DIR:wxImg2PdfGui>
)

INSTALL(
	TARGETS material-design-icons
	DESTINATION img2pdf
	COMPONENT img2pdf
)


