/*
 * FmtSpec.h
 */

#ifndef _FMT_SPEC_H_
#define _FMT_SPEC_H_

#ifdef NDEBUG
#define ENQUOTED_STR_FMT "\u201C%s\u201D"
#else
#define ENQUOTED_STR_FMT "\"%s\""
#endif

#ifndef WXINTL_NO_GETTEXT_MACRO

#ifndef wxNO_IMPLICIT_WXSTRING_ENCODING
#define _wxS(s)                               wxGetTranslation(wxS(s))
#else
#define _wxS(s)                               wxGetTranslation(wxString::FromUTF8(s))
#endif

#endif

#if defined( __WIN64__ )
#define wxSizeTFmtSpec wxLongLongFmtSpec
#define wxTimeTFmtSpec wxLongLongFmtSpec

namespace
{
    inline wxTextOutputStream& WriteSizeT(wxTextOutputStream& stream, size_t c)
    {
        stream.Write64(c);
        return stream;
    }

    inline wxTextOutputStream& WriteTimeT(wxTextOutputStream& stream, time_t t)
    {
        stream.Write64(t);
        return stream;
    }
}

#else
#define wxSizeTFmtSpec
#define wxTimeTFmtSpec

namespace
{
    inline wxTextOutputStream& WriteSizeT(wxTextOutputStream& stream, size_t c)
    {
        stream.Write32(c);
        return stream;
    }

    inline wxTextOutputStream& WriteTimeT(wxTextOutputStream& stream, time_t t)
    {
        stream.Write32(t);
        return stream;
    }
}

#endif

#endif
