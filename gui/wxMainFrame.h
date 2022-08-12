/*
 *      wxMainFrame.h
 */

#pragma once

#ifndef _LOG_LIST_BOX_H_
#include "LogListBox.h"
#endif

class wxMainFrame:
    public wxFrame, wxThreadHelper
{
    wxDECLARE_NO_COPY_CLASS(wxMainFrame);

    typedef wxVector<wxFileName> wxArrayFileName;

    protected:

    wxNotebook* m_notebook;
    wxDataViewListCtrl* m_listViewInputFiles;
    wxCheckBox* m_checkBoxVerbose;
    wxCheckBox* m_checkBoxSwitchToMessagesPane;
    wxTextCtrl* m_textCtrlDst;
    wxCheckBox* m_checkBoxOutputDecompress;
    wxCheckBox* m_checkBoxOutputCompressFonts;
    wxCheckBox* m_checkBoxOutputAscii;
    wxCheckBox* m_checkBoxOutputPretty;
    wxCheckBox* m_checkBoxOutputClean;
    wxCheckBox* m_checkBoxOutputSanitize;
    wxCheckBox* m_checkBoxOutputLinearize;
    wxCheckBox* m_checkBoxMetadataAuthor;
    wxTextCtrl* m_textCtrlMetadataAuthor;
    wxCheckBox* m_checkBoxMetadataTitle;
    wxTextCtrl* m_textCtrlMetadataTitle;
    wxCheckBox* m_checkBoxMetadataSubject;
    wxTextCtrl* m_textCtrlMetadataSubject;
    wxCheckBox* m_checkBoxMetadataCreator;
    wxTextCtrl* m_textCtrlMetadataCreator;
    wxCheckBox* m_checkBoxMetadataCreationDate;
    wxDatePickerCtrl* m_datePickerMetadataCreationDate;
    wxTimePickerCtrl* m_timePickerMetadataCreationDate;
    wxCheckBox* m_checkBoxMetadataModDate;
    wxDatePickerCtrl* m_datePickerMetadataModDate;
    wxTimePickerCtrl* m_timePickerMetadataModDate;
    wxCheckBox* m_checkBoxShowTimestamps;
    ListBox* m_listBoxMessages;

    protected:

    void OnClose(wxCloseEvent&);
    void OnIdle(wxIdleEvent&);
    void OnIdleWakeupTimer(wxTimerEvent&);
    void OnAutoScrollTimer(wxTimerEvent&);
    void OnProcessTerminated(wxProcessEvent&);
#ifdef __WXMSW__
    void OnTaskKillProcessTerminated(wxProcessEvent&);
#endif
    void OnUpdateMsgCnt(wxUpdateUIEvent&);
    void OnUpdateRunUiCtrl(wxUpdateUIEvent&);
    void OnCheckVerbose(wxCommandEvent&);
    void OnUpdateButtonRun(wxUpdateUIEvent&);
    void OnExecMuTool(wxCommandEvent&);
    void OnUpdateButtonAdd(wxUpdateUIEvent&);
    void OnButtonAdd(wxCommandEvent&);
    void OnUpdateButtonDelete(wxUpdateUIEvent&);
    void OnButtonDelete(wxCommandEvent&);
    void OnUpdateDst(wxUpdateUIEvent&);
    void OnChooseDst(wxCommandEvent&);
    void OnCheckAutoScroll(wxCommandEvent&);
    void OnCheckShowTimestamps(wxCommandEvent&);
    void OnUpdateMsgCtrls(wxUpdateUIEvent&);
    void OnCopyEvents(wxCommandEvent&);
    void OnItemUpdated(wxThreadEvent&);

    private:

    wxPanel* create_src_dst_pannel(wxNotebook*, const wxFont&, const wxFont&, const wxSizerFlags&, const wxSizerFlags&, const wxSizerFlags&, const wxSizerFlags&);
    wxPanel* create_metadata_pannel(wxNotebook*, const wxFont&, const wxSizerFlags&, const wxSizerFlags&, const wxSizerFlags&);
    wxPanel* create_messages_panel(wxNotebook*, const wxFont&, const wxSizerFlags&, const wxSizerFlags&, const wxSizerFlags&, const wxSizerFlags&);
    wxNotebook* create_notebook(const wxFont&, const wxFont&, const wxSizerFlags&, const wxSizerFlags&, const wxSizerFlags&, const wxSizerFlags&, const wxSizerFlags&);
    wxBoxSizer* create_bottom_ctrls(const wxFont&, const wxSizerFlags&, const wxSizerFlags&, const wxSizerFlags&, const wxSizerFlags&);

    private:

    void build_script(wxJson&, wxFileName&) const;
    void delete_temporary_files();
    void post_focus_list() const;
    virtual wxThread::ExitCode Entry() wxOVERRIDE;

    void ExecuteMuTool(const wxArrayString&, const wxFileName&, const wxArrayFileName&);
    void ExecuteCmd(const wxFileName&, const wxString&, const wxFileName&, const wxArrayFileName&);

    void ProcessOutErr(bool = false);
#ifdef __WXMSW__
    void ExecuteTaskKill();
#endif

    private:

    wxScopedPtr<wxLog> m_pLog;
    wxScopedPtr<wxLog> m_pNoScrollLog;
    wxLog* m_pPrevLog;

    wxArrayFileName m_temporaryFiles;
    wxScopedPtr<wxProcess> m_pProcess;
#ifdef __WXMSW__
    wxScopedPtr<wxProcess> m_pTaskKillProcess;
#endif

    wxTimer m_timerIdleWakeUp;
    wxTimer m_timerAutoScroll;

    wxString m_execButtonCaptionRun;
    wxString m_execButtonCaptionKill;
    wxString m_logTimestamp;
    bool     m_autoScroll;

    public:

    wxMainFrame(wxWindow* = nullptr, wxWindowID = wxID_ANY, const wxString& = _("Image to PDF converter"), const wxPoint& = wxDefaultPosition, const wxSize& = wxDefaultSize, long = wxDEFAULT_FRAME_STYLE | wxCLIP_SIBLINGS | wxTAB_TRAVERSAL);
    void OnDropFiles(const wxArrayString&);
};
