/*
 * SizeDialog.h
 */
 
#pragma once

class SizeDialog :public wxDialog
{
	public:
	
	SizeDialog(wxWindow*, 
			   wxWindowID = wxID_ANY,
			   const wxString& = wxEmptyString,
	           const wxPoint& = wxDefaultPosition,
	           const wxSize& = wxDefaultSize,
	           long = wxDEFAULT_DIALOG_STYLE);

	void SetValue(const wxSize&);
	wxSize GetValue() const;

	protected:

	void OnInitDialog(wxInitDialogEvent&);
	void OnToggleSingleValue(wxCommandEvent&);

	protected:

	wxTextCtrl* m_textCtrlWidth;
	wxTextCtrl* m_textCtrlHeight;
	wxStaticText* m_staticTextSeparator;
	wxSizer* m_sizer;

	protected:

	wxSize m_size;
	bool m_singleValue;
};
