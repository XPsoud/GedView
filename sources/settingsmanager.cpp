#include "settingsmanager.h"

const wxChar* szStdXmlFileHeader=_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
const int iStdXmlHeaderSize=38;

const wxChar* SettingsManager::m_szSettingsFName=_T("settings.xml");
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
    m_locale=NULL;
}

SettingsManager::~SettingsManager()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Destroying a \"SettingsManager\" object\n"));
#endif // __WXDEBUG__
    wxDELETE(m_locale);
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
#endif // __WXDEBUG__
    // Full path of the application
    m_sAppPath = wxPathOnly(wxStandardPaths::Get().GetExecutablePath());
    if (!m_sAppPath.EndsWith(wxFileName::GetPathSeparator()))
        m_sAppPath.Append(wxFileName::GetPathSeparator());
#ifndef __WXMAC__
    // Windows and Linux : switch to portable mode if the
    // settings file is present in the application folder
    if (wxFileExists(m_sAppPath + m_szSettingsFName))
    {
        m_sSettingsPath=m_sAppPath;
        m_sLngPath=m_sAppPath;
    }
    else
    {
        if (wxDirExists(m_sAppPath + _T("langs")))
        {
            m_sLngPath=m_sAppPath;
        }
        else
        {
            m_sLngPath=wxStandardPaths::Get().GetResourcesDir();
        }
#endif // ndef __WXMAC__
        // Path for the settings file (platform dependant)
        m_sSettingsPath=wxStandardPaths::Get().GetUserDataDir();
#ifndef __WXMAC__
    }
#endif // ndef __WXMAC__
    if (!m_sSettingsPath.EndsWith(wxFileName::GetPathSeparator()))
        m_sSettingsPath.Append(wxFileName::GetPathSeparator());
    if (!m_sLngPath.EndsWith(wxFileName::GetPathSeparator()))
        m_sLngPath.Append(wxFileName::GetPathSeparator());
    m_sLngPath.Append(_T("langs"));
    // Default position and size of the main window
    m_iStartPos=wxCENTER_ON_SCREEN;
    m_ptStartPos=wxDefaultPosition;
    m_szStartSize=wxDefaultSize;
    m_iShashPos=MINIMUM_PANE_WIDTH;
    for (int i=0; i<LST_COL_COUNT; ++i)
        m_iColWdth[i]=wxID_ANY;
    // Other default settings
    m_bSingleInstance=true;
    m_bCompDatas=false;
    m_bCompSettings=false;
    m_sPassword=wxEmptyString;
    m_bProhibI18N=false;

    m_bInitialized=true;
}

bool SettingsManager::ReadSettings()
{
    if (!m_bInitialized) Initialize();

    // Rename the old settings file if it exists
    if (wxFileExists(m_sSettingsPath + _T("Settings.gvw")))
        wxRenameFile(m_sSettingsPath + _T("Settings.gvw"), m_sSettingsPath + _T("settings.xml"));

    wxString sFName=m_sSettingsPath + _T("settings.xml");

    if (!wxFileExists(sFName)) return false;

    wxXmlDocument doc;
    wxXmlNode *root, *node, *subNode;

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
                node->GetAttribute(_T("Split"), _T("-1")).ToLong(&lVal);
                m_iShashPos=(lVal==-1?MINIMUM_PANE_WIDTH:lVal);
            }
        }
        if (nodName==_T("ColumnsWidth"))
        {
            subNode=node->GetChildren();
            while(subNode!=NULL)
            {
                subName=subNode->GetName();
                if (subName==_T("Sex"))
                {
                    subNode->GetNodeContent().ToLong(&lVal);
                    m_iColWdth[LST_COL_SEX]=lVal;
                }
                if (subName==_T("ID"))
                {
                    subNode->GetNodeContent().ToLong(&lVal);
                    m_iColWdth[LST_COL_ID]=lVal;
                }
                if (subName==_T("LastName"))
                {
                    subNode->GetNodeContent().ToLong(&lVal);
                    m_iColWdth[LST_COL_LASTNAME]=lVal;
                }
                if (subName==_T("FirstName"))
                {
                    subNode->GetNodeContent().ToLong(&lVal);
                    m_iColWdth[LST_COL_FIRSTNAME]=lVal;
                }
                subNode=subNode->GetNext();
            }
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
        if (nodName==_T("Translation"))
		{
			// Allowed ?
			m_bProhibI18N=(node->GetAttribute(_T("Allowed"), _T("Yes"))!=_T("Yes"));
		}
		if (nodName==_T("RecentsFiles"))
        {
            m_recents.FromXmlNode(node);
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

    wxString sFName=m_sSettingsPath + _T("settings.xml");

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

    wxXmlNode *node, *subNode;
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
        node->AddAttribute(_T("Split"), wxString::Format(_T("%d"), m_iShashPos));
    }
    // Columns width
    node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T("ColumnsWidth")));
    node = node->GetNext();
        subNode=new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T("Sex"));
        node->AddChild(subNode);
        subNode->AddChild(new wxXmlNode(wxXML_TEXT_NODE, _T(""), wxString::Format(_T("%d"), m_iColWdth[LST_COL_SEX])));

        subNode->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T("ID")));
        subNode = subNode->GetNext();
        subNode->AddChild(new wxXmlNode(wxXML_TEXT_NODE, _T(""), wxString::Format(_T("%d"), m_iColWdth[LST_COL_ID])));

        subNode->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T("LastName")));
        subNode = subNode->GetNext();
        subNode->AddChild(new wxXmlNode(wxXML_TEXT_NODE, _T(""), wxString::Format(_T("%d"), m_iColWdth[LST_COL_LASTNAME])));

        subNode->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T("FirstName")));
        subNode = subNode->GetNext();
        subNode->AddChild(new wxXmlNode(wxXML_TEXT_NODE, _T(""), wxString::Format(_T("%d"), m_iColWdth[LST_COL_FIRSTNAME])));
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
    // Allowing (or not) multiple instances of the application
    node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T("MultiInstances")));
    node = node->GetNext();
    node->AddAttribute(_T("Allowed"), (m_bSingleInstance?_T("No"):_T("Yes")));
    // Allowing (or not) interface translation
	node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T("Translation")));
	node = node->GetNext();
	node->AddAttribute(_T("Allowed"), (m_bProhibI18N?_T("No"):_T("Yes")));
	// Recents setups list (only saved if not empty)
    if (!m_recents.IsEmpty())
    {
        node->SetNext(new wxXmlNode(NULL, wxXML_ELEMENT_NODE, _T("RecentsFiles")));
        node = node->GetNext();
        m_recents.ToXmlNode(node);
    }

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
    m_bModified |= m_recents.IsModified();

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

void SettingsManager::SetLastSashPos(int pos)
{
    if (pos!=m_iShashPos)
    {
        m_bModified=true;
        m_iShashPos=pos;
    }
}

int SettingsManager::GetColumnWidth(int col)
{
    if ((col<0)||(col>=LST_COL_COUNT))
        return wxID_ANY;
    return m_iColWdth[col];
}

void SettingsManager::SetColumnWidth(int col, int width)
{
    if ((col<0)||(col>=LST_COL_COUNT))
        return;
    if (width!=m_iColWdth[col])
    {
        m_iColWdth[col]=width;
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

void SettingsManager::SetProhibitI18N(bool value)
{
	if (value!=m_bProhibI18N)
	{
		m_bProhibI18N=value;
		m_bModified=true;
	}
}

void SettingsManager::SetLocale()
{
    if (m_locale!=NULL)
    {
        wxDELETE(m_locale);
    }
    bool bI18N=(m_bProhibI18N==false);
    // Easter egg for debugging purpose
    if (wxGetKeyState(WXK_SHIFT))
        bI18N=!bI18N;
    if (bI18N)
    {
        wxString sDir=m_sLngPath;
        int lng=wxLocale::GetSystemLanguage();
        if (lng!=wxLANGUAGE_ENGLISH)
        {
            m_locale=new wxLocale();
            m_locale->AddCatalogLookupPathPrefix(m_sLngPath);
            wxLogNull logNo; // Avoid displaying error msgbox if language isn't supported
            if (m_locale->Init(lng, wxLOCALE_LOAD_DEFAULT))
            {
                m_locale->AddCatalog(_T("GedView"));
#ifdef __WXGTK__
        	    m_locale->AddCatalog(_T("wxStd")); // Don't know yet why it isn't loaded automatically :-/
#endif // __WXGTK__
            }
            else
            {
                wxDELETE(m_locale);
            }
        }
    }
}
