#include "settingsmanager.h"

const wxChar* szStdXmlFileHeader=_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
const int iStdXmlHeaderSize=38;

#include <wx/dir.h>
#include <wx/zstream.h>
#include <wx/xml/xml.h>
#include <wx/wfstream.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

SettingsManager SettingsManager::m_instance;

SettingsManager::SettingsManager()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Creating a \"SettingsManager\" object\n"));
#endif // __WXDEBUG__
    m_bInitialized=false;
    m_bModified=false;
}

SettingsManager::~SettingsManager()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Destroying a \"SettingsManager\" object\n"));
#endif // __WXDEBUG__
}

SettingsManager& SettingsManager::Get()
{
    if (!m_instance.m_bInitialized)
        m_instance.Initialize();

    return m_instance;
}

void SettingsManager::Initialize()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Initializing the SettingsManager\n"));
#endif // ENABLE_DEBUG_MSG
    // Full path of the application
    wxFileName fname(wxStandardPaths::Get().GetExecutablePath());
    fname.Normalize();
    m_sAppPath=fname.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);
    // Path for the datas file(s) (platform dependant)
    m_sDatasPath=wxStandardPaths::Get().GetUserDataDir();
    if (!m_sDatasPath.EndsWith(wxFileName::GetPathSeparator()))
        m_sDatasPath.Append(wxFileName::GetPathSeparator());
    // Default position and size of the main window
    m_iStartPos=wxCENTER_ON_SCREEN;
    m_ptStartPos=wxDefaultPosition;
    m_szStartSize=wxDefaultSize;
    // Other default settings
    m_bShowSplashScreen=true;
    m_bSingleInstance=true;
    m_bCompDatas=false;
    m_bCompSettings=false;
    m_sPassword=wxEmptyString;

    m_bInitialized=true;
}

bool SettingsManager::ReadSettings()
{
    if (!m_bInitialized) Initialize();

    wxString sFName=m_sDatasPath + _T("Settings.gvw");

    if (!wxFileExists(sFName)) return false;

    wxXmlDocument doc;
    wxXmlNode *root, *node;

    wxFileInputStream f_in(sFName);
    if (!f_in.IsOk()) return false;

    // Try to detect if the file is compressed or not
    char szBuff[iStdXmlHeaderSize+2];
    f_in.Read(szBuff, iStdXmlHeaderSize);
    szBuff[iStdXmlHeaderSize]=0;
    wxString sHeader(szBuff);
    f_in.SeekI(0, wxFromStart);

    if (sHeader==szStdXmlFileHeader)
    {
        // Non-compressed file
        if (!doc.Load(f_in)) return false;
    }
    else
    {
        // Compressed file
        wxZlibInputStream z_in(f_in);
        if (!doc.Load(z_in)) return false;
    }

    root=doc.GetRoot();
    node=root->GetChildren();
    wxString nodName, subName, sValue;
    long lVal;
    while(node)
    {
        nodName=node->GetName();
        if (nodName==_T("StartupPos")) // Last known position of the main window
        {
            // Window's position at application startup
            m_iStartPos=StartupString2Pos(node->GetAttribute(_T("Value"), _T("CenterScreen")));
            if (m_iStartPos==wxALIGN_NOT)
            {
                node->GetAttribute(_T("X"), _T("-1")).ToLong(&lVal);
                m_ptStartPos.x=lVal;
                node->GetAttribute(_T("Y"), _T("-1")).ToLong(&lVal);
                m_ptStartPos.y=lVal;
                node->GetAttribute(_T("W"), _T("-1")).ToLong(&lVal);
                m_szStartSize.SetWidth(lVal);
                node->GetAttribute(_T("H"), _T("-1")).ToLong(&lVal);
                m_szStartSize.SetHeight(lVal);
            }
        }
        if (nodName==_T("SplashScreen"))
        {
            // Show the splash screen at startup
            m_bShowSplashScreen=(node->GetAttribute(_T("Show"), _T("Yes"))==_T("Yes"));
        }
        if (nodName==_T("MultiInstances"))
        {
            // Only one instance allowed ?
            m_bSingleInstance=(node->GetAttribute(_T("Allowed"), _T("No"))==_T("No"));
        }
        if (nodName==_T("Password"))
        {
            m_sPassword=node->GetAttribute(_T("Value"), wxEmptyString);
        }
        if (nodName==_T("Compress"))
        {
            m_bCompDatas=(node->GetAttribute(_T("Datas"), _T("No"))==_T("Yes"));
            m_bCompSettings=(node->GetAttribute(_T("Settings"), _T("No"))==_T("Yes"));
        }

    	node = node->GetNext();
    }

    m_bModified=false;
    return true;
}

bool SettingsManager::SaveSettings()
{
    if (!m_bInitialized) Initialize();

    wxString sVal;

    wxString sFName=m_sDatasPath + _T("Settings.gvw");

    wxFileName fname(sFName);

    //Check if the folder exists
    if (!fname.DirExists())
    {
        // Try to create the folder
        if (!fname.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL))
            return false;
    }

    if (fname.FileExists())
    {
        if (!wxRemoveFile(fname.GetFullPath()))
            return false;
    }

    wxXmlNode *root = new wxXmlNode(NULL,wxXML_ELEMENT_NODE, _T("GedView_Settings-file"), wxEmptyString,
                            new wxXmlAttribute(_T("Version"), _T("1.0")));

    wxXmlNode *node;
    // Last known position and size of the main window
    node=new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T("StartupPos"));
    root->AddChild(node);
    node->AddAttribute(_T("Value"), StartupPos2String(m_iStartPos));
    if (m_iStartPos==wxALIGN_NOT)
    {
        node->AddAttribute(_T("X"), wxString::Format(_T("%d"), m_ptStartPos.x));
        node->AddAttribute(_T("Y"), wxString::Format(_T("%d"), m_ptStartPos.y));
        node->AddAttribute(_T("W"), wxString::Format(_T("%d"), m_szStartSize.GetWidth()));
        node->AddAttribute(_T("H"), wxString::Format(_T("%d"), m_szStartSize.GetHeight()));
    }
    // General password
    if (!m_sPassword.IsEmpty())
    {
        node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T("Password")));
        node = node->GetNext();
        node->AddAttribute(_T("Value"), m_sPassword);
    }
    // Datas and settings compression
    if (m_bCompDatas || m_bCompSettings)
    {
        node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T("Compress")));
        node = node->GetNext();
        node->AddAttribute(_T("Datas"), (m_bCompDatas?_T("Yes"):_T("No")));
        node->AddAttribute(_T("Settings"), (m_bCompSettings?_T("Yes"):_T("No")));
    }
    // Show (or not) the splash screen at startup
    node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T("SplashScreen")));
    node = node->GetNext();
    node->AddAttribute(_T("Show"), (m_bShowSplashScreen?_T("Yes"):_T("No")));
    // Allowing (or not) multiple instances of the application
    node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T("MultiInstances")));
    node = node->GetNext();
    node->AddAttribute(_T("Allowed"), (m_bSingleInstance?_T("No"):_T("Yes")));

    wxXmlDocument doc;
    doc.SetRoot(root);

    wxFileOutputStream f_out(fname.GetFullPath());
    m_bModified=false;
    if (GetCompressSettings())
    {
        // Write the xml document through a compression stream
        wxZlibOutputStream z_out(f_out, 9);
        return doc.Save(z_out);
    }
    else
    {
        // Write the xml document
        return doc.Save(f_out);
    }
}

const wxString SettingsManager::StartupPos2String(int iValue)
{
    // For optimisation : we first test the default value
    if (iValue==wxCENTER_ON_SCREEN) return _T("CenterScreen");
    // Special value
    if (iValue==wxALIGN_NOT) return _T("LastKnownPos");

    wxString sHPos=wxEmptyString, sVPos=wxEmptyString;

    sHPos=(((iValue&wxLEFT)==wxLEFT)? _T("Left"):((iValue&wxRIGHT)==wxRIGHT)?_T("Right"):_T("Center"));
    sVPos=(((iValue&wxTOP)==wxTOP)? _T("Top"):((iValue&wxBOTTOM)==wxBOTTOM)?_T("Bottom"):_T("Center"));

    if ((sHPos==_T("Center")) && (sVPos==_T("Center"))) return _T("CenterScreen");

    return sVPos + sHPos;
}

int SettingsManager::StartupString2Pos(const wxString& sValue)
{
    // For optimisation : we first test the default value
    if (sValue==_T("CenterScreen")) return wxCENTER_ON_SCREEN;
    // Special value
    if (sValue==_T("LastKnownPos")) return wxALIGN_NOT;
    int iHPos=0, iVPos=0;

    iHPos=((sValue.Find(_T("Left"))!=wxNOT_FOUND)?wxLEFT:((sValue.Find(_T("Right"))!=wxNOT_FOUND)?wxRIGHT:(wxDirection)wxCENTER));
    iVPos=((sValue.Find(_T("Top"))!=wxNOT_FOUND)?wxTOP:((sValue.Find(_T("Bottom"))!=wxNOT_FOUND)?wxBOTTOM:(wxDirection)wxCENTER));
    if ((iHPos==wxCENTER)&&(iVPos==wxCENTER)) return wxCENTER_ON_SCREEN;

    return (iHPos | iVPos);
}

bool SettingsManager::IsModified()
{
    return m_bModified;
}

void SettingsManager::SetMainWndStartupPos(int value)
{
    // We first check that the given value is correct
    int iVal=0;
    if ((value==wxCENTER_ON_SCREEN)||(value==wxALIGN_NOT))
    {
        iVal=value;
    }
    else
    {
        int iHPos=(((value&wxLEFT)==wxLEFT)? wxLEFT:((value&wxRIGHT)==wxRIGHT)?wxRIGHT:(wxDirection)wxCENTER);
        int iVPos=(((value&wxTOP)==wxTOP)? wxTOP:((value&wxBOTTOM)==wxBOTTOM)?wxBOTTOM:(wxDirection)wxCENTER);
        if ((iHPos==wxCENTER)&&(iVPos==wxCENTER))
            iVal=wxCENTER_ON_SCREEN;
        else
            iVal=(iHPos | iVPos);
    }
    if (iVal!=m_iStartPos)
    {
        m_iStartPos=iVal;
        m_bModified=true;
    }
}

void SettingsManager::SetLastWindowRect(const wxPoint& pos, const wxSize& size)
{
    if ((pos!=m_ptStartPos)||(size!=m_szStartSize))
    {
        m_bModified=true;
        m_ptStartPos=pos;
        m_szStartSize=size;
    }
}

void SettingsManager::SetShowSplashScreen(bool show)
{
    if (show!=m_bShowSplashScreen)
    {
        m_bShowSplashScreen=show;
        m_bModified=true;
    }
}

void SettingsManager::SetMultipleInstancesAllowed(bool value)
{
    // Inverted test : it is a normal thing ;)
    if (value==m_bSingleInstance)
    {
        m_bSingleInstance=(value==false);
        m_bModified=true;
    }
}

void SettingsManager::SetPassword(const wxString& pwd)
{
    if (pwd!=m_sPassword)
    {
        m_sPassword=pwd;
        m_bModified=true;
    }
}

void SettingsManager::SetCompressSettings(bool value)
{
    if (value!=m_bCompSettings)
    {
        m_bCompSettings=value;
        m_bModified=true;
    }
}

void SettingsManager::SetCompressDatas(bool value)
{
    if (value!=m_bCompDatas)
    {
        m_bCompDatas=value;
        m_bModified=true;
    }
}