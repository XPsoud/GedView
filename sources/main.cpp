#include "main.h"

#include "mainframe.h"
#include "appversion.h"
#include "settingsmanager.h"

#include <wx/stdpaths.h>

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
    // Initialize the local if possible
#ifdef __WXMAC__
    wxString sDir=wxStandardPaths::Get().GetResourcesDir();
#else
    wxString sDir=settings.GetAppPath();
#endif // __WXMAC__
    if (!sDir.EndsWith(wxFileName::GetPathSeparator()))
        sDir.Append(wxFileName::GetPathSeparator());
    sDir.Append(_T("langs"));
    m_locale.AddCatalogLookupPathPrefix(sDir);
    m_locale.Init(wxLANGUAGE_DEFAULT, wxLOCALE_LOAD_DEFAULT);
    m_locale.AddCatalog(_T("GedView"));
    // Read the settings file
    settings.ReadSettings();

    MainFrame* frame = new MainFrame(GetVersionString(true));
    SetTopWindow(frame);
    frame->Show();
    frame->Raise();

    // Is there a file to open given by command line ?
    if (argc>1)
    {
        wxFileName FName(argv[1]);
        if ((!FName.IsOk()) || (!FName.FileExists()) || (FName.GetExt().Upper()!=_T("GED")))
        {
            wxString sErr=_("Invalid file given to command line!");
            sErr << _T("\n") << argv[1];
            wxMessageBox(sErr, _("Error"), wxICON_EXCLAMATION|wxCENTER|wxOK);
        }
        else
        {
            wxCommandEvent evt(wxEVT_FILEOPEN, -1);
            evt.SetString(FName.GetFullPath());
            frame->GetEventHandler()->AddPendingEvent(evt);
        }
    }

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
