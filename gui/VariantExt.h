/*
 * VariantExt.h
 */

#ifndef _VARIANT_EXT_H_
#define _VARIANT_EXT_H_

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

#endif