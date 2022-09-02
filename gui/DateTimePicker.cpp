/*
 * DateTimePicker.cpp
 */
 
#include "DateTimePicker.h"
#include "wxApp.h"

namespace
{
    bool load_bitmaps(wxBitmapBundle& bbNormal, wxBitmapBundle& bbPressed, wxBitmapBundle& bbDisabled)
    {
        wxCHECK_MSG(wxGetApp().LoadMaterialDesignIcon("action-edit_calendar", wxWINDOW_VARIANT_NORMAL, bbNormal), false, "DateTimePicker: Fail to load action-edit_calendar bitmap");
        wxCHECK_MSG(wxGetApp().LoadMaterialDesignIcon("action-today", wxWINDOW_VARIANT_NORMAL, bbPressed), false, "DateTimePicker: Fail to load action-today bitmap");
        wxCHECK_MSG(wxGetApp().LoadDisabledMaterialDesignIcon("action-today", wxWINDOW_VARIANT_NORMAL, bbDisabled), false, "DateTimePicker: Fail to load action-today bitmap");
        return true;
    }
}

DateTimePicker::DateTimePicker(wxWindow* parent, wxWindowID id, const wxDateTime& value)
    :wxControl(parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE| wxCLIP_CHILDREN)
{
    wxBoxSizer* const sizer = new wxBoxSizer(wxHORIZONTAL);

    const wxSizerFlags ctrlFlags = wxSizerFlags().CentreVertical();

    wxBitmapBundle bbNormal, bbPressed, bbDisabled;
    wxCHECK_RET(load_bitmaps(bbNormal, bbPressed, bbDisabled), "DateTimePicker: fail to load bitmaps");

    wxBitmapToggleButton* toggleButton = new wxBitmapToggleButton(this, wxID_ANY, bbNormal, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    toggleButton->SetBitmapPressed(bbPressed);
    toggleButton->SetBitmapDisabled(bbDisabled);
    toggleButton->SetWindowVariant(wxWINDOW_VARIANT_NORMAL);
    toggleButton->SetValue(true);
    toggleButton->Bind(wxEVT_TOGGLEBUTTON, &DateTimePicker::OnToggleButtonCurrent, this);
    toggleButton->SetToolTip(_("Current date and time"));
    toggleButton->SetCanFocus(false);
    sizer->Add(toggleButton, wxSizerFlags(ctrlFlags).Border(wxRIGHT));
    m_toggleButton = toggleButton;

    wxDatePickerCtrl* const datePicker = new wxDatePickerCtrl(this, wxID_ANY, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
    if (value.IsValid()) datePicker->SetValue(value);
    sizer->Add(datePicker, ctrlFlags);
    m_datePicker = datePicker;

    wxTimePickerCtrl* timePicker = new wxTimePickerCtrl(this, wxID_ANY);
    if (value.IsValid()) timePicker->SetValue(value);
    sizer->Add(timePicker, wxSizerFlags(ctrlFlags).Border(wxLEFT));
    m_timePicker = timePicker;

    m_sizer = sizer;
    SetSizerAndFit(m_sizer);

    show_pickers(false);
}

void DateTimePicker::SetValue(const wxDateTime& val)
{
    if (val.IsValid())
    {
        m_datePicker->SetValue(val);
        m_timePicker->SetValue(val);
    }
    else
    {
        const wxDateTime dt = wxDateTime::Now();
        m_datePicker->SetValue(dt);
        m_timePicker->SetValue(dt);
    }
}

wxDateTime DateTimePicker::GetValue() const
{
    if (m_toggleButton->GetValue())
    {
        return wxDateTime();
    }
    else
    {
        wxDateTime dt = m_datePicker->GetValue();
        dt = dt.GetDateOnly();

        int hour, min, sec;
        m_timePicker->GetTime(&hour, &min, &sec);

        dt.SetHour(hour);
        dt.SetMinute(min);
        dt.SetSecond(sec);

        return dt;
    }
}

void DateTimePicker::OnToggleButtonCurrent(wxCommandEvent& event) const
{
   show_pickers(!event.IsChecked());
}

void DateTimePicker::show_pickers(bool show) const
{
    m_datePicker->Show(show);
    m_timePicker->Show(show);
    m_sizer->Layout();
}
