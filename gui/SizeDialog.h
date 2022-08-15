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

	wxComboBox* m_comboBoxWidth;
	wxComboBox* m_comboBoxHeight;
	wxStaticText* m_staticTextSeparator;
	wxSizer* m_sizer;

	protected:

	wxSize m_size;
	bool m_singleValue;
};

class ScaleDialog:public wxDialog
{
	public:

	ScaleDialog(wxWindow*,
			   wxWindowID = wxID_ANY,
			   const wxString & = wxEmptyString,
			   const wxPoint & = wxDefaultPosition,
			   const wxSize & = wxDefaultSize,
			   long = wxDEFAULT_DIALOG_STYLE);

	void SetValue(const wxSize&);
	wxSize GetValue() const;

	protected:

	void OnInitDialog(wxInitDialogEvent&);
	void OnToggleSingleValue(wxCommandEvent&);

	protected:

	wxSpinCtrl* m_spinCtrlWidth;
	wxSpinCtrl* m_spinCtrlHeight;
	wxStaticText* m_staticTextSeparator;
	wxSizer* m_sizer;

	protected:

	wxSize m_size;
	bool m_singleValue;
};
