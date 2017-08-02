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
    // Read the settings file
    settings.ReadSettings();
    // Initialize the local if possible (and if wanted)
    // Easter egg for debugging purpose
    bool bI18N=(settings.GetProhibitI18N()==false);
    if (wxGetKeyState(WXK_SHIFT))
        bI18N=!bI18N;
    if (bI18N)
    {
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
#ifdef __WXGTK__
        m_locale.AddCatalog(_T("wxStd")); // Don't know yet why it isn't loaded automatically :-/
#endif // __WXGTK__
    }

    // Check for single instance
    m_pSnglInstChkr=NULL;
    if (!settings.GetMultipleInstancesAllowed())
    {
        m_pSnglInstChkr=new wxSingleInstanceChecker;

        m_pSnglInstChkr->Create(_T(PRODUCTNAME));
        if (m_pSnglInstChkr->IsAnotherRunning())
        {
            wxMessageBox(_("An other instance of this application is already running !"), _("Multiple instances forbidden"), wxICON_EXCLAMATION|wxOK|wxCENTER);
            delete m_pSnglInstChkr;
            return false;
        }
    }

    MainFrame* frame = new MainFrame(GetVersionString(true));
    SetTopWindow(frame);
    frame->Show();
    frame->Raise();

#ifndef __WXMAC__
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
#endif // ndef __WXMAC__
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

    if (m_pSnglInstChkr!=NULL)
        delete m_pSnglInstChkr;

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

#ifdef __WXMAC__
void MyApp::MacOpenFiles(const wxArrayString& fileNames)
{
    if (!fileNames.IsEmpty())
    {
        wxCommandEvent evt(wxEVT_FILEOPEN);
        evt.SetString(fileNames[0]);
        MainFrame* frm=(MainFrame*)GetTopWindow();
        if (frm!=NULL)
            frm->GetEventHandler()->AddPendingEvent(evt);
    }
}
#endif // __WXMAC__
