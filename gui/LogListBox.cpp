/*
 *      LogListBox.cpp
 */

#include "LogListBox.h"

 // ===============================================================================================================

ListBox::ListBox(wxWindow* parent)
    : wxListBox(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE | wxLB_HSCROLL | wxLB_NEEDED_SB)
{
    const wxFont font(wxSMALL_FONT->GetPointSize(), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    SetFont(font);
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
}

void ListBox::ShowLastItem()
{
    const int cnt = GetCount();
    if (cnt <= 0) return;
    EnsureVisible(cnt - 1);
}

wxString ListBox::GetItemsAsText() const
{
    const int cnt = GetCount();
    if (cnt <= 0) return wxEmptyString;

    wxTextOutputStreamOnString tos;

    for (int i = 0; i < cnt; ++i)
    {
        *tos << GetString(i) << endl;
    }

    tos->Flush();
    return tos.GetString();
}

// ===============================================================================================================

LogListBox::LogListBox(ListBox* listBox): m_listBox(listBox)
{
    wxASSERT(listBox != nullptr);
}

void LogListBox::DoLogText(const wxString& msg)
{
    m_listBox->AppendAndEnsureVisible(msg);
}

// ===============================================================================================================

SimpleLogListBox::SimpleLogListBox(ListBox* listBox): m_listBox(listBox)
{
    wxASSERT(listBox != nullptr);
}

void SimpleLogListBox::DoLogText(const wxString& msg)
{
    m_listBox->AppendString(msg);
}

