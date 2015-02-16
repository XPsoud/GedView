#ifndef __MAIN_H_INCLUDED__
#define __MAIN_H_INCLUDED__

#include <wx/app.h>
#include <wx/snglinst.h>

class MyApp : public wxApp
{
    public:
        virtual bool OnInit();
        virtual int OnRun();
        virtual int OnExit();
        wxString GetBuildInfos(bool all=false);
    private:
        wxLocale m_locale;
};

DECLARE_APP(MyApp)

#endif // __MAIN_H_INCLUDED__
