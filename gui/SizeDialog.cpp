/*
 * SizeDialog.cpp
 */

#include "SizeDialog.h"
#include "wxApp.h"

namespace
{
    wxStaticLine* create_horizontal_static_line(wxWindow* parent)
    {
        return new wxStaticLine(parent, wxID_ANY, wxDefaultPosition, wxSize(0, parent->FromDIP(1)), wxLI_HORIZONTAL);
    }

    wxSizerFlags get_horizontal_static_line_sizer_flags()
    {
        return wxSizerFlags().Expand().Border(wxLEFT|wxRIGHT|wxTOP);
    }

    void fill_standard_resolutions_array(wxArrayString& standardResolutions, const wxWindow* wnd, bool x)
    {
        const wxDisplay dpl(wnd);
        const wxSize ppi = dpl.GetPPI();
        standardResolutions.Add(wxString::Format("%d", x? ppi.x : ppi.y));
        standardResolutions.Add("150");
        standardResolutions.Add("300");
        standardResolutions.Add("400");
        standardResolutions.Add("600");
    }
}

SizeDialog::SizeDialog(
    wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long style)
    :wxDialog(parent, id, title, pos, size, style), m_singleValue(false)
{
    wxBoxSizer* const sizer = new wxBoxSizer(wxVERTICAL);

    {
        wxBoxSizer* const innerSizer = new wxBoxSizer(wxHORIZONTAL);

        {
            wxBitmapBundle bitmapBundle;
            wxGetApp().LoadMaterialDesignIcon("content-link", bitmapBundle);
            wxBitmapToggleButton* const button = new wxBitmapToggleButton(this, wxID_ANY, bitmapBundle, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);

            const wxGenericValidator validator(&m_singleValue);
            button->SetValidator(validator);

            button->Bind(wxEVT_TOGGLEBUTTON, &SizeDialog::OnToggleSingleValue, this);
            innerSizer->Add(button, wxSizerFlags().Border(wxRIGHT).CenterVertical());
        }

        {
            wxArrayString standardResolutions;
            fill_standard_resolutions_array(standardResolutions, this, true);
            const wxIntegerValidator<int> validator(&m_size.x, 0, 16384);
            wxComboBox* const comboBox = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, standardResolutions, wxCB_SORT, validator);
            innerSizer->Add(comboBox, wxSizerFlags().CenterVertical());
            m_comboBoxWidth = comboBox;
        }

        {
            wxStaticText* const staticTxt = new wxStaticText(this, wxID_ANY, wxS('\u00D7'));
            innerSizer->Add(staticTxt, wxSizerFlags().Border(wxLEFT|wxRIGHT).CenterVertical());
            m_staticTextSeparator = staticTxt;
        }

        {
            wxArrayString standardResolutions;
            fill_standard_resolutions_array(standardResolutions, this, false);
            const wxIntegerValidator<int> validator(&m_size.y, 0, 16384);
            wxComboBox* const comboBox = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, standardResolutions, wxCB_SORT, validator);
            innerSizer->Add(comboBox, wxSizerFlags().CenterVertical());
            m_comboBoxHeight = comboBox;
        }

        {
            wxStaticText* const staticTxt = new wxStaticText(this, wxID_ANY, wxS("dpi"));
            innerSizer->Add(staticTxt, wxSizerFlags().Border(wxLEFT | wxRIGHT).CenterVertical());
        }

        sizer->Add(innerSizer, wxSizerFlags().Expand().Border());
        m_sizer = innerSizer;
    }

    {
        wxStaticLine* const staticLine = create_horizontal_static_line(this);
        sizer->Add(staticLine, get_horizontal_static_line_sizer_flags());
    }

    {
        wxStdDialogButtonSizer* const buttonSizer = CreateStdDialogButtonSizer(wxOK|wxCANCEL);
        sizer->Add(buttonSizer, wxSizerFlags().Border(wxTOP | wxBOTTOM).Right());
    }

    SetSizerAndFit(sizer);
    Bind(wxEVT_INIT_DIALOG, &SizeDialog::OnInitDialog, this);
}

void SizeDialog::OnInitDialog(wxInitDialogEvent& event)
{
    wxDialog::OnInitDialog(event);
    m_staticTextSeparator->Show(!m_singleValue);
    m_comboBoxHeight->Show(!m_singleValue);
}

void SizeDialog::OnToggleSingleValue(wxCommandEvent& event)
{
    const bool show = !event.IsChecked();
    m_staticTextSeparator->Show(show);
    m_comboBoxHeight->Show(show);
    m_sizer->Layout();
}

void SizeDialog::SetValue(const wxSize& size)
{
    m_size = size;
    m_singleValue = size.x == size.y;
}

wxSize SizeDialog::GetValue() const
{
    if (m_singleValue)
    {
        return wxSize(m_size.x, m_size.x);
    }
    else
    {
        return m_size;
    }
}

 // =================================================================================

ScaleDialog::ScaleDialog(
    wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long style)
    :wxDialog(parent, id, title, pos, size, style), m_singleValue(false)
{
    wxBoxSizer* const sizer = new wxBoxSizer(wxVERTICAL);

    {
        wxBoxSizer* const innerSizer = new wxBoxSizer(wxHORIZONTAL);

        {
            wxBitmapBundle bitmapBundle;
            wxGetApp().LoadMaterialDesignIcon("content-link", bitmapBundle);
            wxBitmapToggleButton* const button = new wxBitmapToggleButton(this, wxID_ANY, bitmapBundle, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);

            const wxGenericValidator validator(&m_singleValue);
            button->SetValidator(validator);

            button->Bind(wxEVT_TOGGLEBUTTON, &ScaleDialog::OnToggleSingleValue, this);
            innerSizer->Add(button, wxSizerFlags().Border(wxRIGHT).CenterVertical());
        }

        {
            const wxGenericValidator validator(&m_size.x);
            wxSpinCtrl* const spinCtrl = new wxSpinCtrl(this, wxID_ANY);
            spinCtrl->SetRange(0, 5000);
            spinCtrl->SetValidator(validator);
            innerSizer->Add(spinCtrl, wxSizerFlags().CenterVertical());
            m_spinCtrlWidth = spinCtrl;
        }

        {
            wxStaticText* const staticTxt = new wxStaticText(this, wxID_ANY, wxS('\u00D7'));
            innerSizer->Add(staticTxt, wxSizerFlags().Border(wxLEFT | wxRIGHT).CenterVertical());
            m_staticTextSeparator = staticTxt;
        }

        {
            const wxGenericValidator validator(&m_size.y);
            wxSpinCtrl* const spinCtrl = new wxSpinCtrl(this, wxID_ANY);
            spinCtrl->SetRange(0, 5000);
            spinCtrl->SetValidator(validator);
            innerSizer->Add(spinCtrl, wxSizerFlags().CenterVertical());
            m_spinCtrlHeight = spinCtrl;
        }

        {
            wxStaticText* const staticTxt = new wxStaticText(this, wxID_ANY, wxS('%'));
            innerSizer->Add(staticTxt, wxSizerFlags().Border(wxLEFT | wxRIGHT).CenterVertical());
        }

        sizer->Add(innerSizer, wxSizerFlags().Expand().Border());
        m_sizer = innerSizer;
    }

    {
        wxStaticLine* const staticLine = create_horizontal_static_line(this);
        sizer->Add(staticLine, get_horizontal_static_line_sizer_flags());
    }

    {
        wxStdDialogButtonSizer* const buttonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL);
        sizer->Add(buttonSizer, wxSizerFlags().Border(wxTOP | wxBOTTOM).Right());
    }

    SetSizerAndFit(sizer);
    Bind(wxEVT_INIT_DIALOG, &ScaleDialog::OnInitDialog, this);
}

void ScaleDialog::OnInitDialog(wxInitDialogEvent& event)
{
    wxDialog::OnInitDialog(event);
    m_staticTextSeparator->Show(!m_singleValue);
    m_spinCtrlHeight->Show(!m_singleValue);
}

void ScaleDialog::OnToggleSingleValue(wxCommandEvent& event)
{
    const bool show = !event.IsChecked();
    m_staticTextSeparator->Show(show);
    m_spinCtrlHeight->Show(show);
    m_sizer->Layout();
}

void ScaleDialog::SetValue(const wxSize& size)
{
    m_size = size;
    if (m_size.x <= 0) m_size.x = 100;
    if (m_size.y <= 0) m_size.y = 100;
    m_singleValue = size.x == size.y;
}

wxSize ScaleDialog::GetValue() const
{
    if (m_singleValue)
    {
        return wxSize(m_size.x, m_size.x);
    }
    else
    {
        return m_size;
    }
}
