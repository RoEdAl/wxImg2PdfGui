/*
 * StdWx.h
 */

#ifndef _STD_WX_H
#define _STD_WX_H

#include <StdWx.h>

#include <wx/intl.h>
#include <wx/string.h>
#include <wx/listbox.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/combobox.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <wx/statline.h>
#include <wx/valgen.h>
#include <wx/notebook.h>
#include <wx/bmpbuttn.h>
#include <wx/dataview.h>
#include <wx/tglbtn.h>
#include <wx/spinctrl.h>
#include <wx/frame.h>
#include <wx/app.h>
#include <wx/cmdline.h>
#include <wx/sstream.h>
#include <wx/txtstrm.h>
#include <wx/clipbrd.h>
#include <wx/process.h>
#include <wx/utils.h>
#include <wx/timer.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/filedlg.h>
#include <wx/dnd.h>
#include <wx/regex.h>
#include <wx/mstream.h>
#include <wx/dcscreen.h>
#include <wx/wupdlock.h>
#include <wx/mimetype.h>
#include <wx/valnum.h>

#include <wx/datectrl.h>
#include <wx/timectrl.h>

#include <wx/wfstream.h>
#include <wx/thread.h>
#include <wx/display.h>

#include <FmtSpec.h>
#include <wxJson.h>

#include <utility>
#include <vector>
#include <unordered_map>

extern "C" {
#define IS_INTRESOURCE(_r) ((((ULONG_PTR)(_r)) >> 16)==0)
#define WXMAKEINTRESOURCEW(i) ((LPWSTR)((ULONG_PTR)((WORD)(i))))
#define RT_ICON WXMAKEINTRESOURCEW(3)
#define RT_GROUP_ICON WXMAKEINTRESOURCEW((ULONG_PTR)(RT_ICON)+11)
#define LR_DEFAULTCOLOR 0x00000000

    WINUSERAPI HICON WINAPI CreateIconFromResourceEx(
        _In_reads_bytes_(dwResSize) PBYTE presbits,
        _In_ DWORD dwResSize,
        _In_ BOOL fIcon,
        _In_ DWORD dwVer,
        _In_ int cxDesired,
        _In_ int cyDesired,
        _In_ UINT Flags);

    WINUSERAPI BOOL WINAPI DestroyIcon(_In_ HICON hIcon);
}

#endif  // _STD_WX_H
