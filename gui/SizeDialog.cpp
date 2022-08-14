/*
 * SizeDialog.cpp
 */

#include "SizeDialog.h"

namespace
{
    wxStaticLine* create_horizontal_static_line(wxWindow* parent)
    {
        return new wxStaticLine(parent, wxID_ANY, wxDefaultPosition, wxSize(0, parent->FromDIP(1)), wxLI_HORIZONTAL);
    }

    wxSizerFlags get_horizontal_static_line_sizer_flags(wxWindow* wnd)
    {
        return wxSizerFlags().Expand().Border(wxTOP | wxBOTTOM, wnd->FromDIP(2));
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
            const wxIconBundle iconBundle("ico_link", nullptr);
            wxBitmapToggleButton* const button = new wxBitmapToggleButton(this, wxID_ANY, wxBitmapBundle::FromIconBundle(iconBundle), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);

            wxGenericValidator validator(&m_singleValue);
            button->SetValidator(validator);

            button->Bind(wxEVT_TOGGLEBUTTON, &SizeDialog::OnToggleSingleValue, this);
            innerSizer->Add(button, wxSizerFlags().Border(wxRIGHT));
        }

        {
            wxIntegerValidator<int> validator(&m_size.x, 0, 16384);
            wxTextCtrl* const textCtrl = new wxTextCtrl(this, wxID_ANY);
            textCtrl->SetMaxLength(5);
            textCtrl->SetValidator(validator);
            innerSizer->Add(textCtrl);
            m_textCtrlWidth = textCtrl;
        }

        {
            wxStaticText* const staticTxt = new wxStaticText(this, wxID_ANY, wxS('\u00D7'));
            innerSizer->Add(staticTxt, wxSizerFlags().Border(wxLEFT|wxRIGHT));
            m_staticTextSeparator = staticTxt;
        }

        {
            wxIntegerValidator<int> validator(&m_size.y, 0, 16384);
            wxTextCtrl* const textCtrl = new wxTextCtrl(this, wxID_ANY);
            textCtrl->SetMaxLength(5);
            textCtrl->SetValidator(validator);
            innerSizer->Add(textCtrl);
            m_textCtrlHeight = textCtrl;
        }

        sizer->Add(innerSizer, wxSizerFlags().Expand().Border());
        m_sizer = innerSizer;
    }

    {
        wxStaticLine* const staticLine = create_horizontal_static_line(this);
        sizer->Add(staticLine, get_horizontal_static_line_sizer_flags(this));
    }

    {
        wxStdDialogButtonSizer* const buttonSizer = new wxStdDialogButtonSizer();

        {
            wxButton* const button = new wxButton(this, wxID_OK);
            buttonSizer->AddButton(button);
        }

        {
            wxButton* const button = new wxButton(this, wxID_CANCEL);
            buttonSizer->AddButton(button);
        }

        buttonSizer->Realize();

        sizer->Add(buttonSizer, wxSizerFlags().CenterHorizontal().Border(wxTOP|wxBOTTOM).Right());
    }

    SetSizerAndFit(sizer);
    Bind(wxEVT_INIT_DIALOG, &SizeDialog::OnInitDialog, this);
}

void SizeDialog::OnInitDialog(wxInitDialogEvent& event)
{
    wxDialog::OnInitDialog(event);
    m_staticTextSeparator->Show(!m_singleValue);
    m_textCtrlHeight->Show(!m_singleValue);
}

void SizeDialog::OnToggleSingleValue(wxCommandEvent& event)
{
    if (event.IsChecked())
    {
        m_staticTextSeparator->Show(false);
        m_textCtrlHeight->Show(false);
    }
    else
    {
        m_staticTextSeparator->Show();
        m_textCtrlHeight->Show();
    }
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
