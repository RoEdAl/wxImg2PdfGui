/*
 * wxApp.h
 */

#ifndef _WX_MY_APP_H_
#define _WX_MY_APP_H_

#ifndef _VARIANT_EXT_H_
#include "VariantExt.h"
#endif

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

    bool MaterialDesignIconsFound() const;
    const wxFileName& GetMaterialDesignIconsPath() const;
    bool LoadMaterialDesignIcon(const wxString&, wxBitmapBundle&) const;

    const wxFileName& GetMuToolPath() const;
    const wxFileName& GetScriptPath() const;

    bool SumatraPdfFound() const;
    const wxFileName& GetSumatraPdfPath() const;
    bool RunDocViewer(const wxFileName&) const;

    void ShowToolPaths() const;
    bool ShowLogTimestamps(bool showTimestamps = true);
    bool GetFnColumn(const wxRelativeFileName&, wxVector<wxVariant>&) const;
    const wxIconBundle& GetAppIcon() const;
    const wxVariant& GetEmptyString() const;

    protected:

    void fill_icon_map();

    protected:

    wxFileName m_materialDesignIconsPath;
    wxFileName m_scriptPath;
    wxFileName m_muToolPath;
    wxFileName m_sumatraPdfPath;
    std::unordered_map<wxString, wxBitmapBundle> m_bitmapMap;
    wxIconBundle m_appIcons;
    wxVariant m_emptyStr;
    wxVariant m_null;

    public:

    wxMyApp(void);
};

wxDECLARE_APP(wxMyApp);
#endif

