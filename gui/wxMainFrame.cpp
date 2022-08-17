/*
 *      wxMainFrame.cpp
 */

#include <app_config.h>
#include "wxApp.h"
#include "wxMainFrame.h"
#include "SizeDialog.h"
#include <wxEncodingDetection/wxTextInputStreamOnString.h>

namespace
{
    const int AUTO_SCROLL_UPDATE_INTERVAL = 2000;
    const int TIMER_IDLE_WAKE_UP_INTERVAL = 250;
    const wxChar DEFAULT_OUTPUT_FILE_NAME[] = wxS("album.pdf");

    template<typename T>
    const T* get_variant_data(const wxVariant& v)
    {
        return const_cast<const T*>(static_cast<T*>(v.GetData()));
    }

    template<typename T, typename R = typename T::R>
    const R& get_variant_custom_val(const wxVariant& v)
    {
        return get_variant_data<T>(v)->GetValue();
    }

    wxStaticBoxSizer* create_static_box_sizer(wxWindow* parent, const wxString& label, wxOrientation orientation)
    {
        return new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, label), orientation);
    }

    wxStaticText* create_static_text(wxWindow* parent, const wxString& label)
    {
        wxStaticText* const res = new wxStaticText(parent, wxID_ANY, label);

        res->Wrap(-1);
        return res;
    }

    wxSizerFlags get_left_ctrl_sizer_flags(wxWindow* wnd)
    {
        return wxSizerFlags().CenterVertical().Border(wxRIGHT).Proportion(0);
    }

    wxSizerFlags get_middle_crtl_sizer_flags(wxWindow* wnd)
    {
        return wxSizerFlags().CenterVertical().Border(wxLEFT | wxRIGHT).Proportion(0);
    }

    wxSizerFlags get_left_exp_crtl_sizer_flags(wxWindow* wnd)
    {
        return wxSizerFlags().CenterVertical().Border(wxRIGHT).Proportion(1);
    }

    wxSizerFlags get_right_crtl_sizer_flags(wxWindow* wnd)
    {
        return wxSizerFlags().CenterVertical().Border(wxLEFT).Proportion(0);
    }

    wxSizerFlags get_vertical_allign_sizer_flags()
    {
        return wxSizerFlags().CenterVertical().Proportion(0);
    }

    wxCheckBox* create_checkbox(wxWindow* parent, const wxString& label, bool val = false)
    {
        wxCheckBox* const res = new wxCheckBox(parent, wxID_ANY, label);

        res->SetValue(val);
        return res;
    }

    wxCheckBox* create_checkbox(const wxStaticBoxSizer* parentSizer, const wxString& label, bool val = false)
    {
        return create_checkbox(parentSizer->GetStaticBox(), label, val);
    }

    wxCheckBox* create_3state_checkbox(wxWindow* parent, const wxString& label, wxCheckBoxState state = wxCHK_UNDETERMINED)
    {
        wxCheckBox* const res = new wxCheckBox(parent, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxCHK_3STATE | wxCHK_ALLOW_3RD_STATE_FOR_USER);

        res->Set3StateValue(state);
        return res;
    }

    bool is_checked(const wxCheckBox* checkBox)
    {
        if (checkBox->Is3State()) return (checkBox->Get3StateValue() == wxCHK_CHECKED);
        else return checkBox->GetValue();
    }

    wxCheckBox* create_3state_checkbox(const wxStaticBoxSizer* parentSizer, const wxString& label, wxCheckBoxState state = wxCHK_UNDETERMINED)
    {
        return create_3state_checkbox(parentSizer->GetStaticBox(), label, state);
    }

    wxButton* create_button(const wxStaticBoxSizer* parentSizer, const wxString& label)
    {
        return new wxButton(parentSizer->GetStaticBox(), wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    }

    wxButton* create_button(wxWindow* parent, const wxString& label)
    {
        return new wxButton(parent, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    }

    wxBitmapButton* create_bitmap_button(const wxStaticBoxSizer* parentSizer, const wxBitmapBundle& bitmapBundle)
    {
        wxBitmapButton* const res = new wxBitmapButton(parentSizer->GetStaticBox(), wxID_ANY, bitmapBundle);
        res->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
        return res;
    }

    wxBitmapButton* create_bitmap_button(const wxStaticBoxSizer* parentSizer, const wxString& resName)
    {
        wxBitmapBundle bitmapBundle;
        if (wxGetApp().LoadMaterialDesignIcon(resName, bitmapBundle))
        {
            return create_bitmap_button(parentSizer, bitmapBundle);
        }
        else
        {
            return nullptr;
        }
    }

    wxBitmapButton* create_bitmap_button(wxWindow* parent, const wxBitmapBundle& bitmapBundle)
    {
        wxBitmapButton* const res = new wxBitmapButton(parent, wxID_ANY, bitmapBundle);
        res->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
        return res;
    }

    wxBitmapButton* create_bitmap_button(wxWindow* parent, const wxString& resName)
    {
        wxBitmapBundle bitmapBundle;
        if (wxGetApp().LoadMaterialDesignIcon(resName, bitmapBundle))
        {
            return create_bitmap_button(parent, bitmapBundle);
        }
        else
        {
            return nullptr;
        }
    }

    wxTextCtrl* create_text_ctrl(wxWindow* parent, const wxString& label = wxEmptyString, unsigned long maxLength = 0)
    {
        wxTextCtrl* const res = new wxTextCtrl(parent, wxID_ANY, label);
        if (maxLength > 0) res->SetMaxLength(maxLength);
        return res;
    }

    wxTextCtrl* create_text_ctrl(const wxStaticBoxSizer* parentSizer, const wxString& label = wxEmptyString, unsigned long maxLength = 0)
    {
        return create_text_ctrl(parentSizer->GetStaticBox(), label, maxLength);
    }

    wxStaticText* create_static_text(wxWindow* parent)
    {
        return new wxStaticText(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT | wxST_ELLIPSIZE_MIDDLE| wxST_NO_AUTORESIZE);
    }

    wxStaticText* create_static_text(const wxStaticBoxSizer* parentSizer)
    {
        return create_static_text(parentSizer->GetStaticBox());
    }

    wxStaticLine* create_horizontal_static_line(wxWindow* parent)
    {
        return new wxStaticLine(parent, wxID_ANY, wxDefaultPosition, wxSize(0, parent->FromDIP(1)), wxLI_HORIZONTAL);
    }

    wxSizerFlags get_horizontal_static_line_sizer_flags(wxWindow* wnd)
    {
        return wxSizerFlags().Expand().Border(wxTOP | wxBOTTOM, wnd->FromDIP(2));
    }

    wxSize calc_text_size(int charWidth)
    {
        wxScreenDC dc;
        wxString txt(wxUniChar(0x2013), charWidth);
        const wxSize extent = dc.GetTextExtent(txt);
        return wxSize(extent.GetWidth(), -1);
    }

    class MyProcess:
        public wxProcess
    {
        public:

        MyProcess()
        {
            Redirect();
        }

        void CreateTxtStreams()
        {
            m_txtInputStream.reset(new wxTextInputStream(*(GetInputStream()), wxEmptyString, wxConvUTF8));
            m_txtErrorStream.reset(new wxTextInputStream(*(GetErrorStream()), wxEmptyString, wxConvUTF8));
        }

        wxTextInputStream& GetTxtInputStream() const
        {
            return *(m_txtInputStream.get());
        }

        wxTextInputStream& GetTxtErrorStream() const
        {
            return *(m_txtErrorStream.get());
        }

        bool HaveOutOrErr() const
        {
            return !(GetInputStream()->Eof() && GetErrorStream()->Eof());
        }

        private:

        wxScopedPtr<wxTextInputStream> m_txtInputStream;
        wxScopedPtr<wxTextInputStream> m_txtErrorStream;
    };

    void kill_console_process(long pid)
    {
        if (!AttachConsole(pid))
        {
            const WXDWORD err = GetLastError();
            if (err == ERROR_ACCESS_DENIED)
            {
                FreeConsole(); // detach first
                if (!AttachConsole(pid)) // attach again
                {
                    return;
                }
            }
        }

        wxLogWarning(_("exe[t]: sending CTRL+BREAK event to process %ld"), pid);
        bool res = GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, pid) != 0;
        if (!res)
        {
            const WXDWORD err = GetLastError();
            wxLogError(_("exe[t]: signal CTRL+BREAK not sent, error code %d"), err);
        }
        res = FreeConsole() != 0;
    }

    void kill_console_process(const wxProcess& process)
    {
        kill_console_process(process.GetPid());
    }

    wxFileName get_default_output_fn()
    {
        wxFileName fn = wxFileName::DirName(wxStandardPaths::Get().GetUserDir(wxStandardPaths::Dir_Pictures));
        fn.SetFullName(DEFAULT_OUTPUT_FILE_NAME);
        return fn;
    }

    void append_item(wxDataViewListCtrl* const listCtrl, const wxFileName& fn, const wxObjectDataPtr<wxFileNameRefData>& commonDir)
    {
        wxVector<wxVariant> data;
        if (wxGetApp().GetFnColumn(wxRelativeFileName(fn, commonDir), data))
        {
            listCtrl->AppendItem(data);
        }
        else
        {
            wxLogVerbose(_("Unable to add input file: %s"), fn.GetFullName());
        }
    }

    class CheckBoxUiUpdater
    {
        public:

        CheckBoxUiUpdater(const wxCheckBox* const checkBox)
            : m_checkBox(checkBox)
        {
        }

        CheckBoxUiUpdater(const CheckBoxUiUpdater& uiUpdater)
            : m_checkBox(uiUpdater.m_checkBox)
        {
        }

        void operator ()(wxUpdateUIEvent& event) const
        {
            event.Enable(is_checked());
        }

        bool operator !=(const CheckBoxUiUpdater& uiUpdater) const
        {
            return m_checkBox != uiUpdater.m_checkBox;
        }

        bool operator ==(const CheckBoxUiUpdater& uiUpdater) const
        {
            return m_checkBox == uiUpdater.m_checkBox;
        }

        private:

        const wxCheckBox* const m_checkBox;

        bool is_checked() const
        {
            if (m_checkBox->Is3State()) return (m_checkBox->Get3StateValue() == wxCHK_CHECKED || m_checkBox->Get3StateValue() == wxCHK_UNDETERMINED);
            else return m_checkBox->GetValue();
        }
    };

    class DropTarget:
        public wxFileDropTarget
    {
        public:

        DropTarget(wxMainFrame* pMainFrame): m_pMainFrame(pMainFrame)
        {
        }

        virtual bool OnDropFiles(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y), const wxArrayString& filenames)
        {
            const wxBusyCursor busy;
            m_pMainFrame->OnDropFiles(filenames);
            return true;
        }

        private:

        wxMainFrame* const m_pMainFrame;
    };
}

wxPanel* wxMainFrame::create_src_dst_pannel(wxNotebook* notebook)
{
    wxPanel* const    panel = new wxPanel(notebook);
    wxBoxSizer* const panelSizer = new wxBoxSizer(wxVERTICAL);

    {
        wxStaticBoxSizer* const sizer = create_static_box_sizer(panel, _("Sources"), wxHORIZONTAL);
        {
            {
                wxFlexGridSizer* const innerSizer = new wxFlexGridSizer(2);
                innerSizer->AddGrowableCol(0);
                innerSizer->AddGrowableRow(0);

                {
                    m_listViewInputFiles = new wxDataViewListCtrl(sizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_VERT_RULES | wxDV_MULTIPLE | wxDV_ROW_LINES | wxBORDER_THEME);

                    wxDataViewBitmapRenderer* const bitmapRenderer = new wxDataViewBitmapRenderer(wxS("wxBitmapBundle"));
                    wxDataViewColumn* const iconColumn = new wxDataViewColumn(wxS("#"), bitmapRenderer, 0, wxCOL_WIDTH_AUTOSIZE);
                    iconColumn->SetSortable(false);
                    iconColumn->SetReorderable(false);
                    iconColumn->SetResizeable(false);
                    m_listViewInputFiles->AppendColumn(iconColumn);

                    wxDataViewTextRenderer* const pathRenderer = new wxDataViewTextRenderer(wxS("wxRelativeFileName"));
                    wxDataViewColumn* const pathColumn = new wxDataViewColumn(_("File"), pathRenderer, 1, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
                    pathColumn->SetSortable(false);
                    pathColumn->SetReorderable(false);
                    pathColumn->SetResizeable(false);
                    m_listViewInputFiles->AppendColumn(pathColumn);

                    const wxSize minSize = calc_text_size(15);
                    wxDataViewTextRenderer* const sizeRenderer = new wxDataViewTextRenderer(wxS("wxSize"));
                    wxDataViewColumn* const sizeColumn = new wxDataViewColumn(_("Size"), sizeRenderer, 2, wxCOL_WIDTH_AUTOSIZE);
                    sizeColumn->SetSortable(false);
                    sizeColumn->SetReorderable(false);
                    sizeColumn->SetResizeable(false);
                    sizeColumn->SetMinWidth(minSize.GetWidth());
                    m_listViewInputFiles->AppendColumn(sizeColumn);

                    wxDataViewTextRenderer* const resolutionRenderer = new wxDataViewTextRenderer(wxS("wxResolutionOrScale"));
                    wxDataViewColumn* const resolutionColumn = new wxDataViewColumn(_("Resolution/Scale"), resolutionRenderer, 3, wxCOL_WIDTH_AUTOSIZE);
                    resolutionColumn->SetSortable(false);
                    resolutionColumn->SetReorderable(false);
                    resolutionColumn->SetResizeable(false);
                    m_listViewInputFiles->AppendColumn(resolutionColumn);

                    wxDataViewTextRenderer* const lastRenderer = new wxDataViewTextRenderer(wxS("null"));
                    wxDataViewColumn* const lastColumn = new wxDataViewColumn(wxEmptyString, lastRenderer, 4, wxCOL_WIDTH_AUTOSIZE);
                    lastColumn->SetSortable(false);
                    lastColumn->SetReorderable(false);
                    lastColumn->SetResizeable(false);
                    m_listViewInputFiles->AppendColumn(lastColumn);

                    m_listViewInputFiles->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, &wxMainFrame::OnDataViewItemActiveted, this);

                    innerSizer->Add(m_listViewInputFiles, wxSizerFlags().Proportion(1).Expand());
                }

                {
                    wxBoxSizer* const vinnerSizer = new wxBoxSizer(wxVERTICAL);

                    {
                        wxBitmapButton* const button = create_bitmap_button(sizer, "content-add");
                        button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonAdd, this);
                        button->Bind(wxEVT_BUTTON, &wxMainFrame::OnButtonAdd, this);
                        vinnerSizer->Add(button, wxSizerFlags().CentreHorizontal());
                    }

                    {
                        wxBitmapButton* const button = create_bitmap_button(sizer, "content-remove");
                        button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonDelete, this);
                        button->Bind(wxEVT_BUTTON, &wxMainFrame::OnButtonDelete, this);
                        vinnerSizer->Add(button, wxSizerFlags().CenterHorizontal());
                    }

                    {
                        wxBitmapBundle bitmapBundle;
                        wxBitmapButton* const button = create_bitmap_button(sizer, "content-select_all");
                        button->SetToolTip(_("Select all"));
                        button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonSelectAll, this);
                        button->Bind(wxEVT_BUTTON, &wxMainFrame::OnButtonSelectAll, this);
                        vinnerSizer->Add(button, wxSizerFlags().CenterHorizontal());
                    }

                    {
                        wxStaticLine* const staticLine = create_horizontal_static_line(sizer->GetStaticBox());
                        vinnerSizer->Add(staticLine, get_horizontal_static_line_sizer_flags(sizer->GetStaticBox()));
                    }

                    {
                        wxBitmapButton* const button = create_bitmap_button(sizer, "action-aspect_ratio");
                        button->SetToolTip(_("Change resolution/Scale"));
                        button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonResolutionScale, this);
                        button->Bind(wxEVT_BUTTON, &wxMainFrame::OnButtonResolutionScale, this);
                        vinnerSizer->Add(button, wxSizerFlags().CenterHorizontal());
                    }

                    {
                        wxBitmapButton* const button = create_bitmap_button(sizer, "content-clear");
                        button->SetToolTip(_("Use original image resolution/Do not scale image"));
                        button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonResolutionScale, this);
                        button->Bind(wxEVT_BUTTON, &wxMainFrame::OnButtonClearResolutionScale, this);
                        vinnerSizer->Add(button, wxSizerFlags().CenterHorizontal());
                    }

                    if (wxGetApp().SumatraPdfFound())
                    {

                        vinnerSizer->AddStretchSpacer();

                        {
                            wxBitmapButton* const button = create_bitmap_button(sizer, "action-preview");
                            button->SetToolTip(_("Launch document viewer"));
                            button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonDocOpen, this);
                            button->Bind(wxEVT_BUTTON, &wxMainFrame::OnButtonDocOpen, this);
                            vinnerSizer->Add(button, wxSizerFlags().CenterHorizontal().DoubleBorder(wxTOP));
                        }

                        innerSizer->Add(vinnerSizer, wxSizerFlags().Expand().Border(wxLEFT));
                    }
                    else
                    {
                        innerSizer->Add(vinnerSizer, wxSizerFlags().Top().Border(wxLEFT));
                    }
                }

                {
                    wxStaticText* staticText = create_static_text(panel);
                    staticText->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
                    staticText->SetToolTip(_("Common directory"));
                    staticText->Hide();
                    innerSizer->Add(staticText, wxSizerFlags().Expand().ReserveSpaceEvenIfHidden().CenterVertical().Border(wxTOP));
                    m_staticTextCommonDir = staticText;
                }

                {
                    wxButton* const button = create_button(sizer, wxS("\u25BC"));
                    button->SetToolTip(_("Copy common directory to destination"));
                    button->Hide();
                    button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonCopyToDst, this);
                    button->Bind(wxEVT_BUTTON, &wxMainFrame::OnButtonCopyToDst, this);
                    innerSizer->Add(button, wxSizerFlags().Center().Border(wxTOP|wxLEFT).ReserveSpaceEvenIfHidden());
                    m_buttonCommonDir = button;
                }

                sizer->Add(innerSizer, wxSizerFlags().Expand().Proportion(1));
            }

            panelSizer->Add(sizer, wxSizerFlags().Expand().Proportion(1));
        }
    }

    {
        wxStaticBoxSizer* const sizer = create_static_box_sizer(panel, _("Destination"), wxVERTICAL);

        {
            wxBoxSizer* const innerSizer = new wxBoxSizer(wxHORIZONTAL);

            m_textCtrlDst = create_text_ctrl(sizer, wxEmptyString, 1024);
            const wxFileName fn = get_default_output_fn();
            m_textCtrlDst->SetValue(fn.GetFullPath());
            innerSizer->Add(m_textCtrlDst, wxSizerFlags().CentreVertical().Proportion(1));

            wxBitmapButton* const button = create_bitmap_button(sizer, "navigation-more_horiz");
            button->Bind(wxEVT_BUTTON, &wxMainFrame::OnChooseDst, this);
            innerSizer->Add(button, wxSizerFlags().CenterVertical().Border(wxLEFT));

            sizer->Add(innerSizer, wxSizerFlags().Expand());
        }

        {
            wxFlexGridSizer* const innerSizer = new wxFlexGridSizer(4);

            m_checkBoxOutputCompressFonts = create_checkbox(sizer, _T("Compress fonts"), true);
            m_checkBoxOutputCompressFonts->SetWindowVariant(wxWINDOW_VARIANT_MINI);
            m_checkBoxOutputCompressFonts->SetToolTip(_T("Compress fonts"));
            innerSizer->Add(m_checkBoxOutputCompressFonts);

            m_checkBoxOutputPretty = create_checkbox(sizer, _T("Pretty"), true);
            m_checkBoxOutputPretty->SetWindowVariant(wxWINDOW_VARIANT_MINI);
            m_checkBoxOutputPretty->SetToolTip(_wxS("Pretty\u2011print objects with indentation"));
            innerSizer->Add(m_checkBoxOutputPretty);

            m_checkBoxOutputClean = create_checkbox(sizer, _T("Clean"), true);
            m_checkBoxOutputClean->SetWindowVariant(wxWINDOW_VARIANT_MINI);
            m_checkBoxOutputClean->SetToolTip(_wxS("Pretty\u2011print graphics commands in content streams"));
            innerSizer->Add(m_checkBoxOutputClean);

            m_checkBoxOutputSanitize = create_checkbox(sizer, _T("Sanitize"), true);
            m_checkBoxOutputSanitize->SetWindowVariant(wxWINDOW_VARIANT_MINI);
            m_checkBoxOutputSanitize->SetToolTip(_T("Sanitize graphics commands in content streams"));
            innerSizer->Add(m_checkBoxOutputSanitize);

            m_checkBoxOutputLinearize = create_checkbox(sizer, _T("Linearize"));
            m_checkBoxOutputLinearize->SetWindowVariant(wxWINDOW_VARIANT_MINI);
            m_checkBoxOutputLinearize->SetToolTip(_T("Optimize for web browsers"));
            innerSizer->Add(m_checkBoxOutputLinearize);

            m_checkBoxOutputDecompress = create_checkbox(sizer, _T("Decompress"));
            m_checkBoxOutputDecompress->SetWindowVariant(wxWINDOW_VARIANT_MINI);
            m_checkBoxOutputDecompress->SetToolTip(_T("Decompress all streams"));
            innerSizer->Add(m_checkBoxOutputDecompress);

            m_checkBoxOutputAscii = create_checkbox(sizer, _T("ASCII"));
            m_checkBoxOutputAscii->SetWindowVariant(wxWINDOW_VARIANT_MINI);
            m_checkBoxOutputAscii->SetToolTip(_T("ASCII hex encode binary streams"));
            innerSizer->Add(m_checkBoxOutputAscii);

            sizer->Add(innerSizer, wxSizerFlags().Border(wxTOP));
        }

        panelSizer->Add(sizer, wxSizerFlags().Expand());
    }

    panel->SetSizerAndFit(panelSizer);
    return panel;
}

wxPanel* wxMainFrame::create_metadata_pannel(wxNotebook* notebook)
{
    wxPanel* const    panel = new wxPanel(notebook);
    wxBoxSizer* const panelSizer = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer* const sizer = new wxFlexGridSizer(0, 2, wxSizerFlags::GetDefaultBorder(), wxSizerFlags::GetDefaultBorder());
    sizer->AddGrowableCol(1);

    {
        const wxSizerFlags labelSizerFlags = wxSizerFlags().CenterVertical();
        const wxSizerFlags textCtrlSizerFlags = wxSizerFlags().CenterVertical().Expand().Proportion(1);

        m_checkBoxMetadataAuthor = create_checkbox(panel, _("Author"));
        sizer->Add(m_checkBoxMetadataAuthor, labelSizerFlags);

        m_textCtrlMetadataAuthor = create_text_ctrl(panel);
        m_textCtrlMetadataAuthor->Bind(wxEVT_UPDATE_UI, CheckBoxUiUpdater(m_checkBoxMetadataAuthor));
        sizer->Add(m_textCtrlMetadataAuthor, textCtrlSizerFlags);

        m_checkBoxMetadataTitle = create_checkbox(panel, _("Title"));
        sizer->Add(m_checkBoxMetadataTitle, labelSizerFlags);

        m_textCtrlMetadataTitle = create_text_ctrl(panel);
        m_textCtrlMetadataTitle->Bind(wxEVT_UPDATE_UI, CheckBoxUiUpdater(m_checkBoxMetadataTitle));
        sizer->Add(m_textCtrlMetadataTitle, textCtrlSizerFlags);

        m_checkBoxMetadataSubject = create_checkbox(panel, _("Subject"));
        sizer->Add(m_checkBoxMetadataSubject, labelSizerFlags);

        m_textCtrlMetadataSubject = create_text_ctrl(panel);
        m_textCtrlMetadataSubject->Bind(wxEVT_UPDATE_UI, CheckBoxUiUpdater(m_checkBoxMetadataSubject));
        sizer->Add(m_textCtrlMetadataSubject, textCtrlSizerFlags);

        m_checkBoxMetadataCreator = create_checkbox(panel, _("Creator"));
        sizer->Add(m_checkBoxMetadataCreator, labelSizerFlags);

        m_textCtrlMetadataCreator = create_text_ctrl(panel, wxGetApp().GetScriptPath().GetFullName());
        m_textCtrlMetadataCreator->Bind(wxEVT_UPDATE_UI, CheckBoxUiUpdater(m_checkBoxMetadataCreator));
        sizer->Add(m_textCtrlMetadataCreator, textCtrlSizerFlags);

        const wxDateTime dtNow = wxDateTime::Now();

        m_checkBoxMetadataCreationDate = create_checkbox(panel, _("Creation date"));
        sizer->Add(m_checkBoxMetadataCreationDate, labelSizerFlags);

        m_dateTimePickerMetadataCreationDate = new DateTimePicker(panel, wxID_ANY, dtNow);
        m_dateTimePickerMetadataCreationDate->Bind(wxEVT_UPDATE_UI, CheckBoxUiUpdater(m_checkBoxMetadataCreationDate));
        sizer->Add(m_dateTimePickerMetadataCreationDate, textCtrlSizerFlags);

        m_checkBoxMetadataModDate = create_checkbox(panel, _("Modification date"));
        sizer->Add(m_checkBoxMetadataModDate, labelSizerFlags);

        m_dateTimePickerMetadataModDate = new DateTimePicker(panel, wxID_ANY, dtNow);
        m_dateTimePickerMetadataModDate->Bind(wxEVT_UPDATE_UI, CheckBoxUiUpdater(m_checkBoxMetadataModDate));
        sizer->Add(m_dateTimePickerMetadataModDate, textCtrlSizerFlags);
    }

    panelSizer->Add(sizer, wxSizerFlags().Border(wxLEFT | wxRIGHT | wxTOP).Expand());

    panel->SetSizerAndFit(panelSizer);
    return panel;
}

wxPanel* wxMainFrame::create_messages_panel(wxNotebook* notebook)
{
    wxPanel* const    panel = new wxPanel(notebook);
    wxBoxSizer* const panelSizer = new wxBoxSizer(wxVERTICAL);

    {
        m_listBoxMessages = new ListBox(panel);
        panelSizer->Add(m_listBoxMessages, wxSizerFlags().Border(wxLEFT|wxRIGHT|wxTOP).Expand().Proportion(1));
    }

    {
        wxBoxSizer* const sizer = new wxBoxSizer(wxHORIZONTAL);

        {
            wxCheckBox* const checkBox = create_checkbox(panel, _("Verbose"), wxLog::GetVerbose());
            checkBox->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
            checkBox->SetToolTip(_T("Be more verbose"));
            checkBox->Bind(wxEVT_CHECKBOX, &wxMainFrame::OnCheckVerbose, this);
            sizer->Add(checkBox, wxSizerFlags().CenterVertical());
        }

        sizer->AddStretchSpacer();

        {
            wxBitmapButton* const button = create_bitmap_button(panel, "content-content_copy");
            button->SetToolTip(_("Copy all messages to clipboard"));
            button->Bind(wxEVT_BUTTON, &wxMainFrame::OnCopyEvents, this);
            button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateMsgCtrls, this);
            sizer->Add(button, wxSizerFlags().CenterVertical());
        }

        panelSizer->Add(sizer, wxSizerFlags().Expand().Border());
    }

    panel->SetSizerAndFit(panelSizer);
    return panel;
}

wxNotebook* wxMainFrame::create_notebook()
{
    wxNotebook* const notebook = new wxNotebook(this, wxID_ANY);

    notebook->AddPage(create_src_dst_pannel(notebook), _("Source and destination"), true);
    notebook->AddPage(create_metadata_pannel(notebook), _("Metadata"));
    notebook->AddPage(create_messages_panel(notebook), _("Messages"));

    return notebook;
}

wxBoxSizer* wxMainFrame::create_bottom_ctrls()
{
    wxBoxSizer* const sizer = new wxBoxSizer(wxHORIZONTAL);

    sizer->AddStretchSpacer();

    {
        wxButton* const button = new wxButton(this, wxID_ANY, m_execButtonCaptionRun);
        //button->SetFont(wxFont(wxNORMAL_FONT->GetPointSize() + 1, wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxFONTWEIGHT_BOLD));
        button->SetWindowVariant(wxWINDOW_VARIANT_LARGE);
        button->SetToolTip(_("Execute (or kill) mutool utility"));
        button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonRun, this);
        button->Bind(wxEVT_BUTTON, &wxMainFrame::OnExecMuTool, this);
        sizer->Add(button, wxSizerFlags().CentreVertical());
    }

    return sizer;
}

wxMainFrame::wxMainFrame(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxFrame(parent, id, title, pos, size, style),
    m_pPrevLog(nullptr),
    m_logTimestamp(wxLog::GetTimestamp()),
    m_execButtonCaptionRun(_("Run")),
    m_execButtonCaptionKill(_("Kill")),
    m_autoScroll(true),
    m_commonDir(wxFileNameRefData::Get())
{
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
    SetIcons(wxGetApp().GetAppIcon());

    {
        wxBoxSizer* const sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(m_notebook = create_notebook(), wxSizerFlags().Proportion(1).Expand());
        sizer->Add(create_bottom_ctrls(), wxSizerFlags().Border().Expand());
        this->SetSizerAndFit(sizer);
    }

    m_pLog.reset(new LogListBox(m_listBoxMessages));
    m_pPrevLog = wxLog::SetActiveTarget(m_pLog.get());
    wxLog::DisableTimestamp();
    wxLog::EnableLogging();

    {
        const wxVersionInfo libVer = wxGetLibraryVersionInfo();
        const wxJson jsonInfo = wxJson::meta();
        wxLogMessage(_wxS("Simple image to PDF converter"));
        wxLogMessage(_wxS("Powered by M\u03bcPDF library"));
        wxLogMessage(wxEmptyString);
        wxLogMessage("%-10s: %s", _("Version"), wxGetApp().APP_VERSION);
        wxLogMessage("%-10s: %s", _("Author"), wxGetApp().GetVendorDisplayName());
        wxLogMessage("%-10s: %s", _("OS"), wxPlatformInfo::Get().GetOperatingSystemDescription());
        wxLogMessage("%-10s: %s %s", _("Compiler"), INFO_CXX_COMPILER_ID, INFO_CXX_COMPILER_VERSION);
        wxLogMessage("%-10s: %s %s", _("GUI"), libVer.GetVersionString(), libVer.GetCopyright());
        wxLogMessage("%-10s: %s %s %s", wxS("JSON"),
                     wxString(jsonInfo["name"].get<std::string>()),
                     wxString(jsonInfo["version"]["string"].get<std::string>()),
                     wxString(jsonInfo["copyright"].get<std::string>()));
        wxLogMessage("%-10s: %s", _("Source"), wxS("http://github.com/RoEdAl/wxImg2PdfGui"));
    }
    wxGetApp().ShowToolPaths();

    Bind(wxEVT_CLOSE_WINDOW, &wxMainFrame::OnClose, this);
    m_timerIdleWakeUp.Bind(wxEVT_TIMER, &wxMainFrame::OnIdleWakeupTimer, this);
    Bind(wxEVT_THREAD, &wxMainFrame::OnItemUpdated, this);
    SetDropTarget(new DropTarget(this));
}

void wxMainFrame::OnClose(wxCloseEvent& event)
{
    if (event.CanVeto() && m_pProcess)
    {
        wxLogWarning(_("Vetoing window close request - child process is running"));
        event.Veto();
        return;
    }

    wxThread* const thread = GetThread();
    if (event.CanVeto() && thread != nullptr && thread->IsAlive())
    {
        wxLogWarning(_("Vetoing window close request - background worker thread"));
        event.Veto();
        return;
    }

    if (m_pProcess)
    {
    #ifdef __WXMSW__
        kill_console_process(*m_pProcess);
    #else
        wxLogWarning(_("exe[c]: kill %ld"), m_pProcess->GetPid());
        const wxKillError res = wxProcess::Kill(m_pProcess->GetPid(), wxSIGKILL, wxKILL_CHILDREN);
        if (res != wxKILL_OK)
        {
            wxLogWarning(_("Fail to kill process %ld - error code %d"), m_pProcess->GetPid(), res);
        }
    #endif
    }

    #ifdef __WXMSW__
    if (m_pTaskKillProcess)
    {
        kill_console_process(*m_pTaskKillProcess);
    }
    #endif

    if (m_timerIdleWakeUp.IsRunning()) m_timerIdleWakeUp.Stop();

    if (thread != nullptr && thread->IsAlive())
    {
        thread->Delete();
        thread->Wait();
    }

    wxLog::SetTimestamp(m_logTimestamp);
    wxLog::EnableLogging(false);
    wxLog::SetActiveTarget(m_pPrevLog);

    Destroy();
}

void wxMainFrame::ProcessOutErr(bool once)
{
    wxASSERT(m_pProcess);
    const MyProcess* const pProcess = static_cast<MyProcess*>(m_pProcess.get());

    bool         processOutErr = pProcess->HaveOutOrErr();
    const wxChar c = once ? wxS('r') : wxS('f');

    while (processOutErr)
    {
        if (pProcess->IsInputAvailable() && !pProcess->GetTxtInputStream().GetInputStream().Eof())
        {
            const wxString line = pProcess->GetTxtInputStream().ReadLine();
            wxLogInfo("out[%c]: %s", c, line);
        }

        if (pProcess->IsErrorAvailable() && !pProcess->GetTxtErrorStream().GetInputStream().Eof())
        {
            const wxString line = pProcess->GetTxtErrorStream().ReadLine();
            wxLogInfo("err[%c]: %s", c, line);
        }

        processOutErr = once ? false : pProcess->HaveOutOrErr();
    }
}

void wxMainFrame::OnProcessTerminated(wxProcessEvent& event)
{
    if (m_pProcess->GetPid() != event.GetPid())
    {
        wxLogError(wxT("exe[unk]: pid: %d, exit code: %d [%08x]"), event.GetPid(), event.GetExitCode(), event.GetExitCode());
        return;
    }

    m_timerIdleWakeUp.Stop();
    ProcessOutErr();

    wxLogInfo(_("exe[f]: pid: %d, exit code: %d [%08x]"), event.GetPid(), event.GetExitCode(), event.GetExitCode());
    m_pProcess.reset();
    Unbind(wxEVT_IDLE, &wxMainFrame::OnIdle, this);

    delete_temporary_files();

    if (event.GetExitCode() != 0)
    {
        m_notebook->ChangeSelection(m_notebook->GetPageCount() - 1);
    }
}

void wxMainFrame::OnIdleWakeupTimer(wxTimerEvent& WXUNUSED(event))
{
    if (!m_pProcess) return;

    wxWakeUpIdle();
}

void wxMainFrame::OnIdle(wxIdleEvent& event)
{
    if (!m_pProcess) return;

    ProcessOutErr(true);

    MyProcess* const pProcess = static_cast<MyProcess*>(m_pProcess.get());
    event.RequestMore(pProcess->HaveOutOrErr());
}

void wxMainFrame::OnCheckVerbose(wxCommandEvent& event)
{
    wxLog::SetVerbose(event.IsChecked());
}

void wxMainFrame::OnUpdateButtonRun(wxUpdateUIEvent& event)
{
    if (m_pProcess)
    {
    #ifdef __WXMSW__
        event.Enable(!m_pTaskKillProcess);
    #else
        event.Enable(true);
    #endif
        event.SetText(m_execButtonCaptionKill);
        return;
    }

    event.SetText(m_execButtonCaptionRun);
    if (m_listViewInputFiles->GetItemCount() == 0)
    {
        event.Enable(false);
        return;
    }

    event.Enable(!m_textCtrlDst->IsEmpty());
}

namespace
{
    void get_cmd(const wxFileName& exe, const wxString& params, wxString& cmd, wxString& cmdDesc)
    {
        cmd.Clear();
        cmd << '"' << exe.GetFullPath() << "\" " << params;

        cmdDesc.Clear();
        cmdDesc << exe.GetName() << ' ' << params;
    }

    bool have_spaces(const wxString& str, const wxRegEx& spaceChecker)
    {
        if (str.StartsWith("--") || str.StartsWith('-')) return false;

        return spaceChecker.Matches(str);
    }

    wxString options_to_str(const wxArrayString& options)
    {
        if (options.IsEmpty()) return wxEmptyString;

        const wxRegEx spaceChecker("\\p{Xps}", wxRE_NOSUB);

        wxASSERT(spaceChecker.IsValid());

        wxString res;

        for(const auto& i : options)
        {
            if (have_spaces(i, spaceChecker))
            {
                res.Append('\"').Append(i).Append("\" ");
            }
            else
            {
                res.Append(i).Append(' ');
            }
        }

        res.RemoveLast();
        return res;
    }

    bool save_json(const wxFileName& fn, const wxJson& json)
    {
        wxFileOutputStream os(fn.GetFullPath());

        if (os.IsOk())
        {
            wxTextOutputStream stream(os, wxEOL_NATIVE, wxConvUTF8);
            const wxString j = wxString::FromUTF8Unchecked(json.dump());
            stream << j << endl;
            return true;
        }
        else
        {
            wxLogError(_wxS("Fail to save JSON to " ENQUOTED_STR_FMT), fn.GetFullName());
            return false;
        }
    }
}

void wxMainFrame::ExecuteCmd(const wxFileName& exe, const wxString& params, const wxFileName& cwd, const wxArrayFileName& temporaryFiles)
{
    wxASSERT(!cwd.IsOk() || cwd.IsDir());

    if (m_pProcess)
    {
        wxLogWarning(_("exe: Unable to execute command %s %s"), exe.GetName(), params);
        return;
    }

    m_temporaryFiles.clear();
    WX_APPEND_ARRAY(m_temporaryFiles, temporaryFiles);

    wxExecuteEnv env;
    if (cwd.IsOk())
    {
        env.cwd = cwd.GetFullPath();
    }

    MyProcess* const pProcess = new MyProcess;

    m_pProcess.reset(pProcess);
    m_pProcess->Bind(wxEVT_END_PROCESS, &wxMainFrame::OnProcessTerminated, this);

    wxString cmd, cmdDesc;
    get_cmd(exe, params, cmd, cmdDesc);

    const long pid = wxExecute(cmd, wxEXEC_HIDE_CONSOLE | wxEXEC_MAKE_GROUP_LEADER, m_pProcess.get(), &env);

    if (pid == 0)
    {
        wxLogError(_("exe[b]: fail: %s"), cmdDesc);
        m_pProcess.reset();
        return;
    }

    wxLogInfo(_("exe[b]: pid: %ld, cmd: %s"), pid, cmdDesc);

    m_pProcess->CloseOutput();
    pProcess->CreateTxtStreams();
    Bind(wxEVT_IDLE, &wxMainFrame::OnIdle, this);

    m_timerIdleWakeUp.Start(TIMER_IDLE_WAKE_UP_INTERVAL);
}

#ifdef __WXMSW__
void wxMainFrame::ExecuteTaskKill()
{
    if (!m_pProcess)
    {
        return;
    }

    if (m_pTaskKillProcess)
    {
        return;
    }

    const long pid = m_pProcess->GetPid();
    if (!wxProcess::Exists(pid))
    {
        return;
    }

    const wxString cmd = wxString::Format("taskkill.exe /pid %ld", pid);
    const wxString cmdDesc = wxString::Format("taskkill /pid %ld", pid);

    wxProcess* const process = new wxProcess();
    process->SetPriority(wxPRIORITY_MIN);
    m_pTaskKillProcess.reset(process);
    process->Bind(wxEVT_END_PROCESS, &wxMainFrame::OnTaskKillProcessTerminated, this);

    const long killPid = wxExecute(cmd, wxEXEC_HIDE_CONSOLE, process);

    if (killPid == 0)
    {
        wxLogError(_("kill[b]: fail: %s"), cmdDesc);
        m_pTaskKillProcess.reset();
        return;
    }

    wxLogInfo(_("kill[b]: pid: %ld, cmd: %s"), killPid, cmdDesc);
}

void wxMainFrame::OnTaskKillProcessTerminated(wxProcessEvent& event)
{
    if (m_pTaskKillProcess->GetPid() != event.GetPid())
    {
        wxLogError(_("kill[unk]: pid: %d, exit code: %d [%08x]"), event.GetPid(), event.GetExitCode(), event.GetExitCode());
        return;
    }

    if (event.GetExitCode() != 0)
    {
        wxLogInfo(_("kill[f]: pid: %d, exit code: %d [%08x]"), event.GetPid(), event.GetExitCode(), event.GetExitCode());
    }

    m_pTaskKillProcess.reset();
}

#endif

namespace
{
    wxDateTime get_dt(const DateTimePicker* const dtPicker)
    {
        return dtPicker->GetValue();
    }

    wxJson get_dt_array(const wxDateTime& dt)
    {
        if (dt.IsValid())
        {
            return wxJson::array({
                dt.GetYear(), dt.GetMonth(), dt.GetDay(),
                dt.GetHour(), dt.GetMinute()
            });
        }
        else
        {
            return wxJson::array();
        }
    }

    bool is_valid_size(const wxSize& sz)
    {
        return sz.x > 0 && sz.y > 0;
    }
}

void wxMainFrame::build_script(wxJson& json) const
{
    // output file

    const wxFileName outputPath = wxFileName::FileName(m_textCtrlDst->GetValue());
    json["output"] = outputPath.GetFullPath().utf8_string();

    // output options

    wxJson outputOpts = wxJson::array();

    if (m_checkBoxOutputDecompress->GetValue())
    {
        outputOpts.push_back("decompress");
    }

    if (m_checkBoxOutputCompressFonts->GetValue())
    {
        outputOpts.push_back("compress-fonts");
    }

    if (m_checkBoxOutputAscii->GetValue())
    {
        outputOpts.push_back("ascii");
    }

    if (m_checkBoxOutputPretty->GetValue())
    {
        outputOpts.push_back("pretty");
    }

    if (m_checkBoxOutputClean->GetValue())
    {
        outputOpts.push_back("clean");
    }

    if (m_checkBoxOutputSanitize->GetValue())
    {
        outputOpts.push_back("sanitize");
    }

    if (m_checkBoxOutputLinearize->GetValue())
    {
        outputOpts.push_back("linearize");
    }

    json["outputOpts"] = outputOpts;

    // metadata

    wxJson info = wxJson::object();

    if (m_checkBoxMetadataAuthor->GetValue())
    {
        const wxString author = m_textCtrlMetadataAuthor->GetValue();
        if (!author.IsEmpty())
        {
            info["author"] = author.utf8_string();
        }
    }

    if (m_checkBoxMetadataTitle->GetValue())
    {
        const wxString title = m_textCtrlMetadataTitle->GetValue();
        if (!title.IsEmpty())
        {
            info["title"] = title.utf8_string();
        }
    }

    if (m_checkBoxMetadataSubject->GetValue())
    {
        const wxString subject = m_textCtrlMetadataSubject->GetValue();
        if (!subject.IsEmpty())
        {
            info["subject"] = subject.utf8_string();
        }
    }

    if (m_checkBoxMetadataCreator->GetValue())
    {
        const wxString creator = m_textCtrlMetadataCreator->GetValue();
        if (!creator.IsEmpty())
        {
            info["creator"] = creator.utf8_string();
        }
    }

    if (m_checkBoxMetadataCreationDate->GetValue())
    {
        const wxDateTime dt = get_dt(m_dateTimePickerMetadataCreationDate);
        info["creationDate"] = get_dt_array(dt);
    }

    if (m_checkBoxMetadataModDate->GetValue())
    {
        const wxDateTime dt = get_dt(m_dateTimePickerMetadataModDate);
        info["modDate"] = get_dt_array(dt);
    }

    if (info.size() > 0)
    {
        json["info"] = info;
    }

    // source files

    {
        wxJson src = wxJson::array();

        const wxDataViewModel* const dataModel = m_listViewInputFiles->GetModel();
        wxDataViewItemArray elems;
        dataModel->GetChildren(wxDataViewItem(), elems);
        for (const auto& i :elems)
        {
            wxVariant v;
            dataModel->GetValue(v, i, 1);
            if (v.GetType().CmpNoCase(wxS("wxRelativeFileName")) != 0) continue;
            const wxRelativeFileName& rfn = get_variant_custom_val<wxVariantDataRelativeFileName>(v);
            const wxFileName& fn = rfn.GetFileName();
            const wxString ext = fn.GetExt().MakeLower();
            if (ext.CmpNoCase("pdf") == 0)
            {
                wxJson pdf;
                pdf["pdf"] = fn.GetFullPath().utf8_string();
                src.push_back(pdf);
            }
            else if (ext.CmpNoCase("svg") == 0)
            {
                dataModel->GetValue(v, i, 3);
                if (v.GetType().CmpNoCase(wxS("wxResolutionOrScale")) != 0) continue;
                const wxResolutionOrScale& ros = get_variant_custom_val<wxVariantDataResolutionOrScale>(v);
                wxASSERT(!ros.HasResolution());
                float sx, sy;
                ros.GetScale(sx, sy);

                wxJson doc;
                doc["doc"] = fn.GetFullPath().utf8_string();
                if (sx > 0.0f && sy > 0.0f)
                {
                    if (sx == sy)
                    {
                        doc["scale"] = sx;
                    }
                    else
                    {
                        doc["scaleX"] = sx;
                        doc["scaleY"] = sy;
                    }
                }
                src.push_back(doc);
            }
            else // image file
            {
                dataModel->GetValue(v, i, 3);
                if (v.GetType().CmpNoCase(wxS("wxResolutionOrScale")) != 0) continue;
                const wxResolutionOrScale& ros = get_variant_custom_val<wxVariantDataResolutionOrScale>(v);
                wxASSERT(ros.HasResolution());
                const wxSize sz = ros.GetSize();
                if (is_valid_size(sz))
                {
                    wxJson img;
                    img["img"] = fn.GetFullPath().utf8_string();
                    if (sz.x == sz.y)
                    {
                        img["resolution"] = sz.x;
                    }
                    else
                    {
                        img["resolutionX"] = sz.x;
                        img["resolutionY"] = sz.y;
                    }
                    src.push_back(img);
                }
                else
                {
                    src.push_back(fn.GetFullPath().utf8_string());
                }
            }
        }

        json["src"] = src;
    }
}

namespace
{
    wxFileName get_tmp_file_name(const wxFileName& tmpDir, const wxString& fileName)
    {
        wxASSERT(tmpDir.IsDir());
        wxFileName res(tmpDir);
        res.SetFullName(fileName);
        return res;
    }
}

void wxMainFrame::OnExecMuTool(wxCommandEvent& WXUNUSED(event))
{
    if (m_pProcess)
    {
    #ifdef __WXMSW__
        ExecuteTaskKill();
    #else
        const wxKillError res = wxProcess::Kill(m_pProcess->GetPid(), wxSIGKILL, wxKILL_CHILDREN);
        if (res != wxKILL_OK)
        {
            wxLogWarning(_("Fail to kill process %ld - error %d"), m_pProcess->GetPid(), res);
        }
    #endif
        return;
    }

    m_listBoxMessages->Clear();

    wxArrayString params;

    params.Add("run");
    params.Add(wxGetApp().GetScriptPath().GetFullPath());

    const wxFileName tmpDir = wxFileName::DirName(wxStandardPaths::Get().GetTempDir());
    const wxFileName jsonPath = get_tmp_file_name(tmpDir, "~img2pdf.json");
    const wxFileName tmpDocPath = get_tmp_file_name(tmpDir, "~img2pdf.pdf");
    const wxFileName workingDirectory = m_commonDir->GetFileName().IsOk()? m_commonDir->GetFileName() : wxFileName::DirName(wxFileName::GetCwd());

    wxJson script;
    script["tmpDocPath"] = tmpDocPath.GetFullPath().utf8_string();

    build_script(script);
    if (wxLog::GetVerbose())
    {
        const wxString scriptStr = wxString::FromUTF8Unchecked(script.dump(2));
        wxTextInputStreamOnString tis(scriptStr);
        while (!tis.Eof())
        {
            const wxString line = tis.GetStream().ReadLine();
            wxLogVerbose("json: %s", line);
        }
    }
    if (!save_json(jsonPath, script)) return;

    params.Add(jsonPath.GetFullPath());

    wxArrayFileName temporaryFiles;
    temporaryFiles.push_back(jsonPath);
    temporaryFiles.push_back(tmpDocPath);

    ExecuteMuTool(params, workingDirectory, temporaryFiles);
}

void wxMainFrame::ExecuteMuTool(const wxArrayString& args, const wxFileName& cwd, const wxArrayFileName& temporaryFiles)
{
    const wxFileName& mutool = wxGetApp().GetMuToolPath();
    ExecuteCmd(mutool, options_to_str(args), cwd, temporaryFiles);
}

void wxMainFrame::OnDropFiles(const wxArrayString& fileNames)
{
    {
        const wxThread* thread = GetThread();
        if (thread != nullptr && thread->IsAlive())
        {
            wxLogWarning(_("Cannot drop files - background thread is still running"));
            return;
        }
    }

    const wxWindowUpdateLocker locker(m_notebook);

    m_notebook->ChangeSelection(0);
    for (const auto& i : fileNames)
    {
        append_item(m_listViewInputFiles, wxFileName::FileName(i), m_commonDir);
    }

    const wxThreadError threadRes = CreateThread();
    if (threadRes == wxTHREAD_NO_ERROR)
    {
        GetThread()->Run();
    }

    post_focus_list();
}

void wxMainFrame::post_focus_list() const
{
    wxVector<wxVariant> vempty;
    wxScopedPtr<wxThreadEvent> threadEvent(new wxThreadEvent);
    threadEvent->SetPayload(vempty);
    wxQueueEvent(GetEventHandler(), threadEvent.release());
}

void wxMainFrame::OnUpdateButtonAdd(wxUpdateUIEvent& event)
{
    const wxThread* thread = GetThread();
    event.Enable(thread == nullptr || !thread->IsAlive());
}

void wxMainFrame::OnButtonAdd(wxCommandEvent& WXUNUSED(event))
{
    const wxScopedPtr<wxFileDialog> dlgFile(new wxFileDialog(this,
        _("Specify input file"),
        wxEmptyString,
        wxEmptyString,
        _("JPEG files|*.jpg;*.jpeg|PNG files|*.png|SVG files|*.svg|PDF files|*.pdf|JPEG 2000 files|*.jp2|All files|*"),
        wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE));

    if (!dlgFile) return;
    if (dlgFile->ShowModal() != wxID_OK) return;

    wxFileName fileName;
    fileName.AssignDir(dlgFile->GetDirectory());

    wxArrayString fileNames;
    dlgFile->GetFilenames(fileNames);

    {
        wxBusyCursor busy;
        const wxWindowUpdateLocker locker(m_listViewInputFiles);

        for(const auto& i : fileNames)
        {
            fileName.SetFullName(i);
            append_item(m_listViewInputFiles, fileName, m_commonDir);
        }
    }

    const wxThreadError threadRes = CreateThread();
    if (threadRes == wxTHREAD_NO_ERROR)
    {
        GetThread()->Run();
    }

    post_focus_list();
}

void wxMainFrame::OnUpdateButtonDelete(wxUpdateUIEvent& event)
{
    if (m_listViewInputFiles->GetSelectedItemsCount() > 0)
    {
        const wxThread* thread = GetThread();
        event.Enable(thread == nullptr || !thread->IsAlive());
    }
    else
    {
        event.Enable(false);
    }
}

void wxMainFrame::OnButtonDelete(wxCommandEvent& WXUNUSED(event))
{
    wxDataViewItemArray sel;
    m_listViewInputFiles->GetSelections(sel);

    {
        const wxWindowUpdateLocker locker(m_listViewInputFiles);

        for(const wxDataViewItem& i : sel)
        {
            const int row = m_listViewInputFiles->ItemToRow(i);
            if (row == wxNOT_FOUND) continue;
            m_listViewInputFiles->DeleteItem(row);
        }

        const wxDataViewItem emptyItem;
        m_listViewInputFiles->GetModel()->GetChildren(emptyItem, sel);
        if (!sel.empty())
        {
            m_listViewInputFiles->Select(sel.Last());
        }
    }

    const wxThreadError threadRes = CreateThread();
    if (threadRes == wxTHREAD_NO_ERROR)
    {
        GetThread()->Run();
    }

    post_focus_list();
}

void wxMainFrame::OnUpdateButtonSelectAll(wxUpdateUIEvent& event)
{
    if (m_listViewInputFiles->GetItemCount() > 0)
    {
        const wxThread* thread = GetThread();
        event.Enable(thread == nullptr || !thread->IsAlive());
    }
    else
    {
        event.Enable(false);
    }
}

void wxMainFrame::OnButtonSelectAll(wxCommandEvent& WXUNUSED(event))
{
    m_listViewInputFiles->SelectAll();
    post_focus_list();
}

namespace
{
    bool exclusive_bool(const bool b1, const bool b2)
    {
        int cnt = 0;
        if (b1) cnt += 1;
        if (b2) cnt += 1;
        return cnt == 1;
    }

    bool exclusive_bool(const bool b1, const bool b2, const bool b3)
    {
        int cnt = 0;
        if (b1) cnt += 1;
        if (b2) cnt += 1;
        if (b3) cnt += 1;
        return cnt == 1;
    }

    bool exclusive_cnt(const size_t cnt1, const size_t cnt2)
    {
        return exclusive_bool(cnt1 > 0, cnt2 > 0);
    }

    bool exclusive_cnt(const size_t cnt1, const size_t cnt2, const size_t cnt3)
    {
        return exclusive_bool(cnt1 > 0, cnt2 > 0, cnt3 > 0);
    }

    wxSize normalize_size(const wxSize& sz)
    {
        return wxSize(sz.x < 0 ? 0 : sz.x, sz.y < 0 ? 0 : sz.y);
    }

    wxSize get_common_size(const wxVector<wxSize>& asz)
    {
        if (asz.empty()) return wxSize();
        if (asz.size() == 1) return normalize_size(asz[0]);

        wxSize res(asz[0]);
        bool commonW = true, commonH = true;
        for (wxVector<wxSize>::const_iterator i = asz.begin() + 1, end = asz.end(); i != end; ++i)
        {
            if (i->GetWidth() != res.GetWidth()) commonW = false;
            if (i->GetHeight() != res.GetHeight()) commonH = false;
        }

        if (!commonW) res.SetWidth(0);
        if (!commonH) res.SetHeight(0);
        return res;
    }
}

void wxMainFrame::OnUpdateButtonResolutionScale(wxUpdateUIEvent& event)
{
    if (m_listViewInputFiles->GetSelectedItemsCount() > 0)
    {
        {
            const wxThread* thread = GetThread();
            if (thread != nullptr && thread->IsAlive())
            {
                event.Enable(false);
                return;
            }
        }

        wxDataViewItemArray elems;
        m_listViewInputFiles->GetSelections(elems);

        size_t cntImg = 0, cntPdf = 0, cntDoc = 0;
        wxDataViewModel* const dataModel = m_listViewInputFiles->GetModel();

        for (const auto& i : elems)
        {
            wxVariant v;
            dataModel->GetValue(v, i, 1);
            if (v.GetType().CmpNoCase(wxS("wxRelativeFileName")) != 0) continue;
            const wxRelativeFileName& rfn = get_variant_custom_val<wxVariantDataRelativeFileName>(v);
            const wxFileName& fn = rfn.GetFileName();
            const wxString ext = fn.GetExt().MakeLower();
            if (ext.CmpNoCase("pdf") == 0)
            {
                cntPdf += 1;
            }
            else if (ext.CmpNoCase("svg") == 0)
            {
                cntDoc += 1;
            }
            else
            {
                cntImg += 1;
            }
        }

        event.Enable(exclusive_cnt(cntImg, cntPdf, cntDoc) && (cntPdf == 0));
    }
    else
    {
        event.Enable(false);
    }
}

void wxMainFrame::OnDataViewItemActiveted(wxDataViewEvent& event)
{
    switch (event.GetColumn())
    {
        case 0:
        case 1:
        if (wxGetApp().SumatraPdfFound())
        {
            wxVariant v;
            m_listViewInputFiles->GetModel()->GetValue(v, event.GetItem(), 1);
            if (v.GetType().CmpNoCase(wxS("wxRelativeFileName")) != 0) return;
            const wxRelativeFileName& rfn = get_variant_custom_val<wxVariantDataRelativeFileName>(v);
            wxGetApp().RunDocViewer(rfn.GetFileName());
        }
        break;

        case 3:
        {
            wxDataViewModel* const dataModel = m_listViewInputFiles->GetModel();
            wxVariant v;
            dataModel->GetValue(v, event.GetItem(), 3);
            if (v.GetType().CmpNoCase(wxS("wxResolutionOrScale")) != 0) return;
            const wxResolutionOrScale& ros = get_variant_custom_val<wxVariantDataResolutionOrScale>(v);
            if (ros.HasResolution())
            {
                const wxScopedPtr<SizeDialog> dlg(new SizeDialog(this, wxID_ANY, _("Specify image resolution")));
                dlg->SetValue(ros.GetSize());
                const int res = dlg->ShowModal();
                if (res != wxID_OK) return;
                const wxSize newRes = dlg->GetValue();
                dataModel->SetValue(wxVariantDataResolutionOrScale::GetResolution(newRes), event.GetItem(), 3);
            }
            else
            {
                const wxScopedPtr<ScaleDialog> dlg(new ScaleDialog(this, wxID_ANY, _("Specify document scale")));
                dlg->SetValue(ros.GetSize());
                const int res = dlg->ShowModal();
                if (res != wxID_OK) return;
                const wxSize newScale = dlg->GetValue();
                dataModel->SetValue(wxVariantDataResolutionOrScale::GetScale(newScale), event.GetItem(), 3);
            }
            break;
        }
    }
}

void wxMainFrame::OnButtonResolutionScale(wxCommandEvent& WXUNUSED(event))
{
    wxDataViewItemArray elems;
    m_listViewInputFiles->GetSelections(elems);
    wxDataViewModel* const dataModel = m_listViewInputFiles->GetModel();

    wxVector<wxSize> resolutions;
    wxVector<wxSize> scales;
    for (const auto& i : elems)
    {
        wxVariant v;
        dataModel->GetValue(v, i, 3);
        if (v.GetType().CmpNoCase(wxS("wxResolutionOrScale")) != 0) continue;
        const wxResolutionOrScale& ros = get_variant_custom_val<wxVariantDataResolutionOrScale>(v);
        (ros.HasResolution() ? resolutions : scales).push_back(ros.GetSize());
    }

    if (!exclusive_cnt(resolutions.size(), scales.size()))
    {
        return;
    }

    if (!resolutions.empty())
    {
        const wxSize commonRes = get_common_size(resolutions);

        const wxScopedPtr<SizeDialog> dlg(new SizeDialog(this, wxID_ANY, _("Specify image(s) resolution")));
        dlg->SetValue(commonRes);
        const int res = dlg->ShowModal();
        if (res != wxID_OK) return;
        const wxSize newRes = dlg->GetValue();

        {
            const wxWindowUpdateLocker locker(m_listViewInputFiles);
            for (const auto& i : elems)
            {
                dataModel->SetValue(wxVariantDataResolutionOrScale::GetResolution(newRes), i, 3);
            }
        }
    }

    if (!scales.empty())
    {
        const wxSize commonRes = get_common_size(scales);

        const wxScopedPtr<ScaleDialog> dlg(new ScaleDialog(this, wxID_ANY, _("Specify document(s) scale")));
        dlg->SetValue(commonRes);
        const int res = dlg->ShowModal();
        if (res != wxID_OK) return;
        const wxSize newScale = dlg->GetValue();

        {
            const wxWindowUpdateLocker locker(m_listViewInputFiles);
            for (const auto& i : elems)
            {
                dataModel->SetValue(wxVariantDataResolutionOrScale::GetScale(newScale), i, 3);
            }
        }
    }
}

void wxMainFrame::OnButtonClearResolutionScale(wxCommandEvent& WXUNUSED(event))
{
    wxDataViewItemArray elems;
    m_listViewInputFiles->GetSelections(elems);
    wxDataViewModel* const dataModel = m_listViewInputFiles->GetModel();

    int cnt = 0;
    {
        const wxWindowUpdateLocker locker(m_listViewInputFiles);

        const wxSize sizeCleared(-1, -1);
        const wxSize sizeUndetermined(0, 0);

        for (const auto& i : elems)
        {
            wxVariant v;
            dataModel->GetValue(v, i, 1);
            if (v.GetType().CmpNoCase(wxS("wxRelativeFileName")) != 0) continue;
            const wxRelativeFileName& rfn = get_variant_custom_val<wxVariantDataRelativeFileName>(v);
            const wxFileName& fn = rfn.GetFileName();
            const wxString ext = fn.GetExt().MakeLower();
            const bool undeterminedSize = ext.CmpNoCase("pdf") == 0 || ext.CmpNoCase("jp2") == 0 || ext.CmpNoCase("svg") == 0;
            if (undeterminedSize)
            {
                dataModel->SetValue(wxVariantDataResolutionOrScale::GetScale(sizeUndetermined), i, 3);
            }
            else
            {
                dataModel->SetValue(wxVariantDataResolutionOrScale::GetResolution(sizeCleared), i, 3);
            }
            cnt += 1;
        }
    }

    if (cnt > 0)
    {
        const wxThreadError threadRes = CreateThread();
        if (threadRes == wxTHREAD_NO_ERROR)
        {
            GetThread()->Run();
        }

        post_focus_list();
    }
}

void wxMainFrame::OnChooseDst(wxCommandEvent& WXUNUSED(event))
{
    const wxScopedPtr<wxFileDialog> dlgFile(new wxFileDialog(this,
        _("Specify output file"),
        wxEmptyString,
        wxEmptyString,
        _("PDF files|*.pdf|All files|*"),
        wxFD_SAVE));

    if (!dlgFile) return;

    if (dlgFile->ShowModal() == wxID_OK)
    {
        m_textCtrlDst->SetValue(dlgFile->GetPath());
    }
}

void wxMainFrame::OnCheckShowTimestamps(wxCommandEvent& event)
{
    if (event.IsChecked())
    {
        wxLog::SetTimestamp(m_logTimestamp);
    }
    else
    {
        wxLog::DisableTimestamp();
    }
}

void wxMainFrame::OnUpdateRunUiCtrl(wxUpdateUIEvent& event)
{
    event.Enable(!m_pProcess);
}

void wxMainFrame::OnUpdateMsgCtrls(wxUpdateUIEvent& event)
{
    event.Enable(!m_pProcess && (m_listBoxMessages->GetCount() > 0));
}

void wxMainFrame::OnCopyEvents(wxCommandEvent& WXUNUSED(event))
{
    const wxString txt = m_listBoxMessages->GetItemsAsText();
    if (txt.IsEmpty()) return;

    if (wxTheClipboard->Open())
    {
        wxTheClipboard->SetData(new wxTextDataObject(txt));
        wxTheClipboard->Close();
    }
    else
    {
        wxLogWarning(_("Cannot copy data to clipboard"));
    }
}

void wxMainFrame::delete_temporary_files()
{
    for(const auto& fn: m_temporaryFiles)
    {
        if (!fn.FileExists())
        {
            wxLogDebug("Nonexistent temporary file: %s", fn.GetFullName());
            continue;
        }

        if (!wxRemoveFile(fn.GetFullPath()))
        {
            wxLogWarning(_("Unable to delete temporary file: %s"), fn.GetFullName());
        }
    }

    m_temporaryFiles.clear();
}

void wxMainFrame::OnItemUpdated(wxThreadEvent& event)
{
    const wxVector<wxVariant> evPayload = event.GetPayload<wxVector<wxVariant>>();
    switch (evPayload.size())
    {
        case 0:
        {
            m_listViewInputFiles->SetFocus();
            break;
        }

        case 1:
        {
            const wxWindowUpdateLocker locker(m_listViewInputFiles);
            m_commonDir->GetFileName().Assign(get_variant_custom_val<wxVariantDataFileName>(evPayload[0]));
            {
                wxDataViewItemArray elems;
                m_listViewInputFiles->GetModel()->GetChildren(wxDataViewItem(), elems);
                m_listViewInputFiles->GetModel()->ItemsChanged(elems);
            }

            if (m_commonDir->GetFileName().IsOk())
            {
                m_staticTextCommonDir->SetLabel(m_commonDir->GetFileName().GetFullPath().RemoveLast());
                m_staticTextCommonDir->Show();
                m_buttonCommonDir->Show();
            }
            else
            {
                m_staticTextCommonDir->SetLabel(wxEmptyString);
                m_staticTextCommonDir->Show(false);
                m_buttonCommonDir->Show(false);
            }
            break;
        }

        case 3:
        {
            const wxWindowUpdateLocker locker(m_listViewInputFiles);
            const wxDataViewItem item(evPayload[0].GetVoidPtr());

            wxDataViewModel* const dataModel = m_listViewInputFiles->GetModel();
            dataModel->SetValue(evPayload[1], item, 2);
            const wxSize& sz = get_variant_custom_val<wxVariantDataSize>(evPayload[2]);
            dataModel->SetValue(wxVariantDataResolutionOrScale::GetResolution(sz), item, 3);

            break;
        }
    }
}

namespace
{
    bool is_uninitialized(const wxSize& sz)
    {
        return sz.x < 0 || sz.y < 0;
    }

    wxString get_array_item(const wxArrayString& a, const size_t idx)
    {
        const size_t cnt = a.GetCount();
        if (idx >= cnt) return wxEmptyString;
        return a[idx];
    }

    wxFileName truncate_dir_path(const wxFileName& fn, const size_t dirCnt)
    {
        wxFileName res(fn);

        while (res.GetDirCount() > dirCnt)
        {
            res.RemoveLastDir();
        }

        if (res.GetDirCount() == 0)
        {
            res.Clear();
        }

        return res;
    }

    wxFileName find_common_path(const wxVector<wxFileName>& paths)
    {
        if (paths.empty()) return wxFileName();

        const size_t cnt = paths.size();

        wxFileName path;
        path.AssignDir(wxFileName(paths[0]).GetPath());
        if (cnt == 1) return path;
        size_t dirCnt = path.GetDirCount();

        wxVector<wxFileName> fn;

        for (size_t i = 1; i < cnt; ++i)
        {
            wxFileName f;
            f.AssignDir(wxFileName(paths[i]).GetPath());

            const size_t dirCnt1 = f.GetDirCount();

            if (dirCnt1 > dirCnt) dirCnt = dirCnt1;

            fn.push_back(f);
        }

        // check drive
        const wxString vol = path.GetVolume();

        for(const auto& i : fn)
        {
            const wxString jvol = i.GetVolume();
            if (vol.CmpNoCase(jvol) != 0) return wxFileName();
        }

        // check dir components
        for (size_t i = 0; i < dirCnt; ++i)
        {
            const wxString dir = get_array_item(path.GetDirs(), i);

            if (dir.IsEmpty()) return truncate_dir_path(path, i);

            for(const auto& j : fn)
            {
                const wxString jdir = get_array_item(j.GetDirs(), i);

                if (jdir.IsEmpty() || (dir.CmpNoCase(jdir) != 0)) return truncate_dir_path(path, i);
            }
        }

        return truncate_dir_path(path, dirCnt);
    }

    wxThreadEvent* create_thread_event(const wxVector<wxVariant>& payload)
    {
        wxThreadEvent* const ev = new wxThreadEvent();
        ev->SetPayload(payload);
        return ev;
    }
}

wxThread::ExitCode wxMainFrame::Entry()
{
    wxJson src;

    wxDataViewModel* const dataModel = m_listViewInputFiles->GetModel();

    wxDataViewItemArray elems;
    dataModel->GetChildren(wxDataViewItem(), elems);
    wxVector<wxFileName> afn;
    for (const auto& i : elems)
    {
        if (GetThread()->TestDestroy()) break;

        wxVector<wxVariant> vals;
        vals.push_back(i.m_pItem);

        wxVariant v;

        dataModel->GetValue(v, i, 1);
        if (v.GetType().CmpNoCase(wxS("wxRelativeFileName")) != 0) continue;
        const wxRelativeFileName& rfn = get_variant_custom_val<wxVariantDataRelativeFileName>(v);
        afn.push_back(rfn.GetFileName());

        dataModel->GetValue(v, i, 2);
        if (v.GetType().CmpNoCase(wxS("wxSize")) != 0) continue;
        const wxSize& imgSize = get_variant_custom_val<wxVariantDataSize>(v);

        dataModel->GetValue(v, i, 3);
        if (v.GetType().CmpNoCase(wxS("wxResolutionOrScale")) != 0) continue;
        const wxResolutionOrScale& ros = get_variant_custom_val<wxVariantDataResolutionOrScale>(v);
        if (!ros.HasResolution()) continue;
        const wxSize imgResolution = ros.GetSize();

        if (!(is_uninitialized(imgSize) || is_uninitialized(imgResolution))) continue;

        wxImage img;
        if (img.LoadFile(rfn.GetFileName().GetFullPath()))
        {
            const wxSize newImgSize = img.GetSize();

            wxSize newImgResolution(img.GetOptionInt(wxIMAGE_OPTION_RESOLUTIONX), img.GetOptionInt(wxIMAGE_OPTION_RESOLUTIONY));
            switch (img.GetOptionInt(wxIMAGE_OPTION_RESOLUTIONUNIT))
            {
                case wxIMAGE_RESOLUTION_INCHES:
                break;

                case wxIMAGE_RESOLUTION_CM: // -> DPI
                newImgResolution.x = lroundf(2.54f * newImgResolution.x);
                newImgResolution.y = lroundf(2.54f * newImgResolution.y);
                break;

                default: // unknown
                newImgResolution.x = newImgResolution.y = 0;
                break;
            }

            vals.push_back(wxVariantDataSize::Get(newImgSize));
            vals.push_back(wxVariantDataSize::Get(newImgResolution));
        }
        else
        { // initialize anyway
            const wxSize zeroSize(0, 0);
            const wxVariant zeroVSize(wxVariantDataSize::Get(zeroSize));

            vals.push_back(zeroVSize);
            vals.push_back(zeroVSize);
        }

        wxQueueEvent(GetEventHandler(), create_thread_event(vals));
    }

    const wxFileName commonDir = find_common_path(afn);
    wxVector<wxVariant> vals;
    vals.push_back(wxVariantDataFileName::Get(commonDir));
    wxQueueEvent(GetEventHandler(), create_thread_event(vals));

    return (wxThread::ExitCode)0;
}

void wxMainFrame::OnUpdateButtonDocOpen(wxUpdateUIEvent& event)
{
    if (!wxGetApp().SumatraPdfFound())
    {
        event.Enable(false);
        return;
    }

    if (m_listViewInputFiles->GetSelectedItemsCount() == 1)
    {
        const wxThread* thread = GetThread();
        event.Enable(thread == nullptr || !thread->IsAlive());
    }
    else
    {
        event.Enable(false);
    }
}

void wxMainFrame::OnButtonDocOpen(wxCommandEvent& WXUNUSED(event))
{
    const wxFileName& sumatraPdf = wxGetApp().GetSumatraPdfPath();
    if (!sumatraPdf.IsOk()) return;

    const wxDataViewItem selItem = m_listViewInputFiles->GetSelection();
    if (!selItem.IsOk()) return;

    wxVariant v;
    m_listViewInputFiles->GetModel()->GetValue(v, selItem, 1);
    if (v.GetType().CmpNoCase(wxS("wxRelativeFileName")) != 0) return;
    const wxRelativeFileName& rfn = get_variant_custom_val<wxVariantDataRelativeFileName>(v);
    wxGetApp().RunDocViewer(rfn.GetFileName());
}

void wxMainFrame::OnUpdateButtonCopyToDst(wxUpdateUIEvent& event)
{
    if (m_pProcess)
    {
        event.Enable(false);
        return;
    }

    event.Enable(m_commonDir->GetFileName().IsOk());
}

void wxMainFrame::OnButtonCopyToDst(wxCommandEvent& WXUNUSED(event))
{
    const wxFileName& commonDir = m_commonDir->GetFileName();
    if (!commonDir.IsOk()) return;

    const wxString dst = m_textCtrlDst->GetValue();
    const wxFileName dstFn = wxFileName::FileName(dst);

    if (dst.IsEmpty() || wxFileName::IsDirWritable(dst) || !dstFn.IsOk())
    {
        wxFileName fn;
        fn.AssignDir(commonDir.GetPath());
        fn.SetFullName(DEFAULT_OUTPUT_FILE_NAME);
        m_textCtrlDst->SetValue(fn.GetFullPath());
    }
    else
    {
        wxFileName fn(dstFn);
        fn.SetPath(commonDir.GetPath());
        m_textCtrlDst->SetValue(fn.GetFullPath());
    }
}
