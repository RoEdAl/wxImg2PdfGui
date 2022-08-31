/*
 * wxApp.cpp
 */

#include <app_config.h>
#include <wxCmdTools/wxCmdTool.h>
#include "wxApp.h"
#include "wxMainFrame.h"
#include "VariantExt.h"

 // ===============================================================================

const char wxMyApp::APP_NAME[] = "img2pdf-frontend";
const char wxMyApp::APP_VERSION[] = PROJECT_VERSION_STR;
const char wxMyApp::APP_VENDOR_NAME[] = "Edmunt Pienkowsky";
const char wxMyApp::APP_AUTHOR[] = "Edmunt Pienkowsky - roed@onet.eu";
const char wxMyApp::LICENSE_FILE_NAME[] = "license.txt";

// ===============================================================================

namespace
{
#pragma pack(push)
#pragma pack(2)

    // icon entry in the icon directory resource
    typedef struct
    {
        BYTE   bWidth;               // Width, in pixels, of the image
        BYTE   bHeight;              // Height, in pixels, of the image
        BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
        BYTE   bReserved;            // Reserved
        WORD   wPlanes;              // Color Planes
        WORD   wBitCount;            // Bits per pixel
        DWORD  dwBytesInRes;         // how many bytes in this resource?
        WORD   nID;                  // the ID
    } GRPICONDIRENTRY, * LPGRPICONDIRENTRY;

    // icon directory resource
    typedef struct
    {
        WORD            idReserved;   // Reserved (must be 0)
        WORD            idType;       // Resource type (1 for icons)
        WORD            idCount;      // How many images?
        GRPICONDIRENTRY idEntries[1]; // The entries for each image
    } GRPICONDIR, * LPGRPICONDIR;

#pragma pack(pop)

    typedef std::pair<WXWORD, wxIcon> DepthAndIcon;
    typedef std::vector<DepthAndIcon> TmpIconVector;

    bool load_from_resource(TmpIconVector& icons, const wxString& name, WXHINSTANCE module)
    {
        const void* data = NULL;
        size_t outLen = 0;

        // load the icon directory resource
        if (!wxLoadUserResource(&data, &outLen, name, RT_GROUP_ICON, module))
        {
            wxLogError(_("Failed to load icons from resource '%s'."), name);
            return false;
        }

        // load the individual icons referred from the icon directory
        const GRPICONDIR* const grpIconDir = static_cast<const GRPICONDIR*>(data);

        for (WORD i = 0; i < grpIconDir->idCount; ++i)
        {
            const GRPICONDIRENTRY& iconDir = grpIconDir->idEntries[i];
            const WORD iconID = iconDir.nID;

            if (wxLoadUserResource(&data, &outLen, wxString::Format(wxS("#%u"), iconID), RT_ICON, module))
            {
                const WXHICON hIcon = CreateIconFromResourceEx(
                    static_cast<PBYTE>(const_cast<void*>(data)),
                    static_cast<DWORD>(outLen),
                    TRUE,
                    0x00030000,
                    0, 0,
                    LR_DEFAULTCOLOR);
                if (hIcon == NULL)
                {
                    wxLogDebug(wxS("Failed to load icon from resource with id %u."), iconID);
                    continue;
                }

                wxIcon icon;

                if (icon.CreateFromHICON(hIcon))
                {
                    const WXWORD iconDepth = iconDir.wBitCount * iconDir.wPlanes;
                    //if (iconDepth > 0) icon.SetDepth(iconDepth);
                    icons.push_back(std::make_pair(iconDepth, icon));
                }
                else
                {
                    DestroyIcon(hIcon);
                    wxLogDebug(wxS("Failed to create icon from resource with id %u."), iconID);
                }
            }
            else
            {
                wxLogDebug(wxS("Failed to load icon with id %u for group icon resource '%s'."), iconID, name);
            }
        }

        return true;
    }

    bool load_png_from_resource(wxBitmap& bitmap, const wxString& name, WXHINSTANCE module)
    {
        const void* data = NULL;
        size_t outLen = 0;

        // load the PNG resource
        if (!wxLoadUserResource(&data, &outLen, name, wxS("PNG"), module))
        {
            wxLogError(_("Failed to load PNG from resource '%s'."), name);
            return false;
        }

        const wxBitmap bmp = wxBitmap::NewFromPNGData(data, outLen);
        if (bmp.IsOk())
        {
            bitmap = bmp;
            return true;
        }
        
        return false;
    }

    class IconBundleHolder
    {
        wxDECLARE_NO_COPY_CLASS(IconBundleHolder);

        public:

        IconBundleHolder(wxIconBundle& iconBundle, const int iconPos = 0)
            :m_iconBundle(iconBundle), m_iconPos(iconPos), m_pos(0), m_iconsLoaded(false)
        {
        }

        void Load(WXHINSTANCE hModule, const wxString& resName)
        {
            if (!(m_iconsLoaded = load_from_resource(m_icons, resName, hModule)))
            {
                return;
            }

            std::sort(m_icons.begin(), m_icons.end(), icon_cmp);
            for(const auto& i : m_icons)
            {
                m_iconBundle.AddIcon(i.second);
            }
        }

        void Load(WXHINSTANCE hModule)
        {
            if (m_iconPos < 0)
            {
                const WXWORD iconId = static_cast<WXWORD>(-m_iconPos);
                const wxString iconIdStr = wxString::Format(wxS("#%u"), iconId);
                if (!(m_iconsLoaded = load_from_resource(m_icons, iconIdStr, hModule)))
                {
                    return;
                }
            }
            else
            {
                EnumResourceNames(hModule, RT_GROUP_ICON, ResEnumProc, reinterpret_cast<LONG_PTR>(this));
                if (!m_iconsLoaded)
                {
                    return;
                }
            }

            std::sort(m_icons.begin(), m_icons.end(), icon_cmp);
            for(const auto& i : m_icons)
            {
                m_iconBundle.AddIcon(i.second);
            }
        }

        bool IconsLoaded() const
        {
            return m_iconsLoaded;
        }

        protected:

        bool Process(WXHINSTANCE hModule, LPCWSTR pszName)
        {
            if (m_iconPos == 0 || ++m_pos == m_iconPos)
            {
                if (IS_INTRESOURCE(pszName))
                {
                    const wxString resName = wxString::Format("#%" wxLongLongFmtSpec "u", reinterpret_cast<ULONG_PTR>(pszName));
                    if (!load_from_resource(m_icons, resName, hModule)) return false;
                }
                else
                {
                    if (!load_from_resource(m_icons, pszName, hModule)) return false;
                }

                m_iconsLoaded = true;
            }
            return m_iconPos > 0 && m_pos < m_iconPos;
        }

        static bool Process(WXHINSTANCE hModule, LPCWSTR pszName, LONG_PTR paramThis)
        {
            IconBundleHolder* const pThis = reinterpret_cast<IconBundleHolder*>(paramThis);
            return pThis->Process(hModule, pszName);
        }

        static BOOL CALLBACK ResEnumProc(HMODULE hModule, LPCWSTR pszType, LPWSTR pszName, LONG_PTR param)
        {
            return Process(hModule, pszName, param);
        }

        static bool icon_cmp(const DepthAndIcon& i1, const DepthAndIcon& i2)
        {
            const wxSize sz1 = i1.second.GetSize();
            const wxSize sz2 = i2.second.GetSize();

            const int px1 = sz1.GetWidth() * sz1.GetHeight();
            const int px2 = sz2.GetWidth() * sz2.GetHeight();

            if (px1 < px2)
            {
                return true;
            }
            else if (px1 > px2)
            {
                return false;
            }
            else
            {
                const int d1 = i1.first;
                const int d2 = i2.first;

                return d1 < d2;
            }
        }

        protected:

        const int m_iconPos;
        int m_pos;
        bool m_iconsLoaded;
        TmpIconVector m_icons;
        wxIconBundle& m_iconBundle;
    };

    class ResourceModuleLoader
    {
        wxDECLARE_NO_COPY_CLASS(ResourceModuleLoader);

        public:

        ResourceModuleLoader(const wxString& moduleName)
        {
            m_hModule = LoadLibraryEx(moduleName.wc_str(), NULL, LOAD_LIBRARY_AS_IMAGE_RESOURCE | LOAD_LIBRARY_AS_DATAFILE);
        }

        ~ResourceModuleLoader()
        {
            if (m_hModule != NULL) FreeLibrary(m_hModule);
        }

        operator bool() const
        {
            return m_hModule != NULL;
        }

        operator WXHINSTANCE() const
        {
            return m_hModule;
        }

        protected:

        WXHINSTANCE m_hModule;
    };

    bool load_icons(const wxIconLocation& iconLocation, wxIconBundle& iconBundle)
    {
        ResourceModuleLoader resourceLoader(iconLocation.GetFileName());
        if (!resourceLoader) return false;

        IconBundleHolder bundleHolder(iconBundle, iconLocation.GetIndex());
        bundleHolder.Load(resourceLoader);

        return bundleHolder.IconsLoaded();
    }

    bool load_icons(const wxFileName& iconLocation, const wxString& resName, wxIconBundle& iconBundle)
    {
        ResourceModuleLoader resourceLoader(iconLocation.GetFullPath());
        if (!resourceLoader) return false;

        IconBundleHolder bundleHolder(iconBundle);
        bundleHolder.Load(resourceLoader, resName);

        return bundleHolder.IconsLoaded();
    }

    double window_variant_to_scale(const wxWindowVariant windowVariant)
    {
        switch (windowVariant)
        {
            case wxWINDOW_VARIANT_SMALL:
            return 1.25;

            case wxWINDOW_VARIANT_MINI:
            return 1.25 * 1.25;

            case wxWINDOW_VARIANT_LARGE:
            return 1.0 / 1.25;

            default:
            return 1.0;
        }
    }

    double get_scale_factor(const wxWindow* const wnd)
    {
        if (wnd != nullptr)
        {
            return window_variant_to_scale(wnd->GetWindowVariant());
        }
        else
        {
            return window_variant_to_scale(wxWINDOW_VARIANT_NORMAL);
        }
    }

    bool load_pngs(const wxFileName& iconLocation, const wxString& resName, wxBitmapBundle& bitmapBundle, const double scaleFactor, const bool disabled = false)
    {
        ResourceModuleLoader resourceLoader(iconLocation.GetFullPath());
        if (!resourceLoader) return false;

        wxVector<wxBitmap> bitmaps;
        const std::vector<int> sz{ 18, 24, 36, 48, 64, 96, 128, 256 };
        for (auto i : sz)
        {
            const wxString rn = wxString::Format("%s-%d", resName, i);
            wxBitmap bmp;
            if (!load_png_from_resource(bmp, rn, resourceLoader)) return false;
            if (disabled)
            {
                wxBitmap dbmp = bmp.ConvertToDisabled();
                dbmp.SetScaleFactor(scaleFactor);
                bitmaps.push_back(dbmp);
            }
            else
            {
                bmp.SetScaleFactor(scaleFactor);
                bitmaps.push_back(bmp);
            }
        }

        bitmapBundle = wxBitmapBundle::FromBitmaps(bitmaps);
        return true;
    }

    void bmp_to_ico_bundle(const wxBitmapBundle& bitmapBundle, wxIconBundle& iconBundle)
    {
        const std::vector<int> sz{ 18, 24, 36, 48, 64, 96, 128, 256 };
        for (auto i : sz)
        {
            const wxSize szs(i, i);
            iconBundle.AddIcon(bitmapBundle.GetIcon(szs));
        }
    }

    bool get_file_type_icons(const wxString& fext, wxIconBundle& iconBundle)
    {
        const wxScopedPtr<wxFileType> ft(wxTheMimeTypesManager->GetFileTypeFromExtension(fext));
        if (!ft) return false;

        wxIconLocation iconLocation;
        if (!ft->GetIcon(&iconLocation)) return false;

        return load_icons(iconLocation, iconBundle);
    }

    wxString dot_ext(const wxString& ext)
    {
        wxString res(ext);
        res.Prepend('.');
        return res;
    }

    bool add_icons_for_ext(const wxString& ext, std::unordered_map<wxString, wxIconBundle>& iconMap)
    {
        const wxString dotExt(dot_ext(ext));
        wxIconBundle icons;

        if (get_file_type_icons(dotExt, icons))
        {
            iconMap[ext] = icons;
            return true;
        }

        return false;
    }

    bool get_fn_ext(const wxRelativeFileName& rfn, wxString& ext)
    {
        const wxFileName& fn = rfn.GetFileName();
        if (!fn.HasExt()) return false;
        ext = fn.GetExt();
        ext.LowerCase();
        return true;
    }
}

bool wxMyApp::LoadMaterialDesignIcon(const wxWindow* const wnd, const wxString& resName, wxBitmapBundle& bitmapBundle) const
{
    wxASSERT(MaterialDesignIconsFound());
    const double bitmapScale = get_scale_factor(wnd);
    return load_pngs(m_materialDesignIconsPath, resName, bitmapBundle, bitmapScale, false);
}

bool wxMyApp::LoadMaterialDesignIcon(const wxString& resName, const wxWindowVariant windowVariant, wxBitmapBundle& bitmapBundle) const
{
    wxASSERT(MaterialDesignIconsFound());
    const double bitmapScale = window_variant_to_scale(windowVariant);
    return load_pngs(m_materialDesignIconsPath, resName, bitmapBundle, bitmapScale, false);
}

bool wxMyApp::LoadDisabledMaterialDesignIcon(const wxString& resName, const wxWindowVariant windowVariant, wxBitmapBundle& bitmapBundle) const
{
    wxASSERT(MaterialDesignIconsFound());
    const double bitmapScale = window_variant_to_scale(windowVariant);
    return load_pngs(m_materialDesignIconsPath, resName, bitmapBundle, bitmapScale, true);
}

void wxMyApp::fill_icon_map()
{
    wxBitmapBundle bmpImg;
    const bool imgLoaded = LoadMaterialDesignIcon("image-image", wxWINDOW_VARIANT_NORMAL, bmpImg);
    wxASSERT(imgLoaded);

    wxIconBundle icoImg;
    bmp_to_ico_bundle(bmpImg, icoImg);

    std::unordered_map<wxString, wxIconBundle> iconMap;

    if (!add_icons_for_ext("jpg", iconMap))
    {
        iconMap["jpg"] = icoImg;
    }

    if (!add_icons_for_ext("jpeg", iconMap))
    {
        iconMap["jpeg"] = icoImg;
    }

    if (!add_icons_for_ext("jp2", iconMap))
    {
        iconMap["jp2"] = icoImg;
    }

    if (!add_icons_for_ext("png", iconMap))
    {
        iconMap["png"] = icoImg;
    }

    if (!add_icons_for_ext("svg", iconMap))
    {
        iconMap["svg"] = icoImg;
    }

    if (add_icons_for_ext("pdf", iconMap))
    {
        m_appIcons = iconMap["pdf"];
    }
    else
    {
        const wxIconBundle iconBundle("ico_picture_as_pdf", nullptr);
        wxASSERT(iconBundle.IsOk());
        m_appIcons = iconBundle;
        iconMap["pdf"] = iconBundle;
    }

    for (const auto& i : iconMap)
    {
        m_bitmapMap[i.first] = wxBitmapBundle::FromIconBundle(i.second);
    }
}

bool wxMyApp::GetFnColumn(const wxRelativeFileName& rfn, wxVector<wxVariant>& column) const
{
    wxString ext;
    if (!get_fn_ext(rfn, ext)) return false;

    try
    {
        const bool undeterminedSize = ext.CmpNoCase("pdf") == 0 || ext.CmpNoCase("jp2") == 0 || ext.CmpNoCase("svg") == 0;
        const wxSize elSize(undeterminedSize ? 0 : -1, undeterminedSize ? 0 : -1);

        column.push_back(wxVariant(m_bitmapMap.at(ext)));
        column.push_back(wxVariantDataRelativeFileName::Get(rfn));
        column.push_back(wxVariantDataSize::Get(elSize));
        if (undeterminedSize)
        {
            column.push_back(wxVariantDataResolutionOrScale::GetScale(elSize));
        }
        else
        {
            column.push_back(wxVariantDataResolutionOrScale::GetResolution(elSize));
        }
        column.push_back(wxVariant(m_emptyStr));
        column.push_back(m_null);
        return true;
    }
    catch (std::out_of_range WXUNUSED(oe))
    {
        return false;
    }
}

const wxIconBundle& wxMyApp::GetAppIcon() const
{
    return m_appIcons;
}

const wxVariant& wxMyApp::GetEmptyString() const
{
    return m_emptyStr;
}

// ===============================================================================

wxIMPLEMENT_APP(wxMyApp);

wxMyApp::wxMyApp(void)
    :m_emptyStr(wxString())
{
}

void wxMyApp::OnInitCmdLine(wxCmdLineParser& parser)
{
    wxApp::OnInitCmdLine(parser);

    parser.AddLongSwitch("log-timestamps", _("Show/hide log timestamps"), wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_SWITCH_NEGATABLE);
}

bool wxMyApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
    wxASSERT(parser.AreLongOptionsEnabled());

    const bool res = wxApp::OnCmdLineParsed(parser);

    if (res)
    {
        const wxCmdLineSwitchState state = parser.FoundSwitch("verbose");
        switch (state)
        {
            case wxCMD_SWITCH_ON:
            {
                // wxLog::SetLogLevel(wxLOG_Info);
                wxLog::SetVerbose(true);
                break;
            }

            default:
            {
                // wxLog::SetLogLevel(wxLOG_Message);
                wxLog::SetVerbose(false);
                break;
            }
        }

        if (parser.FoundSwitch("log-timestamps") == wxCMD_SWITCH_OFF) wxLog::DisableTimestamp();
    }

    return res;
}

bool wxMyApp::OnInit()
{
    SetAppName(APP_NAME);
    SetVendorName(APP_VENDOR_NAME);
    SetVendorDisplayName(APP_AUTHOR);

    if (!wxApp::OnInit()) return false;

    wxInitAllImageHandlers();

    wxLog::EnableLogging(false);
#ifdef NDEBUG
    wxDisableAsserts();
#endif

    m_scriptPath.Assign(wxStandardPaths::Get().GetExecutablePath());
    m_scriptPath.SetFullName("img2pdf.js");

    if (!m_scriptPath.IsFileReadable())
    {
    #ifdef NDEBUG
        wxLogDebug(_("Using img2pdf.js tool from current directory"));

        const wxString fullName(m_scriptPath.GetFullName());
        m_scriptPath.AssignCwd();
        m_scriptPath.SetFullName(fullName);
    #else
        const wxArrayString& dirs = m_scriptPath.GetDirs();
        m_scriptPath.RemoveLastDir();
        m_scriptPath.RemoveLastDir();
        m_scriptPath.AppendDir("js");
        m_scriptPath.AppendDir("public");
    #endif
    }

    m_materialDesignIconsPath.Assign(wxStandardPaths::Get().GetExecutablePath());
    m_materialDesignIconsPath.SetFullName("material-design-icons.dll");

    if (m_materialDesignIconsPath.IsFileReadable())
    {
        fill_icon_map();
    }
    else
    {
        wxLogWarning(_("Unable to find material-design-icons.dll"));
        return false;
    }

    if (!wxCmdTool::FindTool(wxCmdTool::TOOL_MUTOOL, m_muToolPath))
    {
        wxLogWarning(_("mutool tool not found."));
    }

    if (!wxCmdTool::FindLocalTool(wxCmdTool::TOOL_SUMATRA_PDF, m_sumatraPdfPath))
    {
        wxLogVerbose(_("SumatraPDF not found."));
    }

    wxFrame* const pFrame = new wxMainFrame();
    pFrame->Show(true);

    return true;
}

bool wxMyApp::MaterialDesignIconsFound() const
{
    return m_materialDesignIconsPath.IsFileReadable();
}

const wxFileName& wxMyApp::GetMaterialDesignIconsPath() const
{
    return m_materialDesignIconsPath;
}

const wxFileName& wxMyApp::GetScriptPath() const
{
    return m_scriptPath;
}

const wxFileName& wxMyApp::GetMuToolPath() const
{
    return m_muToolPath;
}

bool wxMyApp::SumatraPdfFound() const
{
    return m_sumatraPdfPath.IsOk();
}

const wxFileName& wxMyApp::GetSumatraPdfPath() const
{
    return m_sumatraPdfPath;
}

namespace
{
    wxString get_short_path_if_possible(const wxFileName& fn)
    {
        const wxString res = fn.GetShortPath();
        return res.IsEmpty() ? fn.GetFullPath() : res;
    }
}

bool wxMyApp::RunDocViewer(const wxFileName& doc) const
{
    wxASSERT(doc.IsFileReadable());

    const wxString sumatraPdfPath = get_short_path_if_possible(m_sumatraPdfPath);

    wxLogMessage(_("Launching viewer for %s"), doc.GetFullName());
    const wxString cmd = wxString::Format("\"%s\" -new-window -restrict -view \"single page\" \"%s\"", 
                                          sumatraPdfPath,
                                          doc.GetFullPath());

    wxExecuteEnv env;
    env.cwd = m_sumatraPdfPath.GetPath();

    const long pid = wxExecute(cmd, 0, nullptr, &env);
    if (pid <= 0)
    {
        wxLogWarning(_("Fail to run SumatraPDF viewer"));
        wxLogVerbose(_("cmd: %s"), cmd);
        return false;
    }

    return true;
}

namespace
{
    void show_tool_path(const wxFileName& tool)
    {
        if (tool.IsOk() && tool.IsAbsolute())
        {
            wxLogMessage("%-10s: %s", tool.GetName(), tool.GetFullPath());
        }
        else
        {
            wxLogMessage(_("%-10s: <not found>"), tool.GetName());
        }
    }
}

void wxMyApp::ShowToolPaths() const
{
    show_tool_path(m_muToolPath);
    show_tool_path(m_sumatraPdfPath);
    show_tool_path(m_scriptPath);
}
