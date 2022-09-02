/*
 * DateTimePicker.h
 */
 
#ifndef _DATE_TIME_PICKER_H_
#define _DATE_TIME_PICKER_H_

class DateTimePicker:public wxControl
{
    public:

    DateTimePicker(wxWindow*, wxWindowID, const wxDateTime&);

    void SetValue(const wxDateTime&);
    wxDateTime GetValue() const;

    protected:

    void OnToggleButtonCurrent(wxCommandEvent&) const;

    protected:

    void show_pickers(bool) const;

    protected:

    wxSizer* m_sizer;
    wxToggleButton* m_toggleButton;
    wxDatePickerCtrl* m_datePicker;
    wxTimePickerCtrl* m_timePicker;
};

#endif
