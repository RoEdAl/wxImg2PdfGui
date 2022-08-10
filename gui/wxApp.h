/*
 * wxApp.h
 */

#ifndef _WX_MY_APP_H_
#define _WX_MY_APP_H_

class wxMyApp:
    public wxApp
{
    protected:

    virtual void OnInitCmdLine(wxCmdLineParser& parser) wxOVERRIDE;
    virtual bool OnCmdLineParsed(wxCmdLineParser& parser) wxOVERRIDE;
    virtual bool OnInit() wxOVERRIDE;

    public:

    static const char APP_NAME[];
    static const char APP_VERSION[];
    static const char APP_VENDOR_NAME[];
    static const char APP_AUTHOR[];
    static const char LICENSE_FILE_NAME[];

    const wxFileName& GetMuToolPath() const;
    const wxFileName& GetScriptPath() const;

    void ShowToolPaths() const;
    bool ShowLogTimestamps(bool showTimestamps = true);
    bool GetFnColumn(const wxFileName&, wxVector<wxVariant>&) const;
    const wxIconBundle& GetAppIcon() const;

    protected:

    void fill_icon_map();

    protected:

    wxFileName m_scriptPath;
    wxFileName m_muToolPath;
    std::unordered_map<wxString, wxIconBundle> m_iconMap;
    wxIconBundle m_appIcons;

    public:

    wxMyApp(void);
};

wxDECLARE_APP(wxMyApp);
#endif

