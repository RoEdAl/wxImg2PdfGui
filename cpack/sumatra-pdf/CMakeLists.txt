#
# SumatraPDF
#

INSTALL(
	FILES
		${CMAKE_CURRENT_SOURCE_DIR}/sumatrapdfrestrict.ini
		${CMAKE_CURRENT_SOURCE_DIR}/SumatraPDF-settings.txt
	DESTINATION img2pdf/tools/sumatrapdf
	COMPONENT img2pdf
	EXCLUDE_FROM_ALL
)
