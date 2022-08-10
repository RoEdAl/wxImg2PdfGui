/*
 *      MyLogStderr.h
 */

#ifndef _LOG_LIST_BOX_H_
#define _LOG_LIST_BOX_H_

class ListBox:
    public wxListBox
{
    public:

    ListBox(wxWindow*);
    void ShowLastItem();
    wxString GetItemsAsText() const;
};

class LogListBox:
    public wxLog
{
    public:

    LogListBox(ListBox*);
    virtual void DoLogText(const wxString&);

    protected:

    ListBox* m_listBox;
};

class SimpleLogListBox:
    public wxLog
{
    public:

    SimpleLogListBox(ListBox*);
    virtual void DoLogText(const wxString&);

    protected:

    ListBox* m_listBox;
};

#endif

