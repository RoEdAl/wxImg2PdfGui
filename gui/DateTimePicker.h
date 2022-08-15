/*
 * DateTimePicker.h
 */
 
#pragma once

class DateTimePicker:public wxControl
{
    public:

    DateTimePicker(wxWindow*, wxWindowID, const wxDateTime&);

    void SetValue(const wxDateTime&);
    wxDateTime GetValue() const;

    protected:

    void OnToggleButtonCurrent(wxCommandEvent&);

    protected:

    void show_pickers(bool);

    protected:

    wxSizer* m_sizer;
    wxToggleButton* m_toggleButton;
    wxDatePickerCtrl* m_datePicker;
    wxTimePickerCtrl* m_timePicker;
};