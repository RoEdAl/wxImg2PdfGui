/*
 *      wxMainFrame.h
 */

#pragma once

#ifndef _LOG_LIST_BOX_H_
#include "LogListBox.h"
#endif

#ifndef _VARIANT_EXT_H_
#include "VariantExt.h"
#endif

#ifndef _DATE_TIME_PICKER_H_
#include "DateTimePicker.h"
#endif

class wxMainFrame:
    public wxFrame, wxThreadHelper
{
    wxDECLARE_NO_COPY_CLASS(wxMainFrame);

    typedef std::vector<wxFileName> wxArrayFileName;

    protected:

    wxNotebook* m_notebook;
    wxDataViewListCtrl* m_listViewInputFiles;
    wxStaticText* m_staticTextCommonDir;
    wxButton* m_buttonCommonDir;
    wxSizer* m_sizerDst;
    wxTextCtrl* m_textCtrlDst;
    wxCheckBox* m_checkBoxOutputDecompress;
    wxCheckBox* m_checkBoxOutputCompressFonts;
    wxCheckBox* m_checkBoxOutputAscii;
    wxCheckBox* m_checkBoxOutputPretty;
    wxCheckBox* m_checkBoxOutputClean;
    wxCheckBox* m_checkBoxOutputSanitize;
    wxCheckBox* m_checkBoxOutputLinearize;
    wxStaticText* m_staticTextEstimatedSize;
    wxCheckBox* m_checkBoxMetadataAuthor;
    wxTextCtrl* m_textCtrlMetadataAuthor;
    wxCheckBox* m_checkBoxMetadataTitle;
    wxTextCtrl* m_textCtrlMetadataTitle;
    wxCheckBox* m_checkBoxMetadataSubject;
    wxTextCtrl* m_textCtrlMetadataSubject;
    wxCheckBox* m_checkBoxMetadataCreator;
    wxTextCtrl* m_textCtrlMetadataCreator;
    wxCheckBox* m_checkBoxMetadataCreationDate;
    DateTimePicker* m_dateTimePickerMetadataCreationDate;
    wxCheckBox* m_checkBoxMetadataModDate;
    DateTimePicker* m_dateTimePickerMetadataModDate;
    ListBox* m_listBoxMessages;

    protected:

    void OnClose(wxCloseEvent&);
    void OnIdle(wxIdleEvent&);
    void OnIdleWakeupTimer(wxTimerEvent&);
    void OnProcessTerminated(wxProcessEvent&);
#ifdef __WXMSW__
    void OnTaskKillProcessTerminated(wxProcessEvent&);
#endif
    void OnUpdateRunUiCtrl(wxUpdateUIEvent&);
    void OnCheckVerbose(wxCommandEvent&);
    void OnUpdateButtonRun(wxUpdateUIEvent&);
    void OnExecMuTool(wxCommandEvent&);
    void OnUpdateButtonAdd(wxUpdateUIEvent&);
    void OnButtonAdd(wxCommandEvent&);
    void OnUpdateButtonDelete(wxUpdateUIEvent&);
    void OnButtonDelete(wxCommandEvent&);
    void OnUpdateButtonSelectAll(wxUpdateUIEvent&);
    void OnButtonSelectAll(wxCommandEvent&);
    void OnUpdateButtonResolutionScale(wxUpdateUIEvent&);
    void OnButtonResolutionScale(wxCommandEvent&);
    void OnUpdateButtonDocOpen(wxUpdateUIEvent&);
    void OnButtonDocOpen(wxCommandEvent&);
    void OnUpdateButtonCopyToDst(wxUpdateUIEvent&);
    void OnButtonCopyToDst(wxCommandEvent&);
    void OnButtonClearResolutionScale(wxCommandEvent&);
    void OnChooseDst(wxCommandEvent&);
    void OnCheckShowTimestamps(wxCommandEvent&);
    void OnUpdateMsgCtrls(wxUpdateUIEvent&);
    void OnCopyEvents(wxCommandEvent&);
    void OnItemUpdated(wxThreadEvent&);
    void OnDataViewItemActiveted(wxDataViewEvent&);
    void OnUpdateEstimatedOutputSize(wxUpdateUIEvent&);

    private:

    wxSizer* create_vertical_button_panel(const wxStaticBoxSizer* const) const;
    wxSizer* create_pdf_options_panel(const wxStaticBoxSizer* const);
    wxPanel* create_src_dst_pannel(wxNotebook*);
    wxPanel* create_metadata_pannel(wxNotebook*);
    wxPanel* create_messages_panel(wxNotebook*);
    wxNotebook* create_notebook();
    wxBoxSizer* create_bottom_ctrls();

    private:

    void build_script(wxJson&) const;
    void delete_temporary_files();
    void post_focus_list() const;
    void update_total_size_text() const;
    virtual wxThread::ExitCode Entry() wxOVERRIDE;

    void ExecuteMuTool(const wxArrayString&, const wxFileName&, const wxArrayFileName&);
    void ExecuteCmd(const wxFileName&, const wxString&, const wxFileName&, const wxArrayFileName&);

    void ProcessOutErr(bool = false);
#ifdef __WXMSW__
    void ExecuteTaskKill();
#endif

    private:

    wxScopedPtr<wxLog> m_pLog;
    wxLog* m_pPrevLog;

    wxArrayFileName m_temporaryFiles;
    wxScopedPtr<wxProcess> m_pProcess;
#ifdef __WXMSW__
    wxScopedPtr<wxProcess> m_pTaskKillProcess;
#endif

    wxTimer m_timerIdleWakeUp;

    wxString m_execButtonCaptionRun;
    wxString m_execButtonCaptionKill;
    wxString m_logTimestamp;
    bool     m_autoScroll;

    wxObjectDataPtr<wxFileNameRefData> m_commonDir;
    wxULongLong m_totalSize;

    public:

    wxMainFrame(wxWindow* = nullptr, wxWindowID = wxID_ANY, const wxString& = _("Image to PDF converter"), const wxPoint& = wxDefaultPosition, const wxSize& = wxDefaultSize, long = wxDEFAULT_FRAME_STYLE | wxCLIP_SIBLINGS | wxTAB_TRAVERSAL);
    void OnDropFiles(const wxArrayString&);
};
