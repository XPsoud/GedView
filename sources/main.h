#ifndef __MAIN_H_INCLUDED__
#define __MAIN_H_INCLUDED__

#include <wx/app.h>
#include <wx/snglinst.h>

class MyApp : public wxApp
{
    public:
        virtual bool OnInit();
        virtual int OnExit();
        wxString GetBuildInfos(bool all=false);
        wxString GetVersionString(bool full=false, bool build=false);
    private:
#ifdef __WXMAC__
        virtual void MacOpenFiles(const wxArrayString& fileNames);
#endif // __WXMAC
        wxLocale m_locale;
        wxSingleInstanceChecker *m_pSnglInstChkr;
};

DECLARE_APP(MyApp)

#endif // __MAIN_H_INCLUDED__
