/*
 *      LogListBox.cpp
 */

#include "LogListBox.h"
 #include <wxEncodingDetection/wxTextOutputStreamOnString.h>

 // ===============================================================================================================

ListBox::ListBox(wxWindow* const parent)
    : wxListBox(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE | wxLB_HSCROLL | wxLB_NEEDED_SB | wxBORDER_THEME)
{
    const wxFont font(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_TELETYPE, wxNORMAL_FONT->GetStyle(), wxNORMAL_FONT->GetWeight());
    SetFont(font);
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
}

wxString ListBox::GetItemsAsText() const
{
    const int cnt = GetCount();
    if (cnt <= 0) return wxEmptyString;

    wxTextOutputStreamOnString tos;

    for (int i = 0; i < cnt; ++i)
    {
        const wxString line = GetString(i);
        if (line.IsEmpty())
        {
            *tos << endl;
        }
        else
        {
            *tos << GetString(i) << endl;
        }
    }

    tos->Flush();
    return tos.GetString();
}

// ===============================================================================================================

LogListBox::LogListBox(ListBox* const listBox): m_listBox(listBox)
{
    wxASSERT(listBox != nullptr);
}

void LogListBox::DoLogText(const wxString& msg)
{
    m_listBox->AppendAndEnsureVisible(msg);
}
