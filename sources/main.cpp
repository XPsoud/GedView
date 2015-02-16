#include "main.h"

#include "mainframe.h"
#include "appversion.h"


IMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Initializing a \"wxApp\" derived object\n"));
#endif // DEBUG

    wxInitAllImageHandlers();
    // Application name
    SetAppName(_T(PRODUCTNAME));

    MainFrame* frame = new MainFrame(wxString::Format(_("GedView (v%d.%d.%d)"), VERSION_MAJOR, VERSION_MINOR, VERSION_REV));
    frame->Show();

    return true;
}

int MyApp::OnExit()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Exiting from a \"wxApp\" derived object\n"));
#endif // __WXDEBUG__
    return wxApp::OnExit();
}

wxString MyApp::GetBuildInfos(bool all)
{
    wxString wxbuild(wxVERSION_STRING);
    if (sizeof(void *)==4)
        wxbuild << _T("-32 Bits");
    else
        wxbuild << _T("-64 Bits");
    if (all)
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__WXMAC__)
        wxbuild << _T("-OSX");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if defined(wxUSE_UNICODE)
        wxbuild << _T("-Unicode");
#else
        wxbuild << _T("-Ansi");
#endif // wxUSE_UNICODE

#if defined(WXUSINGDLL)
		wxbuild << _T("-Dynamic");
#else
		wxbuild << _T("-Static");
#endif

		wxbuild << _T(" build");
    }
    return wxbuild;
}

int MyApp::OnRun()
{
    return wxApp::OnRun();
}
