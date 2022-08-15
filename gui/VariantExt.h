/*
 * VariantExt.h
 */

#ifndef _VARIANT_EXT_H_
#define _VARIANT_EXT_H_

class wxFileNameRefData :public wxRefCounter
{
    public:

    static wxObjectDataPtr<wxFileNameRefData> Get()
    {
        return wxObjectDataPtr<wxFileNameRefData>(new wxFileNameRefData());
    }

    static wxObjectDataPtr<wxFileNameRefData> Get(const wxFileName& fileName)
    {
        return wxObjectDataPtr<wxFileNameRefData>(new wxFileNameRefData(fileName));
    }

    protected:

    wxFileNameRefData()
    {}

    wxFileNameRefData(const wxFileName& fileName)
        :m_fileName(fileName)
    {}

    public:

    explicit wxFileNameRefData(const wxFileNameRefData& fn)
        :m_fileName(fn.m_fileName)
    {}

    const wxFileName& GetFileName() const
    {
        return m_fileName;
    }

    wxFileName& GetFileName()
    {
        return m_fileName;
    }

    protected:

    wxFileName m_fileName;
};

class wxRelativeFileName
{
    public:

    wxRelativeFileName()
        :m_relativeDir(wxFileNameRefData::Get())
    {}

    wxRelativeFileName(const wxFileName& fileName)
        :m_fileName(fileName),m_relativeDir(wxFileNameRefData::Get())
    {
        wxASSERT(!fileName.IsOk() || fileName.IsAbsolute());
    }

    wxRelativeFileName(const wxFileName& fileName, const wxFileName& relativeDir)
        :m_fileName(fileName),m_relativeDir(wxFileNameRefData::Get(relativeDir))
    {
        wxASSERT(!fileName.IsOk() || fileName.IsAbsolute());
        wxASSERT(!relativeDir.IsOk() || (relativeDir.IsDir() && relativeDir.IsAbsolute()));
    }

    wxRelativeFileName(const wxFileName& fileName, const wxObjectDataPtr<wxFileNameRefData>& relativeDir)
        :m_fileName(fileName), m_relativeDir(relativeDir)
    {
        wxASSERT(!fileName.IsOk() || fileName.IsAbsolute());
        wxASSERT(relativeDir);
        wxASSERT(!relativeDir->GetFileName().IsOk() || (relativeDir->GetFileName().IsDir() && relativeDir->GetFileName().IsAbsolute()));
    }

    wxRelativeFileName(const wxRelativeFileName& relFileName)
        :m_fileName(relFileName.m_fileName), m_relativeDir(relFileName.m_relativeDir)
    {}

    void SetRelativeDir(const wxFileName& relativeDir)
    {
        wxASSERT(!relativeDir.IsOk() || (relativeDir.IsDir() && relativeDir.IsAbsolute()));
        m_relativeDir->GetFileName().Assign(relativeDir);
    }

    bool IsOk() const
    {
        return m_fileName.IsOk();
    }

    bool SameAs(const wxRelativeFileName& rfn) const
    {
        if (m_fileName.IsOk())
        {
            if (!rfn.GetFileName().IsOk()) return false;
            return m_fileName.SameAs(rfn.GetFileName());
        }
        else
        {
            return !rfn.GetFileName().IsOk();
        }
    }

    const wxFileName& GetFileName() const
    {
        return m_fileName;
    }

    bool HasRelativeDir() const
    {
        return m_relativeDir->GetFileName().IsOk();
    }

    const wxFileName& GetRelativeDir() const
    {
        return m_relativeDir->GetFileName();
    }

    wxFileName GetRelativeFileName() const
    {
        if (m_relativeDir->GetFileName().IsOk())
        {
            wxFileName fn(m_fileName);
            if (fn.MakeRelativeTo(m_relativeDir->GetFileName().GetFullPath()))
            {
                return fn;
            }
            else
            {
                return m_fileName;
            }
        }
        else
        {
            return m_fileName;
        }
    }

    protected:

    wxFileName m_fileName;
    wxObjectDataPtr<wxFileNameRefData> m_relativeDir;
};

class wxResolutionOrScale
{
    public:

    wxResolutionOrScale(const wxResolutionOrScale& ros)
        :m_size(ros.m_size), m_resolution(ros.m_resolution)
    {}

    static wxResolutionOrScale Resolution(const wxSize& resolution)
    {
        return wxResolutionOrScale(resolution, true);
    }

    static wxResolutionOrScale Scale(const wxSize& scale)
    {
        return wxResolutionOrScale(scale, false);
    }

    bool HasResolution() const
    {
        return m_resolution;
    }

    bool HasScale() const
    {
        return !m_resolution;
    }

    const wxSize& GetSize() const
    {
        return m_size;
    }

    void GetScale(float& sx, float& sy) const
    {
        wxASSERT(!m_resolution);
        sx = (float)m_size.x / 100.0f;
        sy = (float)m_size.y / 100.0f;
    }

    bool operator==(const wxResolutionOrScale& ros) const
    {
        return (m_resolution == ros.m_resolution) && (m_size == ros.m_size);
    }

    protected:

    wxResolutionOrScale(const wxSize& size, const bool resolution)
        :m_size(size), m_resolution(resolution)
    {}

    protected:

    wxSize m_size;
    bool m_resolution;
};

class wxVariantDataFileName: public wxVariantData
{
    public:

    typedef wxFileName R;

    static wxVariant Get(const wxFileName& fn)
    {
        return wxVariant(new wxVariantDataFileName(fn));
    }

    explicit wxVariantDataFileName(const wxFileName& fn)
        :m_fn(fn)
    {
    }

    const wxFileName& GetValue() const { return m_fn; }
    void SetValue(const wxFileName& fn) { m_fn = fn; }

    virtual bool Eq(wxVariantData& data) const wxOVERRIDE
    {
        if (data.GetType().CmpNoCase(wxS("wxFileName")) != 0)
        {
            return false;
        }
        wxVariantDataFileName& fnData = static_cast<wxVariantDataFileName&>(data);
        return m_fn.SameAs(fnData.GetValue());
    }

#if wxUSE_STD_IOSTREAM
    virtual bool Write(wxSTD ostream& str) const wxOVERRIDE
    {
        if (m_fn.IsOk())
        {
            const wxString fnPath = m_fn.GetFullPath();
            const wxScopedCharBuffer buf = fnPath.ToUTF8();
            str.write(buf.data(), buf.length());
            return true;
        }
        else
        {
            return false;
        }
    }
#endif
    virtual bool Write(wxString& str) const wxOVERRIDE
    {
        if (m_fn.IsOk())
        {
            str = m_fn.GetFullPath();
            return true;
        }
        else
        {
            return false;
        }
    }

    wxVariantData* Clone() const wxOVERRIDE { return new wxVariantDataFileName(m_fn); }
    virtual wxString GetType() const wxOVERRIDE { return wxS("wxFileName"); }

    virtual bool GetAsAny(wxAny* any) const wxOVERRIDE
    {
        if (m_fn.IsOk())
        {
            *any = m_fn.GetFullPath();
        }
        else
        {
            any->MakeNull();
        }
        return true;
    }

    static wxVariantData* VariantDataFactory(const wxAny& any)
    {
        if (any.GetType()->CheckType<wxString>())
        {
            const wxString path = any.As<wxString>();
            return new wxVariantDataFileName(wxFileName::FileName(path));
        }

        return nullptr;
    }

    private:

    wxFileName m_fn;
};

class wxVariantDataRelativeFileName: public wxVariantData
{
    public:

    typedef wxRelativeFileName R;

    static wxVariant Get(const wxFileName& fn)
    {
        return wxVariant(new wxVariantDataRelativeFileName(fn));
    }

    static wxVariant Get(const wxRelativeFileName& rfn)
    {
        return wxVariant(new wxVariantDataRelativeFileName(rfn));
    }

    wxVariantDataRelativeFileName()
    {
    }

    explicit wxVariantDataRelativeFileName(const wxFileName& fn)
        :m_rfn(fn)
    {
    }

    explicit wxVariantDataRelativeFileName(const wxFileName& fileName, const wxFileName& relativeDir)
        :m_rfn(fileName, relativeDir)
    {
    }

    explicit wxVariantDataRelativeFileName(const wxRelativeFileName& rfn)
        :m_rfn(rfn)
    {
    }

    const wxRelativeFileName& GetValue() const { return m_rfn; }
    void SetValue(const wxRelativeFileName& rfn) { m_rfn = rfn; }

    virtual bool Eq(wxVariantData& data) const wxOVERRIDE
    {
        if (data.GetType().CmpNoCase(wxS("wxRelativeFileName")) != 0)
        {
            return false;
        }
        wxVariantDataRelativeFileName& fnData = static_cast<wxVariantDataRelativeFileName&>(data);
        return m_rfn.SameAs(fnData.GetValue());
    }

#if wxUSE_STD_IOSTREAM
    virtual bool Write(wxSTD ostream& str) const wxOVERRIDE
    {
        if (m_rfn.IsOk())
        {
            const wxString fnPath = m_rfn.GetRelativeFileName().GetFullPath();
            const wxScopedCharBuffer buf = fnPath.ToUTF8();
            str.write(buf.data(), buf.length());
            return true;
        }
        else
        {
            return false;
        }
    }
#endif
    virtual bool Write(wxString& str) const wxOVERRIDE
    {
        if (m_rfn.IsOk())
        {
            str = m_rfn.GetRelativeFileName().GetFullPath();
            return true;
        }
        else
        {
            return false;
        }
    }

    wxVariantData* Clone() const wxOVERRIDE { return new wxVariantDataRelativeFileName(m_rfn); }
    virtual wxString GetType() const wxOVERRIDE { return wxS("wxRelativeFileName"); }

    virtual bool GetAsAny(wxAny* any) const wxOVERRIDE
    {
        if (m_rfn.IsOk())
        {
            wxVector<wxFileName> afn;
            afn.push_back(m_rfn.GetFileName());
            afn.push_back(m_rfn.GetRelativeDir());
            *any = afn;
        }
        else
        {
            any->MakeNull();
        }
        return true;
    }

    static wxVariantData* VariantDataFactory(const wxAny& any)
    {
        if (any.GetType()->CheckType<wxVector<wxFileName>>())
        {
            wxVector<wxFileName> afn = any.As<wxVector<wxFileName>>();
            wxASSERT(afn.size() == 2);
            return new wxVariantDataRelativeFileName(afn[0], afn[1]);
        }

        return nullptr;
    }

    private:

    wxRelativeFileName m_rfn;
};

class wxVariantDataSize: public wxVariantData
{
    public:

    typedef wxSize R;

    static wxVariant Get(const wxSize& sz)
    {
        return wxVariant(new wxVariantDataSize(sz));
    }

    explicit wxVariantDataSize(const wxSize& size)
        :m_size(size)
    {
    }

    const wxSize& GetValue() const { return m_size; }
    void SetValue(const wxSize& size) { m_size = size; }

    virtual bool Eq(wxVariantData& data) const wxOVERRIDE
    {
        if (data.GetType().CmpNoCase(wxS("wxSize")) != 0)
        {
            return false;
        }
        wxVariantDataSize& sizeData = static_cast<wxVariantDataSize&>(data);
        return m_size == sizeData.GetValue();
    }

#if wxUSE_STD_IOSTREAM
    virtual bool Write(wxSTD ostream& str) const wxOVERRIDE
    {
        str << m_size.x << m_size.y;
        return true;
    }
#endif
    virtual bool Write(wxString& str) const wxOVERRIDE
    {
        if (m_size.x <= 0 || m_size.y <= 0)
        {
            str.Empty();
        }
        else if (m_size.x == m_size.y)
        {
            str << m_size.x;
        }
        else
        {
            str << m_size.x << wxS('\u00D7') << m_size.y;
        }
        return true;
    }

    wxVariantData* Clone() const wxOVERRIDE { return new wxVariantDataSize(m_size); }
    virtual wxString GetType() const wxOVERRIDE { return wxS("wxSize"); }

    virtual bool GetAsAny(wxAny* any) const wxOVERRIDE
    {
        return false;
    }

    private:

    wxSize m_size;
};

class wxVariantDataResolutionOrScale: public wxVariantData
{
    public:

    typedef wxResolutionOrScale R;

    static wxVariant Get(const wxResolutionOrScale& ros)
    {
        return wxVariant(new wxVariantDataResolutionOrScale(ros));
    }

    static wxVariant GetResolution(const wxSize& resolution)
    {
        return wxVariant(new wxVariantDataResolutionOrScale(wxResolutionOrScale::Resolution(resolution)));
    }

    static wxVariant GetScale(const wxSize& scale)
    {
        return wxVariant(new wxVariantDataResolutionOrScale(wxResolutionOrScale::Scale(scale)));
    }

    protected:

    explicit wxVariantDataResolutionOrScale(const wxResolutionOrScale& resolutionOrScale)
        :m_resolutionOrScale(resolutionOrScale)
    {
    }

    public:

    const wxResolutionOrScale& GetValue() const { return m_resolutionOrScale; }
    void SetValue(const wxResolutionOrScale& resolutionOrScale) { m_resolutionOrScale = resolutionOrScale; }

    virtual bool Eq(wxVariantData& data) const wxOVERRIDE
    {
        if (data.GetType().CmpNoCase(wxS("wxResolutionOrScale")) != 0)
        {
            return false;
        }
        wxVariantDataResolutionOrScale& rosData = static_cast<wxVariantDataResolutionOrScale&>(data);
        return m_resolutionOrScale == rosData.GetValue();
    }

#if wxUSE_STD_IOSTREAM
    virtual bool Write(wxSTD ostream& str) const wxOVERRIDE
    {
        const wxSize& sz = m_resolutionOrScale.GetSize();
        str << m_resolutionOrScale.HasResolution();
        str << sz.x << sz.y;
        return true;
    }
#endif
    virtual bool Write(wxString& str) const wxOVERRIDE
    {
        if (m_resolutionOrScale.HasResolution())
        {
            const wxSize& sz = m_resolutionOrScale.GetSize();
            if (sz.x <= 0 || sz.y <= 0)
            {
                str.Empty();
            }
            else if (sz.x == sz.y)
            {
                str << sz.x << wxS("\u200Adpi");
            }
            else
            {
                str << sz.x << wxS('\u00D7') << sz.y << wxS("\u200Adpi");
            }
        }
        else
        {
            const wxSize& sz = m_resolutionOrScale.GetSize();
            if (sz.x <= 0 || sz.y <= 0)
            {
                str.Empty();
            }
            else if (sz.x == sz.y)
            {
                str << sz.x << wxS("\u200A%");
            }
            else
            {
                str << sz.x << wxS('\u00D7') << sz.y << wxS("\u200A%");
            }
        }
        return true;
    }

    wxVariantData* Clone() const wxOVERRIDE { return new wxVariantDataResolutionOrScale(m_resolutionOrScale); }
    virtual wxString GetType() const wxOVERRIDE { return wxS("wxResolutionOrScale"); }

    virtual bool GetAsAny(wxAny* any) const wxOVERRIDE
    {
        return false;
    }

    private:

    wxResolutionOrScale m_resolutionOrScale;
};

#endif