/*
 *      wxMainFrame.cpp
 */

#include <app_config.h>
#include "wxApp.h"
#include "wxMainFrame.h"
#include "SizeDialog.h"

namespace
{
    const int DEF_MARGIN = 2;
    const int AUTO_SCROLL_UPDATE_INTERVAL = 2000;
    const int TIMER_IDLE_WAKE_UP_INTERVAL = 250;
    const int TIMER_LAYOUT = 2000;

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
        return wxSizerFlags().CenterVertical().Border(wxRIGHT, wnd->FromDIP(DEF_MARGIN)).Proportion(0);
    }

    wxSizerFlags get_middle_crtl_sizer_flags(wxWindow* wnd)
    {
        return wxSizerFlags().CenterVertical().Border(wxLEFT | wxRIGHT, wnd->FromDIP(DEF_MARGIN)).Proportion(0);
    }

    wxSizerFlags get_left_exp_crtl_sizer_flags(wxWindow* wnd)
    {
        return wxSizerFlags().CenterVertical().Border(wxRIGHT, wnd->FromDIP(DEF_MARGIN)).Proportion(1);
    }

    wxSizerFlags get_right_crtl_sizer_flags(wxWindow* wnd)
    {
        return wxSizerFlags().CenterVertical().Border(wxLEFT, wnd->FromDIP(DEF_MARGIN)).Proportion(0);
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

    wxBitmapButton* create_bitmap_button(const wxStaticBoxSizer* parentSizer, const wxIconBundle& iconBundle)
    {
        return new wxBitmapButton(parentSizer->GetStaticBox(), wxID_ANY, wxBitmapBundle::FromIconBundle(iconBundle));
    }

    wxBitmapButton* create_bitmap_button(wxWindow* parent, const wxIconBundle& iconBundle)
    {
        return new wxBitmapButton(parent, wxID_ANY, wxBitmapBundle::FromIconBundle(iconBundle));
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

    wxStaticText* create_ro_text_ctrl(wxWindow* parent)
    {
        wxStaticText* const res = new wxStaticText(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT | wxST_ELLIPSIZE_MIDDLE);
        return res;
    }

    wxStaticText* create_ro_text_ctrl(const wxStaticBoxSizer* parentSizer)
    {
        return create_ro_text_ctrl(parentSizer->GetStaticBox());
    }

    wxStaticLine* create_horizontal_static_line(wxWindow* parent)
    {
        return new wxStaticLine(parent, wxID_ANY, wxDefaultPosition, wxSize(0, parent->FromDIP(1)), wxLI_HORIZONTAL);
    }

    wxSizerFlags get_horizontal_static_line_sizer_flags(wxWindow* wnd)
    {
        return wxSizerFlags().Expand().Border(wxTOP | wxBOTTOM, wnd->FromDIP(2));
    }

    class CollapsiblePaneUiUpdater
    {
        public:

        CollapsiblePaneUiUpdater(wxWindow* wnd)
            :m_wnd(wnd)
        {
        }

        CollapsiblePaneUiUpdater(const CollapsiblePaneUiUpdater& uiUpdater)
            :m_wnd(uiUpdater.m_wnd)
        {
        }

        bool operator==(const CollapsiblePaneUiUpdater& uiUpdater) const
        {
            return m_wnd == uiUpdater.m_wnd;
        }

        bool operator!=(const CollapsiblePaneUiUpdater& uiUpdater) const
        {
            return m_wnd != uiUpdater.m_wnd;
        }

        void operator()(wxCollapsiblePaneEvent& WXUNUSED(event)) const
        {
            m_wnd->Layout();
        }

        private:

        wxWindow* m_wnd;
    };

    wxSize calc_text_size(int charWidth)
    {
        wxScreenDC dc;
        wxString txt(wxUniChar(0x2013), charWidth);
        const wxSize extent = dc.GetTextExtent(txt);
        return wxSize(extent.GetWidth(), -1);
    }

    wxSize calc_text_size(int charWidth, int charHeight)
    {
        wxScreenDC dc;
        wxString txt(wxUniChar(0x2013), charWidth);
        const wxSize extent = dc.GetTextExtent(txt);
        return wxSize(extent.GetWidth(), extent.GetHeight() * charHeight);
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
        fn.SetFullName("output.pdf");
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

        CheckBoxUiUpdater(wxCheckBox* checkBox)
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

        wxCheckBox* m_checkBox;

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
            wxBusyCursor busy;
            m_pMainFrame->OnDropFiles(filenames);
            return true;
        }

        private:

        wxMainFrame* const m_pMainFrame;
    };
}

wxPanel* wxMainFrame::create_src_dst_pannel(wxNotebook* notebook, const wxFont& toolFont, const wxFont& toolFontEx, const wxSizerFlags& btnLeft, const wxSizerFlags& btnMiddle, const wxSizerFlags& btnLeftExp, const wxSizerFlags& btnRight)
{
    wxPanel* const    panel = new wxPanel(notebook);
    wxBoxSizer* const panelSizer = new wxBoxSizer(wxVERTICAL);

    {
        wxStaticBoxSizer* const sizer = create_static_box_sizer(panel, _("Sources"), wxVERTICAL);
        {
            wxBoxSizer* const innerSizer = new wxBoxSizer(wxHORIZONTAL);

            {
                wxBoxSizer* const vinnerSizer = new wxBoxSizer(wxVERTICAL);

                {
                    m_listViewInputFiles = new wxDataViewListCtrl(sizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_VERT_RULES | wxDV_MULTIPLE | wxDV_ROW_LINES);

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

                    vinnerSizer->Add(m_listViewInputFiles, wxSizerFlags().Proportion(1).Expand());
                }

                m_staticTextCommonDir = create_ro_text_ctrl(panel);
                m_staticTextCommonDir->SetFont(toolFont);
                m_staticTextCommonDir->Hide();

                vinnerSizer->Add(m_staticTextCommonDir, 0, wxTOP | wxBOTTOM, btnLeft.GetBorderInPixels() * 2);

                innerSizer->Add(vinnerSizer, wxSizerFlags().Expand().Proportion(1));
                m_sizerInputFiles = vinnerSizer;
            }

            {
                wxBoxSizer* const vinnerSizer = new wxBoxSizer(wxVERTICAL);

                {
                    const wxIconBundle iconBundle("ico_add", nullptr);
                    wxBitmapButton* const button = create_bitmap_button(sizer, iconBundle);
                    button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonAdd, this);
                    button->Bind(wxEVT_BUTTON, &wxMainFrame::OnButtonAdd, this);
                    vinnerSizer->Add(button, wxSizerFlags().CentreHorizontal());
                }

                {
                    const wxIconBundle iconBundle("ico_remove", nullptr);
                    wxBitmapButton* const button = create_bitmap_button(sizer, iconBundle);
                    button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonDelete, this);
                    button->Bind(wxEVT_BUTTON, &wxMainFrame::OnButtonDelete, this);
                    vinnerSizer->Add(button, wxSizerFlags().CenterHorizontal());
                }

                {
                    wxStaticLine* const staticLine = create_horizontal_static_line(sizer->GetStaticBox());
                    vinnerSizer->Add(staticLine, get_horizontal_static_line_sizer_flags(sizer->GetStaticBox()));
                }

                {
                    const wxIconBundle iconBundle("ico_acpect_ratio", nullptr);
                    wxBitmapButton* const button = create_bitmap_button(sizer, iconBundle);
                    button->SetToolTip(_("Change resolution/Scale"));
                    button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonResolutionScale, this);
                    button->Bind(wxEVT_BUTTON, &wxMainFrame::OnButtonResolutionScale, this);
                    vinnerSizer->Add(button, wxSizerFlags().CenterHorizontal());
                }

                {
                    const wxIconBundle iconBundle("ico_clear", nullptr);
                    wxBitmapButton* const button = create_bitmap_button(sizer, iconBundle);
                    button->SetToolTip(_("Use original image resolution/Do not scale image"));
                    button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonResolutionScale, this);
                    button->Bind(wxEVT_BUTTON, &wxMainFrame::OnButtonClearResolutionScale, this);
                    vinnerSizer->Add(button, wxSizerFlags().CenterHorizontal());
                }

                innerSizer->Add(vinnerSizer, wxSizerFlags().Top().Border(wxLEFT, btnLeft.GetBorderInPixels()));
            }

            sizer->Add(innerSizer, wxSizerFlags().Expand().Proportion(1));
        }

        panelSizer->Add(sizer, wxSizerFlags().Expand().Proportion(1));
    }

    {
        wxStaticBoxSizer* const sizer = create_static_box_sizer(panel, _("Destination"), wxVERTICAL);
        //sizer->GetStaticBox()->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateDst, this);

        {
            wxBoxSizer* const innerSizer = new wxBoxSizer(wxHORIZONTAL);

            m_textCtrlDst = create_text_ctrl(sizer, wxEmptyString, 1024);
            const wxFileName fn = get_default_output_fn();
            m_textCtrlDst->SetValue(fn.GetFullPath());
            //m_textCtrlDst->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateCtrlDst, this);
            innerSizer->Add(m_textCtrlDst, btnLeftExp);

            const wxIconBundle iconBundle("ico_more_horiz", nullptr);
            wxBitmapButton* const button = create_bitmap_button(sizer, iconBundle);
            button->Bind(wxEVT_BUTTON, &wxMainFrame::OnChooseDst, this);
            innerSizer->Add(button, btnRight);

            sizer->Add(innerSizer, wxSizerFlags().Expand());
        }

        {
            wxFlexGridSizer* const innerSizer = new wxFlexGridSizer(4);

            m_checkBoxOutputDecompress = create_checkbox(sizer, _T("Decompress"));
            m_checkBoxOutputDecompress->SetFont(toolFont);
            m_checkBoxOutputDecompress->SetToolTip(_T("Decompress all streams"));
            innerSizer->Add(m_checkBoxOutputDecompress, btnLeft);

            m_checkBoxOutputCompressFonts = create_checkbox(sizer, _T("Compress fonts"), true);
            m_checkBoxOutputCompressFonts->SetFont(toolFont);
            m_checkBoxOutputCompressFonts->SetToolTip(_T("Compress fonts"));
            innerSizer->Add(m_checkBoxOutputCompressFonts, btnLeft);

            m_checkBoxOutputAscii = create_checkbox(sizer, _T("ASCII"));
            m_checkBoxOutputAscii->SetFont(toolFont);
            m_checkBoxOutputAscii->SetToolTip(_T("ASCII hex encode binary streams"));
            innerSizer->Add(m_checkBoxOutputAscii, btnLeft);

            m_checkBoxOutputPretty = create_checkbox(sizer, _T("Pretty"), true);
            m_checkBoxOutputPretty->SetFont(toolFont);
            m_checkBoxOutputPretty->SetToolTip(_wxS("Pretty\u2011print objects with indentation"));
            innerSizer->Add(m_checkBoxOutputPretty, btnLeft);

            m_checkBoxOutputClean = create_checkbox(sizer, _T("Clean"), true);
            m_checkBoxOutputClean->SetFont(toolFont);
            m_checkBoxOutputClean->SetToolTip(_wxS("Pretty\u2011print graphics commands in content streams"));
            innerSizer->Add(m_checkBoxOutputClean, btnLeft);

            m_checkBoxOutputSanitize = create_checkbox(sizer, _T("Sanitize"), true);
            m_checkBoxOutputSanitize->SetFont(toolFont);
            m_checkBoxOutputSanitize->SetToolTip(_T("Sanitize graphics commands in content streams"));
            innerSizer->Add(m_checkBoxOutputSanitize, btnLeft);

            m_checkBoxOutputLinearize = create_checkbox(sizer, _T("Linearize"));
            m_checkBoxOutputLinearize->SetFont(toolFont);
            m_checkBoxOutputLinearize->SetToolTip(_T("Optimize for web browsers"));
            innerSizer->Add(m_checkBoxOutputLinearize, btnLeft);

            sizer->Add(innerSizer, wxSizerFlags().Border(wxTOP, btnLeft.GetBorderInPixels()));
        }

        panelSizer->Add(sizer, wxSizerFlags().Expand());
    }

    panel->SetSizerAndFit(panelSizer);
    return panel;
}

wxPanel* wxMainFrame::create_metadata_pannel(wxNotebook* notebook, const wxFont& toolFont, const wxSizerFlags& btnLeft, const wxSizerFlags& btnMiddle, const wxSizerFlags& centerVertical)
{
    wxPanel* const    panel = new wxPanel(notebook);
    wxFlexGridSizer* const panelSizer = new wxFlexGridSizer(0, 2, btnLeft.GetBorderInPixels() * 2, btnLeft.GetBorderInPixels() * 2);
    panelSizer->AddGrowableCol(1);

    {
        const wxSizerFlags textCtrlSizerFlags = wxSizerFlags(centerVertical).Expand().Proportion(1);

        m_checkBoxMetadataAuthor = create_checkbox(panel, _("Author"));
        panelSizer->Add(m_checkBoxMetadataAuthor, btnLeft);

        m_textCtrlMetadataAuthor = create_text_ctrl(panel);
        m_textCtrlMetadataAuthor->Bind(wxEVT_UPDATE_UI, CheckBoxUiUpdater(m_checkBoxMetadataAuthor));
        panelSizer->Add(m_textCtrlMetadataAuthor, textCtrlSizerFlags);

        m_checkBoxMetadataTitle = create_checkbox(panel, _("Title"));
        panelSizer->Add(m_checkBoxMetadataTitle, btnLeft);

        m_textCtrlMetadataTitle = create_text_ctrl(panel);
        m_textCtrlMetadataTitle->Bind(wxEVT_UPDATE_UI, CheckBoxUiUpdater(m_checkBoxMetadataTitle));
        panelSizer->Add(m_textCtrlMetadataTitle, textCtrlSizerFlags);

        m_checkBoxMetadataSubject = create_checkbox(panel, _("Subject"));
        panelSizer->Add(m_checkBoxMetadataSubject, btnLeft);

        m_textCtrlMetadataSubject = create_text_ctrl(panel);
        m_textCtrlMetadataSubject->Bind(wxEVT_UPDATE_UI, CheckBoxUiUpdater(m_checkBoxMetadataSubject));
        panelSizer->Add(m_textCtrlMetadataSubject, textCtrlSizerFlags);

        m_checkBoxMetadataCreator = create_checkbox(panel, _("Creator"));
        panelSizer->Add(m_checkBoxMetadataCreator, btnLeft);

        m_textCtrlMetadataCreator = create_text_ctrl(panel, wxGetApp().GetScriptPath().GetFullName());
        m_textCtrlMetadataCreator->Bind(wxEVT_UPDATE_UI, CheckBoxUiUpdater(m_checkBoxMetadataCreator));
        panelSizer->Add(m_textCtrlMetadataCreator, textCtrlSizerFlags);

        m_checkBoxMetadataCreationDate = create_checkbox(panel, _("Creation date"));
        panelSizer->Add(m_checkBoxMetadataCreationDate, btnLeft);

        {
            const CheckBoxUiUpdater uiUpdater(m_checkBoxMetadataCreationDate);
            wxBoxSizer* const innerSizer = new wxBoxSizer(wxHORIZONTAL);

            m_datePickerMetadataCreationDate = new wxDatePickerCtrl(panel, wxID_ANY, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
            m_datePickerMetadataCreationDate->Bind(wxEVT_UPDATE_UI, uiUpdater);
            innerSizer->Add(m_datePickerMetadataCreationDate, btnLeft);

            m_timePickerMetadataCreationDate = new wxTimePickerCtrl(panel, wxID_ANY);
            m_timePickerMetadataCreationDate->Bind(wxEVT_UPDATE_UI, uiUpdater);
            innerSizer->Add(m_timePickerMetadataCreationDate, btnMiddle);

            panelSizer->Add(innerSizer, textCtrlSizerFlags);
        }

        m_checkBoxMetadataModDate = create_checkbox(panel, _("Modification date"));
        panelSizer->Add(m_checkBoxMetadataModDate, btnLeft);

        {
            const CheckBoxUiUpdater uiUpdater(m_checkBoxMetadataModDate);
            wxBoxSizer* const innerSizer = new wxBoxSizer(wxHORIZONTAL);

            m_datePickerMetadataModDate = new wxDatePickerCtrl(panel, wxID_ANY, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
            m_datePickerMetadataModDate->Bind(wxEVT_UPDATE_UI, uiUpdater);
            innerSizer->Add(m_datePickerMetadataModDate, btnLeft);

            m_timePickerMetadataModDate = new wxTimePickerCtrl(panel, wxID_ANY);
            m_timePickerMetadataModDate->Bind(wxEVT_UPDATE_UI, uiUpdater);
            innerSizer->Add(m_timePickerMetadataModDate, btnMiddle);

            panelSizer->Add(innerSizer, textCtrlSizerFlags);
        }
    }

    panel->SetSizerAndFit(panelSizer);
    return panel;
}

wxPanel* wxMainFrame::create_messages_panel(wxNotebook* notebook, const wxFont& toolFont, const wxSizerFlags& btnLeft, const wxSizerFlags& btnMiddle, const wxSizerFlags& btnRight, const wxSizerFlags& centerVertical)
{
    wxPanel* const    panel = new wxPanel(notebook);
    wxBoxSizer* const panelSizer = new wxBoxSizer(wxVERTICAL);

    wxCollapsiblePane* const collapsiblePane = new wxCollapsiblePane(panel, wxID_ANY, _("Tools"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxCP_NO_TLW_RESIZE);
    if (collapsiblePane->GetControlWidget() != nullptr)
    {
        collapsiblePane->GetControlWidget()->SetFont(toolFont);
    }
    collapsiblePane->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, CollapsiblePaneUiUpdater(panel));

    {
        wxWindow* const insPane = collapsiblePane->GetPane();
        insPane->SetFont(toolFont);
        {
            wxBoxSizer* const innerSizer = new wxBoxSizer(wxHORIZONTAL);
            innerSizer->AddStretchSpacer();

            {
                wxCheckBox* const checkBox = create_checkbox(insPane, _("Auto scroll"), m_autoScroll);
                checkBox->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateRunUiCtrl, this);
                checkBox->Bind(wxEVT_CHECKBOX, &wxMainFrame::OnCheckAutoScroll, this);
                innerSizer->Add(checkBox, btnRight);
            }

            {
                wxCheckBox* const checkBox = create_checkbox(insPane, _("Show timestamps"));
                checkBox->SetToolTip(_("Show/hide message timestamps"));
                checkBox->Bind(wxEVT_CHECKBOX, &wxMainFrame::OnCheckShowTimestamps, this);
                checkBox->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateRunUiCtrl, this);
                innerSizer->Add(checkBox, btnRight);
                m_checkBoxShowTimestamps = checkBox;
            }

            insPane->SetSizer(innerSizer);
        }
        panelSizer->Add(collapsiblePane, 0, wxEXPAND | wxLEFT | wxRIGHT, wxSizerFlags::GetDefaultBorder());
    }

    {
        const wxSize listMinSize = calc_text_size(80, 20);
        m_listBoxMessages = new ListBox(panel);
        m_listBoxMessages->SetSizeHints(listMinSize);
        panelSizer->Add(m_listBoxMessages, 1, wxEXPAND | wxALL, wxSizerFlags::GetDefaultBorder());
    }

    {
        wxBoxSizer* const sizer = new wxBoxSizer(wxHORIZONTAL);

        {
            wxStaticText* const staticTxt = create_static_text(panel, wxEmptyString);
            staticTxt->SetFont(toolFont);
            staticTxt->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateMsgCnt, this);
            sizer->Add(staticTxt, centerVertical);
        }

        sizer->AddStretchSpacer();

        {
            const wxIconBundle iconBundle("ico_content_copy", nullptr);
            wxBitmapButton* const button = create_bitmap_button(panel, iconBundle);
            button->SetToolTip(_("Copy all messages to clipboard"));
            button->Bind(wxEVT_BUTTON, &wxMainFrame::OnCopyEvents, this);
            button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateMsgCtrls, this);
            sizer->Add(button, centerVertical);
        }

        panelSizer->Add(sizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, wxSizerFlags::GetDefaultBorder());
    }

    panel->SetSizerAndFit(panelSizer);
    return panel;
}

wxNotebook* wxMainFrame::create_notebook(const wxFont& toolFont, const wxFont& toolFontEx, const wxSizerFlags& btnLeft, const wxSizerFlags& btnMiddle, const wxSizerFlags& btnLeftExp, const wxSizerFlags& btnRight, const wxSizerFlags& centerVertical)
{
    wxNotebook* const notebook = new wxNotebook(this, wxID_ANY);

    notebook->AddPage(create_src_dst_pannel(notebook, toolFont, toolFontEx, btnLeft, btnMiddle, btnLeftExp, btnRight), _("Source and destination"), true);
    notebook->AddPage(create_metadata_pannel(notebook, toolFont, btnLeft, btnMiddle, centerVertical), _("Metadata"));
    notebook->AddPage(create_messages_panel(notebook, toolFont, btnLeft, btnMiddle, btnRight, centerVertical), _("Messages"));

    return notebook;
}

wxBoxSizer* wxMainFrame::create_bottom_ctrls(const wxFont& toolFont, const wxSizerFlags& btnLeft, const wxSizerFlags& btnMiddle, const wxSizerFlags& btnRight, const wxSizerFlags& centerVertical)
{
    wxBoxSizer* const sizer = new wxBoxSizer(wxHORIZONTAL);

    {
        wxBoxSizer* const innerSizer = new wxBoxSizer(wxVERTICAL);

        m_checkBoxVerbose = create_checkbox(this, _("Verbose mode"), wxLog::GetVerbose());
        m_checkBoxVerbose->SetFont(toolFont);
        m_checkBoxVerbose->Bind(wxEVT_CHECKBOX, &wxMainFrame::OnCheckVerbose, this);
        innerSizer->Add(m_checkBoxVerbose);

        m_checkBoxSwitchToMessagesPane = create_checkbox(this, _("Switch to Messages tab"));
        m_checkBoxSwitchToMessagesPane->SetFont(toolFont);
        m_checkBoxSwitchToMessagesPane->SetToolTip(_("Switch to Messages tab before execution"));
        innerSizer->Add(m_checkBoxSwitchToMessagesPane);

        sizer->Add(innerSizer, centerVertical);
    }

    sizer->AddStretchSpacer();

    {
        wxButton* const button = new wxButton(this, wxID_ANY, m_execButtonCaptionRun);
        button->SetFont(wxFont(wxNORMAL_FONT->GetPointSize() + 1, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
        button->SetToolTip(_("Execute (or kill) mutool utility"));
        button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonRun, this);
        button->Bind(wxEVT_BUTTON, &wxMainFrame::OnExecMuTool, this);
        sizer->Add(button, centerVertical);
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
        const wxFont toolFont(wxSMALL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        const wxFont toolFontEx(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);

        const wxSizerFlags btnLeft(get_left_ctrl_sizer_flags(this));
        const wxSizerFlags btnMiddle(get_middle_crtl_sizer_flags(this));
        const wxSizerFlags btnLeftExp(get_left_exp_crtl_sizer_flags(this));
        const wxSizerFlags btnRight(get_right_crtl_sizer_flags(this));
        const wxSizerFlags centerVertical(get_vertical_allign_sizer_flags());

        wxBoxSizer* const sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(m_notebook = create_notebook(toolFont, toolFontEx, btnLeft, btnMiddle, btnLeftExp, btnRight, centerVertical), 1, wxEXPAND);
        sizer->Add(create_bottom_ctrls(toolFont, btnLeft, btnMiddle, btnRight, centerVertical), 0, wxEXPAND | wxALL, FromDIP(DEF_MARGIN*2));
        this->SetSizerAndFit(sizer);
    }

    m_pLog.reset(new LogListBox(m_listBoxMessages));
    m_pNoScrollLog.reset(new SimpleLogListBox(m_listBoxMessages));

    m_pPrevLog = wxLog::SetActiveTarget(m_autoScroll ? m_pLog.get() : m_pNoScrollLog.get());
    wxLog::DisableTimestamp();
    wxLog::EnableLogging();

    wxLogInfo(_("Simple image to PDF converter"));
    wxLogInfo(_("Version: %s"), wxGetApp().APP_VERSION);
    wxLogInfo(_("Author: %s"), wxGetApp().GetVendorDisplayName());
    wxLogInfo(_("Operating system: %s"), wxPlatformInfo::Get().GetOperatingSystemDescription());
    wxLogInfo(_("Compiler: %s %s"), INFO_CXX_COMPILER_ID, INFO_CXX_COMPILER_VERSION);
    wxLogInfo(_("Compiled on: %s %s (%s)"), INFO_HOST_SYSTEM_NAME, INFO_HOST_SYSTEM_VERSION, INFO_HOST_SYSTEM_PROCESSOR);

    wxGetApp().ShowToolPaths();

    Bind(wxEVT_CLOSE_WINDOW, &wxMainFrame::OnClose, this);

    m_timerIdleWakeUp.Bind(wxEVT_TIMER, &wxMainFrame::OnIdleWakeupTimer, this);
    m_timerAutoScroll.Bind(wxEVT_TIMER, &wxMainFrame::OnAutoScrollTimer, this);
    
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
    if (m_timerAutoScroll.IsRunning()) m_timerAutoScroll.Stop();

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
    m_timerAutoScroll.Stop();

    ProcessOutErr();

    wxLog::SetActiveTarget(m_autoScroll ? m_pLog.get() : m_pNoScrollLog.get());

    wxLogInfo(_("exe[f]: pid: %d, exit code: %d [%08x]"), event.GetPid(), event.GetExitCode(), event.GetExitCode());
    m_pProcess.reset();
    Unbind(wxEVT_IDLE, &wxMainFrame::OnIdle, this);

    delete_temporary_files();
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

void wxMainFrame::OnAutoScrollTimer(wxTimerEvent& WXUNUSED(event))
{
    m_listBoxMessages->ShowLastItem();
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
    if (m_listBoxMessages->IsEmpty())
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
            const wxString j = wxString::FromUTF8Unchecked(json.dump(2));
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

    if (m_checkBoxSwitchToMessagesPane->GetValue())
    {
        m_notebook->ChangeSelection(m_notebook->GetPageCount() - 1);
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

    m_listBoxMessages->Clear();

    wxLogInfo(_("exe[b]: pid: %ld, cmd: %s"), pid, cmdDesc);

    m_pProcess->CloseOutput();
    pProcess->CreateTxtStreams();
    wxLog::SetActiveTarget(m_pNoScrollLog.get());
    Bind(wxEVT_IDLE, &wxMainFrame::OnIdle, this);

    m_timerIdleWakeUp.Start(TIMER_IDLE_WAKE_UP_INTERVAL);
    if (m_autoScroll) m_timerAutoScroll.Start(AUTO_SCROLL_UPDATE_INTERVAL);
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
    wxDateTime get_dt(const wxDatePickerCtrl* const datePicker, const wxTimePickerCtrl* const timePicker)
    {
        wxDateTime dt = datePicker->GetValue();
        dt = dt.GetDateOnly();

        int hour, min, sec;
        timePicker->GetTime(&hour, &min, &sec);

        dt.SetHour(hour);
        dt.SetMinute(min);
        dt.SetSecond(sec);

        return dt;
    }

    wxJson get_dt_array(const wxDateTime& dt)
    {
        return wxJson::array({
            dt.GetYear(), dt.GetMonth(), dt.GetDay(),
            dt.GetHour(), dt.GetMinute()
        });
    }

    bool is_valid_size(const wxSize& sz)
    {
        return sz.x > 0 && sz.y > 0;
    }
}

void wxMainFrame::build_script(wxJson& json, wxFileName& workingDirectory) const
{
    // output file, working directory

    const wxFileName outputPath = wxFileName::FileName(m_textCtrlDst->GetValue());
    json["output"] = outputPath.GetFullPath().utf8_string();
    workingDirectory = wxFileName::DirName(outputPath.GetPath());

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
        const wxDateTime dt = get_dt(m_datePickerMetadataCreationDate, m_timePickerMetadataCreationDate);
        info["creationDate"] = get_dt_array(dt);
    }

    if (m_checkBoxMetadataModDate->GetValue())
    {
        const wxDateTime dt = get_dt(m_datePickerMetadataModDate, m_timePickerMetadataModDate);
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

    wxArrayString params;

    params.Add("run");
    params.Add(wxGetApp().GetScriptPath().GetFullPath());

    const wxFileName tmpDir = wxFileName::DirName(wxStandardPaths::Get().GetTempDir());

    wxFileName jsonPath(tmpDir);
    jsonPath.SetFullName("~img2pdf.json");

    wxFileName tmpDocPath(tmpDir);
    tmpDocPath.SetFullName("~img2pdf.pdf");

    wxFileName workingDirectory = wxFileName::DirName(wxFileName::GetCwd());
    wxJson script;
    script["tmpDocPath"] = tmpDocPath.GetFullPath().utf8_string();

    build_script(script, workingDirectory);
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
    wxScopedPtr<wxFileDialog> dlgFile(new wxFileDialog(this,
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

        wxScopedPtr<SizeDialog> dlg(new SizeDialog(this, wxID_ANY, _("Specify image(s) resolution")));
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
    wxScopedPtr<wxFileDialog> dlgFile(new wxFileDialog(this,
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

void wxMainFrame::OnUpdateMsgCnt(wxUpdateUIEvent& event)
{
    const int cnt = m_listBoxMessages->GetCount();
    wxString txt;
    if (m_pProcess)
    {
    #ifdef __WXMSW__
        if (m_pTaskKillProcess)
        {
            txt = wxString::Format(_wxS("pid:\u2009%ld [killed],\u2009"), m_pProcess->GetPid());
        }
        else
        {
            txt = wxString::Format(_wxS("pid:\u2009%ld,\u2009"), m_pProcess->GetPid());
        }
    #else
        txt = wxString::Format(_wxS("pid:\u2009%ld,\u2009"), m_pProcess->GetPid());
    #endif
    }

    switch (cnt)
    {
        case 0:
        break;

        case 1:
        txt << _("one message");

        default:
        txt << cnt << _(" messages");
        break;
    }

    if (cnt >= 1000)
    {
        txt << wxS("\u2009\u203C");
    }

    event.SetText(txt);
    event.Enable(!m_pProcess && (cnt > 0));
}

void wxMainFrame::OnCheckAutoScroll(wxCommandEvent& event)
{
    m_autoScroll = event.IsChecked();

    if (m_pProcess)
    {
        wxLog::SetActiveTarget(m_pNoScrollLog.get());
        if (m_autoScroll)
        {
            m_timerAutoScroll.Start(AUTO_SCROLL_UPDATE_INTERVAL);
        }
        else
        {
            m_timerAutoScroll.Stop();
        }
    }
    else
    {
        wxLog::SetActiveTarget(m_autoScroll ? m_pLog.get() : m_pNoScrollLog.get());
        if (!m_autoScroll && m_timerAutoScroll.IsRunning())
        {
            m_timerAutoScroll.Stop();
        }
    }

    if (m_autoScroll)
    {
        m_listBoxMessages->ShowLastItem();
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
        const wxString fnFullPath = fn.GetFullPath();

        if (!wxFileExists(fnFullPath))
        {
            wxLogDebug("Nonexistent temporary file: %s", fnFullPath);
            continue;
        }

        if (!wxRemoveFile(fn.GetFullPath()))
        {
            wxLogWarning(_("Unable to delete temporary file: %s"), fnFullPath);
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
                m_staticTextCommonDir->SetLabel(m_commonDir->GetFileName().GetFullPath());
                m_staticTextCommonDir->Show();
            }
            else
            {
                m_staticTextCommonDir->SetLabel(wxEmptyString);
                m_staticTextCommonDir->Show(false);
            }
            m_sizerInputFiles->Layout();
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
        const wxRelativeFileName rfn = get_variant_custom_val<wxVariantDataRelativeFileName>(v);
        afn.push_back(rfn.GetFileName());

        dataModel->GetValue(v, i, 2);
        if (v.GetType().CmpNoCase(wxS("wxSize")) != 0) continue;
        const wxSize imgSize = get_variant_custom_val<wxVariantDataSize>(v);

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
