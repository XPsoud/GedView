#include "main.h"

#include "mainframe.h"
#include "appversion.h"
#include "settingsmanager.h"

IMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Initializing a \"wxApp\" derived object\n"));
#endif // DEBUG

    wxInitAllImageHandlers();
    // Set application name
    SetAppName(_T(PRODUCTNAME));
    // Init the SettingManager instance
    SettingsManager& settings=SettingsManager::Get();
    // Read the settings file
    settings.ReadSettings();

    MainFrame* frame = new MainFrame(GetVersionString(true));
    SetTopWindow(frame);
    frame->Show();
    frame->Raise();

    return true;
}

int MyApp::OnExit()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Exiting from a \"wxApp\" derived object\n"));
#endif // __WXDEBUG__

    // If they have been modified, save the settings
    SettingsManager& settings=SettingsManager::Get();
    if (settings.IsModified())
        settings.SaveSettings();

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

wxString MyApp::GetVersionString(bool full, bool build)
{
    wxString sRes=wxTheApp->GetAppName();
    sRes << _T(" (v") << VERSION_MAJOR << _T(".") << VERSION_MINOR;
    if (full) sRes << _T(".") << VERSION_REV;
    sRes << _T(")");
    if (build)
        sRes << _T(" build ") << VERSION_BUILD;
    return sRes;
}

int MyApp::OnRun()
{
    return wxApp::OnRun();
}
