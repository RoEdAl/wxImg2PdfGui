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
    constexpr int AUTO_SCROLL_UPDATE_INTERVAL = 2000;
    constexpr int TIMER_IDLE_WAKE_UP_INTERVAL = 250;
    constexpr wxChar DEFAULT_OUTPUT_FILE_NAME[] = wxS("album.pdf");
    constexpr wxWindowVariant LAUNCH_BUTTON_VARIANT = wxWINDOW_VARIANT_LARGE;

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

    wxStaticBoxSizer* create_static_box_sizer(wxWindow* const parent, const wxString& label, const wxOrientation orientation)
    {
        return new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, label), orientation);
    }

    wxCheckBox* create_checkbox(wxWindow* const parent, const wxString& label, const bool val = false)
    {
        wxCheckBox* const res = new wxCheckBox(parent, wxID_ANY, label);
        res->SetValue(val);
        return res;
    }

    wxCheckBox* create_checkbox(const wxStaticBoxSizer* const parentSizer, const wxString& label, const bool val = false)
    {
        return create_checkbox(parentSizer->GetStaticBox(), label, val);
    }

    wxCheckBox* create_mini_checkbox(const wxStaticBoxSizer* const sizer,
                                 const wxString& label,
                                 const wxString& toolTip,
                                 const bool value = false)
    {
        wxCheckBox* const checkBox = create_checkbox(sizer, label, value);
        checkBox->SetWindowVariant(wxWINDOW_VARIANT_MINI);
        checkBox->SetToolTip(toolTip);
        return checkBox;
    }

    wxCheckBox* create_3state_checkbox(wxWindow* const parent, const wxString& label, const wxCheckBoxState state = wxCHK_UNDETERMINED)
    {
        wxCheckBox* const res = new wxCheckBox(parent, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxCHK_3STATE | wxCHK_ALLOW_3RD_STATE_FOR_USER);
        res->Set3StateValue(state);
        return res;
    }

    wxCheckBox* create_3state_checkbox(const wxStaticBoxSizer* const parentSizer, const wxString& label, const wxCheckBoxState state = wxCHK_UNDETERMINED)
    {
        return create_3state_checkbox(parentSizer->GetStaticBox(), label, state);
    }

    bool is_checked(const wxCheckBox* const checkBox)
    {
        if (checkBox->Is3State()) return (checkBox->Get3StateValue() == wxCHK_CHECKED);
        else return checkBox->GetValue();
    }

    wxButton* create_button(wxWindow* const parent, const wxString& label)
    {
        return new wxButton(parent, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    }

    wxButton* create_button(const wxStaticBoxSizer* const parentSizer, const wxString& label)
    {
        return create_button(parentSizer->GetStaticBox(), label);
    }

    wxBitmapButton* create_bitmap_button(wxWindow* const parent, const wxBitmapBundle& bitmapBundle, const wxWindowVariant windowVariant = wxWINDOW_VARIANT_SMALL)
    {
        wxBitmapButton* const res = new wxBitmapButton(parent, wxID_ANY, bitmapBundle);
        res->SetWindowVariant(windowVariant);
        return res;
    }

    template<typename P>
    void create_bitmap_button(wxWindow* const parent, const wxBitmapBundle& bitmapBundle, P initializer, const wxWindowVariant windowVariant = wxWINDOW_VARIANT_SMALL)
    {
        wxBitmapButton* const button = create_bitmap_button(parent, bitmapBundle, windowVariant);
        initializer(button);
    }

    wxBitmapButton* create_bitmap_button(const wxStaticBoxSizer* const parentSizer, const wxBitmapBundle& bitmapBundle, const wxWindowVariant windowVariant = wxWINDOW_VARIANT_SMALL)
    {
        return create_bitmap_button(parentSizer->GetStaticBox(), bitmapBundle, windowVariant);
    }

    template<typename P>
    void create_bitmap_button(const wxStaticBoxSizer* const parentSizer, const wxBitmapBundle& bitmapBundle, P initializer, const wxWindowVariant windowVariant = wxWINDOW_VARIANT_SMALL)
    {
        wxBitmapButton* const button = create_bitmap_button(parentSizer, bitmapBundle, windowVariant);
        initializer(button);
    }

    wxBitmapButton* create_bitmap_button(wxWindow* const parent, const wxString& resName, const wxWindowVariant windowVariant = wxWINDOW_VARIANT_SMALL)
    {
        wxBitmapBundle bitmapBundle;
        wxCHECK_MSG(wxGetApp().LoadMaterialDesignIcon(resName, windowVariant, bitmapBundle), nullptr, wxString::Format("Fail to load bitmap: id=%s", resName));
        return create_bitmap_button(parent, bitmapBundle, windowVariant);
    }

    template<typename P>
    void create_bitmap_button(wxWindow* const parent, const wxString& resName, P initializer, const wxWindowVariant widnowVariant = wxWINDOW_VARIANT_SMALL)
    {
        wxBitmapButton* const button = create_bitmap_button(parent, resName, widnowVariant);
        if (button != nullptr)
        {
            initializer(button);
        }
    }

    wxBitmapButton* create_bitmap_button(const wxStaticBoxSizer* const parentSizer, const wxString& resName, const wxWindowVariant windowVariant = wxWINDOW_VARIANT_SMALL)
    {
        return create_bitmap_button(parentSizer->GetStaticBox(), resName, windowVariant);
    }

    template<typename P>
    void create_bitmap_button(const wxStaticBoxSizer* const parentSizer, const wxString& resName, P initializer, const wxWindowVariant widnowVariant = wxWINDOW_VARIANT_SMALL)
    {
        create_bitmap_button(parentSizer->GetStaticBox(), resName, initializer, widnowVariant);
    }

    wxStaticBitmap* create_static_bitmap(wxWindow* const parent, const wxBitmapBundle& bitmapBundle, const wxWindowVariant windowVariant = wxWINDOW_VARIANT_SMALL)
    {
        wxStaticBitmap* const res = new wxStaticBitmap(parent, wxID_ANY, bitmapBundle);
        res->SetWindowVariant(windowVariant);
        return res;
    }

    wxStaticBitmap* create_static_bitmap(const wxStaticBoxSizer* const parentSizer, const wxBitmapBundle& bitmapBundle, const wxWindowVariant windowVariant = wxWINDOW_VARIANT_SMALL)
    {
        return create_static_bitmap(parentSizer->GetStaticBox(), bitmapBundle, windowVariant);
    }

    wxStaticBitmap* create_static_bitmap(wxWindow* const parent, const wxString& resName, const wxWindowVariant windowVariant = wxWINDOW_VARIANT_SMALL)
    {
        wxBitmapBundle bitmapBundle;
        wxCHECK_MSG(wxGetApp().LoadMaterialDesignIcon(resName, windowVariant, bitmapBundle), nullptr, wxString::Format("Fail to load bitmap: id=%s", resName));
        return create_static_bitmap(parent, bitmapBundle, windowVariant);
    }

    wxStaticBitmap* create_static_bitmap(const wxStaticBoxSizer* const parentSizer, const wxString& resName, const wxWindowVariant windowVariant = wxWINDOW_VARIANT_SMALL)
    {
        return create_static_bitmap(parentSizer->GetStaticBox(), resName, windowVariant);
    }

    wxTextCtrl* create_text_ctrl(wxWindow* const parent, const wxString& label = wxEmptyString, const unsigned long maxLength = 0)
    {
        wxTextCtrl* const res = new wxTextCtrl(parent, wxID_ANY, label);
        if (maxLength > 0) res->SetMaxLength(maxLength);
        return res;
    }

    wxTextCtrl* create_text_ctrl(const wxStaticBoxSizer* const parentSizer, const wxString& label = wxEmptyString, unsigned long maxLength = 0)
    {
        return create_text_ctrl(parentSizer->GetStaticBox(), label, maxLength);
    }

    wxStaticText* create_static_text(wxWindow* const parent, const wxString& label = wxEmptyString)
    {
        return new wxStaticText(parent, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT | wxST_ELLIPSIZE_MIDDLE | wxST_NO_AUTORESIZE);
    }

    wxStaticText* create_static_text(const wxStaticBoxSizer* const parentSizer, const wxString& label = wxEmptyString)
    {
        return create_static_text(parentSizer->GetStaticBox(), label);
    }

    wxStaticText* create_centered_static_text(wxWindow* const parent, const wxString& label = wxEmptyString)
    {
        return new wxStaticText(parent, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER | wxST_ELLIPSIZE_START);
    }

    wxStaticText* create_centered_static_text(const wxStaticBoxSizer* const parentSizer, const wxString& label = wxEmptyString)
    {
        return create_centered_static_text(parentSizer->GetStaticBox(), label);
    }

    wxStaticLine* create_horizontal_static_line(wxWindow* const parent)
    {
        return new wxStaticLine(parent, wxID_ANY, wxDefaultPosition, wxSize(0, parent->FromDIP(1)), wxLI_HORIZONTAL);
    }

    template<typename P>
    void create_horizontal_static_line(wxWindow* const parent, P initializer)
    {
        wxStaticLine* const staticLine = create_horizontal_static_line(parent);
        if (staticLine != nullptr)
        {
            initializer(staticLine);
        }
    }

    wxStaticLine* create_horizontal_static_line(const wxStaticBoxSizer* const parentSizer)
    {
        return create_horizontal_static_line(parentSizer->GetStaticBox());
    }

    template<typename P>
    void create_horizontal_static_line(const wxStaticBoxSizer* const parentSizer, P initializer)
    {
        wxStaticLine* const staticLine = create_horizontal_static_line(parentSizer->GetStaticBox());
        if (staticLine != nullptr)
        {
            initializer(staticLine);
        }
    }

    wxSizerFlags get_horizontal_static_line_sizer_flags(const wxWindow* const wnd)
    {
        return wxSizerFlags().Expand().Border(wxTOP | wxBOTTOM, wnd->FromDIP(2));
    }

    wxSizerFlags get_horizontal_static_line_sizer_flags(const wxStaticBoxSizer* const sizer)
    {
        return get_horizontal_static_line_sizer_flags(sizer->GetStaticBox());
    }

    wxHyperlinkCtrl* create_hyperlink(wxWindow* const parent, const wxString& label = wxEmptyString, const wxWindowVariant windowVariant = wxWINDOW_VARIANT_SMALL)
    {
        wxHyperlinkCtrl* const res = new wxHyperlinkCtrl(parent, wxID_ANY, label, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxHL_ALIGN_CENTRE);
        res->SetWindowVariant(windowVariant);
        return res;
    }

    wxHyperlinkCtrl* create_hyperlink(const wxStaticBoxSizer* const parentSizer, const wxString& label = wxEmptyString, const wxWindowVariant windowVariant = wxWINDOW_VARIANT_SMALL)
    {
        return create_hyperlink(parentSizer->GetStaticBox(), label, windowVariant);
    }

    wxSize calc_text_size(const wxWindow* const wnd, int charWidth)
    {
        return wnd->GetTextExtent(wxString(wxUniChar(0x2013), charWidth));
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

namespace
{
    template<typename P>
    void create_bmp_data_view_column(const wxString& variantType, const wxString& title, unsigned int modelColumn, P initializer)
    {
        wxDataViewBitmapRenderer* const renderer = new wxDataViewBitmapRenderer(variantType);
        wxDataViewColumn* const column = new wxDataViewColumn(title, renderer, modelColumn, wxCOL_WIDTH_AUTOSIZE);
        column->SetSortable(false);
        column->SetReorderable(false);
        column->SetResizeable(false);
        initializer(column);
    }

    template<typename P>
    void create_txt_data_view_column(const wxString& variantType, const wxString& title, unsigned int modelColumn, P initializer, const wxAlignment aligment = wxALIGN_CENTER)
    {
        wxDataViewTextRenderer* const renderer = new wxDataViewTextRenderer(variantType);
        wxDataViewColumn* const column = new wxDataViewColumn(title, renderer, modelColumn, wxCOL_WIDTH_AUTOSIZE, aligment);
        column->SetSortable(false);
        column->SetReorderable(false);
        column->SetResizeable(false);
        initializer(column);
    }

    wxDataViewListCtrl* create_data_view_list(const wxStaticBoxSizer* const sizer)
    {
        wxDataViewListCtrl* const dataViewList = new wxDataViewListCtrl(sizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_VERT_RULES | wxDV_MULTIPLE | wxDV_ROW_LINES | wxBORDER_THEME);
        auto& columnAppender = [dataViewList](auto* const column) {
            dataViewList->AppendColumn(column);
        };

        create_bmp_data_view_column(wxS("wxBitmapBundle"), wxS("#"), 0, columnAppender);
        create_txt_data_view_column(wxS("wxRelativeFileName"), _("File"), 1, columnAppender, wxALIGN_LEFT);

        create_txt_data_view_column(wxS("wxSize"), _("Dimensions"), 2, [dataViewList, &columnAppender](auto* const column) {
            const wxSize minSize = calc_text_size(dataViewList, 15);
            column->SetMinWidth(minSize.GetWidth());
            columnAppender(column);
        });

        create_txt_data_view_column(wxS("wxResolutionOrScale"), _("Resolution/Scale"), 3, columnAppender);
        create_txt_data_view_column(wxS("string"), _("Size"), 4, columnAppender);
        create_txt_data_view_column(wxS("null"), wxEmptyString, 5, columnAppender);
        return dataViewList;
    }
}

wxSizer* wxMainFrame::create_vertical_button_panel(const wxStaticBoxSizer* const sizer) const
{
    wxBoxSizer* const vinnerSizer = new wxBoxSizer(wxVERTICAL);
    wxMainFrame* const frame = wxConstCast(this, wxMainFrame);
    const wxSizerFlags buttonFlags = wxSizerFlags().CentreHorizontal();

    create_bitmap_button(sizer, "content-add", [frame, vinnerSizer, &buttonFlags](auto* const button) {
        button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonAdd, frame);
        button->Bind(wxEVT_BUTTON, &wxMainFrame::OnButtonAdd, frame);
        vinnerSizer->Add(button, buttonFlags);
    });

    create_bitmap_button(sizer, "content-remove", [frame, vinnerSizer, &buttonFlags](auto* const button) {
        button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonDelete, frame);
        button->Bind(wxEVT_BUTTON, &wxMainFrame::OnButtonDelete, frame);
        vinnerSizer->Add(button, buttonFlags);
    });

    create_bitmap_button(sizer, "content-select_all", [frame, vinnerSizer, &buttonFlags](auto* const button) {
        button->SetToolTip(_("Select all"));
        button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonSelectAll, frame);
        button->Bind(wxEVT_BUTTON, &wxMainFrame::OnButtonSelectAll, frame);
        vinnerSizer->Add(button, buttonFlags);
    });

    create_horizontal_static_line(sizer, [vinnerSizer, sizer](auto* const staticLine) {
        vinnerSizer->Add(staticLine, get_horizontal_static_line_sizer_flags(sizer));
    });

    create_bitmap_button(sizer, "action-aspect_ratio", [frame, vinnerSizer, &buttonFlags](auto* const button) {
        button->SetToolTip(_("Change resolution/Scale"));
        button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonResolutionScale, frame);
        button->Bind(wxEVT_BUTTON, &wxMainFrame::OnButtonResolutionScale, frame);
        vinnerSizer->Add(button, buttonFlags);
    });

    create_bitmap_button(sizer, "content-clear", [frame, vinnerSizer, &buttonFlags](auto* const button) {
        button->SetToolTip(_("Use original image resolution/Do not scale image"));
        button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonResolutionScale, frame);
        button->Bind(wxEVT_BUTTON, &wxMainFrame::OnButtonClearResolutionScale, frame);
        vinnerSizer->Add(button, buttonFlags);
    });

    if (wxGetApp().SumatraPdfFound())
    {
        vinnerSizer->AddStretchSpacer();
        create_bitmap_button(sizer, "action-preview", [frame, vinnerSizer, &buttonFlags](auto* const button) {
            button->SetToolTip(_("Launch document viewer"));
            button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonDocOpen, frame);
            button->Bind(wxEVT_BUTTON, &wxMainFrame::OnButtonDocOpen, frame);
            vinnerSizer->Add(button, wxSizerFlags(buttonFlags).DoubleBorder(wxTOP));
        });
    }

    return vinnerSizer;
}

wxSizer* wxMainFrame::create_pdf_options_panel(const wxStaticBoxSizer* const sizer)
{
    wxFlexGridSizer* const innerSizer = new wxFlexGridSizer(4);
    innerSizer->Add(m_checkBoxOutputCompressFonts = create_mini_checkbox(sizer, _T("Compress fonts"), _T("Compress fonts"), true));
    innerSizer->Add(m_checkBoxOutputPretty = create_mini_checkbox(sizer, _T("Pretty"), _wxS("Pretty\u2011print objects with indentation"), true));
    innerSizer->Add(m_checkBoxOutputClean = create_mini_checkbox(sizer, _T("Clean"), _wxS("Pretty\u2011print graphics commands in content streams"), true));
    innerSizer->Add(m_checkBoxOutputSanitize = create_mini_checkbox(sizer, _T("Sanitize"), _T("Sanitize graphics commands in content streams"), true));
    innerSizer->Add(m_checkBoxOutputLinearize = create_mini_checkbox(sizer, _T("Linearize"), _T("Optimize for web browsers")));
    innerSizer->Add(m_checkBoxOutputDecompress = create_mini_checkbox(sizer, _T("Decompress"), _T("Decompress all streams")));
    innerSizer->Add(m_checkBoxOutputAscii = create_mini_checkbox(sizer, _T("ASCII"), _T("ASCII hex encode binary streams")));
    return innerSizer;
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
                    wxDataViewListCtrl* const dataViewList = create_data_view_list(sizer);
                    dataViewList->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, &wxMainFrame::OnDataViewItemActiveted, this);
                    innerSizer->Add(dataViewList, wxSizerFlags().Proportion(1).Expand());
                    m_listViewInputFiles = dataViewList;
                }

                {
                    wxSizer* const vinnerSizer = create_vertical_button_panel(sizer);
                    wxSizerFlags panelFlags = wxSizerFlags().Border(wxLEFT);

                    if (wxGetApp().SumatraPdfFound())
                    {
                        panelFlags.Expand();
                        
                    }
                    else
                    {
                        panelFlags.Top();
                    }
                    innerSizer->Add(vinnerSizer, panelFlags);
                }

                {
                    wxBoxSizer* const hsizer = new wxBoxSizer(wxHORIZONTAL);
                    const wxSizerFlags ctrlFlags = wxSizerFlags().CenterVertical();

                    create_bitmap_button(sizer, "file-folder", [this, hsizer, &ctrlFlags, sizer](auto* const button) {
                        button->SetToolTip(_("Open common directory"));
                        button->Enable(false);
                        button->Bind(wxEVT_BUTTON, &wxMainFrame::OnButtonOpenCommonDir, this);
                        hsizer->Add(button, wxSizerFlags(ctrlFlags).Border(wxRIGHT, sizer->GetStaticBox()->FromDIP(4)));
                    });

                    {
                        wxStaticText* const staticText = create_static_text(sizer);
                        staticText->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
                        staticText->SetToolTip(_("Common directory"));
                        staticText->Enable(false);
                        hsizer->Add(staticText, wxSizerFlags(ctrlFlags).Proportion(1));
                        m_staticTextCommonDir = staticText;
                    }

                    innerSizer->Add(hsizer, wxSizerFlags(ctrlFlags).Expand().Border(wxTOP));
                    m_sizerCommonDir = hsizer;
                }

                create_bitmap_button(sizer, "navigation-arrow_downward", [this, innerSizer](auto* const button) {
                    button->SetToolTip(_("Copy common directory to destination"));
                    button->Enable(false);
                    button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonCopyToDst, this);
                    button->Bind(wxEVT_BUTTON, &wxMainFrame::OnButtonCopyToDst, this);
                    innerSizer->Add(button, wxSizerFlags().Center().Border(wxLEFT));
                    m_buttonCommonDir = button;
                });

                sizer->Add(innerSizer, wxSizerFlags().Expand().Proportion(1));
            }

            panelSizer->Add(sizer, wxSizerFlags().Expand().Proportion(1));
        }
    }

    {
        wxStaticBoxSizer* const sizer = create_static_box_sizer(panel, _("Destination"), wxVERTICAL);

        {
            wxBoxSizer* const innerSizer = new wxBoxSizer(wxHORIZONTAL);

            {
                wxTextCtrl* const textCtrl = create_text_ctrl(sizer, wxEmptyString, 1024);
                const wxFileName fn = get_default_output_fn();
                textCtrl->SetValue(fn.GetFullPath());
                innerSizer->Add(textCtrl, wxSizerFlags().CentreVertical().Proportion(1));
                m_textCtrlDst = textCtrl;
            }

            create_bitmap_button(sizer, "navigation-more_horiz", [this, innerSizer](auto* const button) {
                button->Bind(wxEVT_BUTTON, &wxMainFrame::OnChooseDst, this);
                innerSizer->Add(button, wxSizerFlags().CenterVertical().Border(wxLEFT));
            }, wxWINDOW_VARIANT_NORMAL);

            sizer->Add(innerSizer, wxSizerFlags().Expand());
        }

        {
            wxBoxSizer* const innerSizer = new wxBoxSizer(wxHORIZONTAL);

            {
                wxSizer* const panelSizer = create_pdf_options_panel(sizer);
                innerSizer->Add(panelSizer, wxSizerFlags().CenterVertical());
            }

            innerSizer->AddStretchSpacer();

            {
                wxHyperlinkCtrl* const hyperlinkCtrl = create_hyperlink(sizer);
                hyperlinkCtrl->SetToolTip(_("Estimated output file size\nClick to open"));
                hyperlinkCtrl->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateEstimatedOutputSize, this);
                hyperlinkCtrl->Bind(wxEVT_HYPERLINK, &wxMainFrame::OnOpenDestination, this);
                innerSizer->Add(hyperlinkCtrl, wxSizerFlags().CenterVertical().DoubleBorder(wxLEFT));
                m_hyperlinkCtrlEstimatedSize = hyperlinkCtrl;
            }

            sizer->Add(innerSizer, wxSizerFlags().Expand().Border(wxTOP));
            m_sizerDst = innerSizer;
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

        {
            wxCheckBox* const checkBox = create_checkbox(panel, _("Author"));
            sizer->Add(checkBox, labelSizerFlags);
            m_checkBoxMetadataAuthor = checkBox;
        }

        {
            wxTextCtrl* const textCtrl = create_text_ctrl(panel);
            textCtrl->Bind(wxEVT_UPDATE_UI, CheckBoxUiUpdater(m_checkBoxMetadataAuthor));
            sizer->Add(textCtrl, textCtrlSizerFlags);
            m_textCtrlMetadataAuthor = textCtrl;
        }

        {
            wxCheckBox* const checkBox = create_checkbox(panel, _("Title"));
            sizer->Add(checkBox, labelSizerFlags);
            m_checkBoxMetadataTitle = checkBox;
        }

        {
            wxTextCtrl* const textCtrl = create_text_ctrl(panel);
            textCtrl->Bind(wxEVT_UPDATE_UI, CheckBoxUiUpdater(m_checkBoxMetadataTitle));
            sizer->Add(textCtrl, textCtrlSizerFlags);
            m_textCtrlMetadataTitle = textCtrl;
        }

        {
            wxCheckBox* const checkBox = create_checkbox(panel, _("Subject"));
            sizer->Add(checkBox, labelSizerFlags);
            m_checkBoxMetadataSubject = checkBox;
        }

        {
            wxTextCtrl* const textCtrl = create_text_ctrl(panel);
            textCtrl->Bind(wxEVT_UPDATE_UI, CheckBoxUiUpdater(m_checkBoxMetadataSubject));
            sizer->Add(textCtrl, textCtrlSizerFlags);
            m_textCtrlMetadataSubject = textCtrl;
        }

        {
            wxCheckBox* const checkBox = create_checkbox(panel, _("Creator"));
            sizer->Add(checkBox, labelSizerFlags);
            m_checkBoxMetadataCreator = checkBox;
        }

        {
            wxTextCtrl* const textCtrl = create_text_ctrl(panel, wxGetApp().GetScriptPath().GetFullName());
            textCtrl->Bind(wxEVT_UPDATE_UI, CheckBoxUiUpdater(m_checkBoxMetadataCreator));
            sizer->Add(textCtrl, textCtrlSizerFlags);
            m_textCtrlMetadataCreator = textCtrl;
        }

        const wxDateTime dtNow = wxDateTime::Now();

        {
            wxCheckBox* const checkBox = create_checkbox(panel, _("Creation date"));
            sizer->Add(checkBox, labelSizerFlags);
            m_checkBoxMetadataCreationDate = checkBox;
        }

        {
            DateTimePicker* const dateTimePicker = new DateTimePicker(panel, wxID_ANY, dtNow);
            dateTimePicker->Bind(wxEVT_UPDATE_UI, CheckBoxUiUpdater(m_checkBoxMetadataCreationDate));
            sizer->Add(dateTimePicker, textCtrlSizerFlags);
            m_dateTimePickerMetadataCreationDate = dateTimePicker;
        }

        {
            wxCheckBox* const checkBox = create_checkbox(panel, _("Modification date"));
            sizer->Add(checkBox, labelSizerFlags);
            m_checkBoxMetadataModDate = checkBox;
        }

        {
            DateTimePicker* const dateTimePicker = new DateTimePicker(panel, wxID_ANY, dtNow);
            dateTimePicker->Bind(wxEVT_UPDATE_UI, CheckBoxUiUpdater(m_checkBoxMetadataModDate));
            sizer->Add(dateTimePicker, textCtrlSizerFlags);
            m_dateTimePickerMetadataModDate = dateTimePicker;
        }
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
        ListBox* const listBox = new ListBox(panel);
        panelSizer->Add(listBox, wxSizerFlags().Border(wxLEFT|wxRIGHT|wxTOP).Expand().Proportion(1));
        m_listBoxMessages = listBox;
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

        create_bitmap_button(panel, "content-content_copy", [this, sizer](auto* const button) {
            button->SetToolTip(_("Copy all messages to clipboard"));
            button->Bind(wxEVT_BUTTON, &wxMainFrame::OnCopyEvents, this);
            button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateMsgCtrls, this);
            sizer->Add(button, wxSizerFlags().CenterVertical());
        });

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

    create_bitmap_button(this, m_bbLaunch, [this, sizer](auto* const button) {
        button->SetToolTip(_("Execute (or kill) mutool utility"));
        button->Bind(wxEVT_UPDATE_UI, &wxMainFrame::OnUpdateButtonRun, this);
        button->Bind(wxEVT_BUTTON, &wxMainFrame::OnExecMuTool, this);
        m_buttonRun = button;
        sizer->Add(button, wxSizerFlags().CentreVertical());
    }, LAUNCH_BUTTON_VARIANT);

    return sizer;
}

namespace
{
    bool load_bitmaps(wxBitmapBundle& bbLaunch, wxBitmapBundle& bbKill)
    {
        wxCHECK_MSG(wxGetApp().LoadMaterialDesignIcon("action-launch", LAUNCH_BUTTON_VARIANT, bbLaunch), false, "Fail to laod action-launch bitmap");
        wxCHECK_MSG(wxGetApp().LoadMaterialDesignIcon("image-flash_on", LAUNCH_BUTTON_VARIANT, bbKill), false, "Fail to load image-flash_on bitmap");
        return true;
    }
}

wxMainFrame::wxMainFrame(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxFrame(parent, id, title, pos, size, style),
    m_pPrevLog(nullptr),
    m_logTimestamp(wxLog::GetTimestamp()),
    m_autoScroll(true),
    m_commonDir(wxFileNameRefData::Get())
{
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
    SetIcons(wxGetApp().GetAppIcon());
    wxCHECK_RET(load_bitmaps(m_bbLaunch, m_bbKill), "MainFrame: fail to load launch bitmap");

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

    wxGetApp().ShowInfo();
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

    if (event.CanVeto() && IsThreadAlive())
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

    if (IsThreadAlive())
    {
        wxThread* const thread = GetThread();
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

    m_buttonRun->SetBitmapLabel(m_bbLaunch);

    delete_temporary_files();

    if (event.GetExitCode() != 0)
    {
        m_notebook->ChangeSelection(m_notebook->GetPageCount() - 1);
    }
    else
    {
        const wxFileName fn = wxFileName::FileName(m_textCtrlDst->GetValue());
        if (fn.IsFileReadable())
        {
            m_totalSize = fn.GetSize();
            update_total_size_text();
        }
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
        return;
    }

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
    m_buttonRun->SetBitmapLabel(m_bbKill);
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
    if (IsThreadAlive())
    {
        wxLogWarning(_("Cannot drop files - background thread is still running"));
        return;
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
    event.Enable(!IsThreadAlive());
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
        event.Enable(!IsThreadAlive());
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
        event.Enable(!IsThreadAlive());
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

bool wxMainFrame::IsThreadAlive() const
{
    const wxThread* thread = GetThread();
    if (thread != nullptr && thread->IsAlive())
    {
        return true;
    }
    return false;
}

void wxMainFrame::OnUpdateButtonResolutionScale(wxUpdateUIEvent& event)
{
    if ((m_listViewInputFiles->GetSelectedItemsCount() == 0) || IsThreadAlive())
    {
        event.Enable(false);
        return;
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
            dataModel->SetValue(wxVariantDataSize::Get(sizeUndetermined), i, 2); // dimensions
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
            }
            dataModel->SetValue(wxGetApp().GetEmptyString(), i, 4);
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

namespace
{
    void enable_sizer_windows(wxSizer* const sizer, bool enable)
    {
        const size_t cnt = sizer->GetItemCount();
        for (size_t i = 0; i < cnt; ++i)
        {
            const wxSizerItem* const si = sizer->GetItem(i);
            if (!si->IsWindow()) continue;
            wxWindow* const wnd = si->GetWindow();
            wnd->Enable(enable);
        }
    }
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

        case 2:
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
                enable_sizer_windows(m_sizerCommonDir, true);
                m_buttonCommonDir->Enable(true);
            }
            else
            {
                m_staticTextCommonDir->SetLabel(wxEmptyString);
                enable_sizer_windows(m_sizerCommonDir, false);
                m_buttonCommonDir->Enable(false);
            }

            m_totalSize = evPayload[1].GetULongLong();
            update_total_size_text();
            break;
        }

        case 4:
        {
            const wxWindowUpdateLocker locker(m_listViewInputFiles);
            const wxDataViewItem item(evPayload[0].GetVoidPtr());

            wxDataViewModel* const dataModel = m_listViewInputFiles->GetModel();
            dataModel->SetValue(evPayload[1], item, 2);
            const wxSize& sz = get_variant_custom_val<wxVariantDataSize>(evPayload[2]);
            dataModel->SetValue(wxVariantDataResolutionOrScale::GetResolution(sz), item, 3);
            dataModel->SetValue(evPayload[3], item, 4);
            break;
        }
    }
}

void wxMainFrame::update_total_size_text() const
{
    m_hyperlinkCtrlEstimatedSize->SetLabel(wxFileName::GetHumanReadableSize(m_totalSize, wxEmptyString));
    m_sizerDst->Layout();
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

    wxFileName find_common_path(const std::vector<wxFileName>& paths)
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

    wxULongLong sum_file_sizes(const std::vector<wxFileName>& paths)
    {
        wxULongLong res;
        for (const auto& i : paths)
        {
            if (!i.IsFileReadable()) continue;
            res += i.GetSize();
        }
        return res;
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
    std::vector<wxFileName> afn;
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

        const wxString hrFileSize = rfn.GetFileName().GetHumanReadableSize(wxEmptyString);
        vals.push_back(wxVariant(hrFileSize));
        wxQueueEvent(GetEventHandler(), create_thread_event(vals));
    }

    const wxFileName commonDir = find_common_path(afn);
    const wxULongLong totalSize = sum_file_sizes(afn);
    wxVector<wxVariant> vals;
    vals.push_back(wxVariantDataFileName::Get(commonDir));
    vals.push_back(wxVariant(totalSize));
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
        event.Enable(!IsThreadAlive());
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

void wxMainFrame::OnButtonOpenCommonDir(wxCommandEvent& WXUNUSED(event))
{
    const wxFileName& commonDir = m_commonDir->GetFileName();
    if (!commonDir.IsOk()) return;
    wxLaunchDefaultApplication(commonDir.GetFullPath());
}

void wxMainFrame::OnUpdateButtonCopyToDst(wxUpdateUIEvent& event)
{
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

void wxMainFrame::OnUpdateEstimatedOutputSize(wxUpdateUIEvent& event)
{
    if (m_pProcess || IsThreadAlive())
    {
        event.Enable(false);
        return;
    }

    if (m_totalSize == wxULL(0) || m_checkBoxOutputDecompress->GetValue() || m_checkBoxOutputAscii->GetValue())
    {
        event.Enable(false);
    }
    else
    {
        event.Enable(wxGetApp().SumatraPdfFound());
    }
}

void wxMainFrame::OnOpenDestination(wxHyperlinkEvent& WXUNUSED(event))
{
    if (m_pProcess || IsThreadAlive() || !wxGetApp().SumatraPdfFound())
    {
        return;
    }

    const wxString fns = m_textCtrlDst->GetValue();
    if (fns.IsEmpty()) return;
    const wxFileName fn = wxFileName::FileName(fns);
    if (!fn.IsFileReadable()) return;
    wxGetApp().RunDocViewer(fn);
}
