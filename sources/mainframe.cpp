#include "mainframe.h"

#include "main.h"
#include "dlgabout.h"
#include "toolbaricons.h"
#include "datasmanager.h"
#include "settingsmanager.h"

#include <wx/display.h>
#include <wx/filedlg.h>

#ifndef __WXMSW__
#include "../graphx/wxwin32x32.xpm"
#endif // __WXMSW__

enum
{
    LST_COL_SEX,
    LST_COL_ID,
    LST_COL_LASTNAME,
    LST_COL_FIRSTNAME,

    LST_COL_COUNT
};

MainFrame::MainFrame(const wxString& title) : wxFrame(NULL, -1, title),
    m_settings(SettingsManager::Get()), m_datas(DatasManager::Get())
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Creating a \"MainFrame\" object\n"));
#endif // DEBUG

    SetIcon(wxICON(appIcon)); // Defining app icon

    CreateControls();

    UpdateSummary();

    int iStartPos=m_settings.GetMainWndStartupPos();
    if (iStartPos==wxALIGN_NOT)
    {
        wxPoint pt;
        m_settings.GetMainWndStartupPos(pt);
        wxSize sz=m_settings.GetMainWndStartupSize();

        if (sz==wxDefaultSize)
        {
            if (pt==wxDefaultPosition)
                Maximize(true);
            else
                CenterOnScreen();
        }
        else
        {
            Move(pt);
            SetSize(sz);
        }
    }
    else
    {

        wxDisplay d;
        wxRect rcD=d.GetClientArea();
        int iWScr=rcD.GetWidth();
        int iHScr=rcD.GetHeight();
        wxSize sz=GetSize();
        wxPoint pt=wxDefaultPosition;
        pt.x=(((iStartPos&wxLEFT)==wxLEFT)?0:((iStartPos&wxRIGHT)==wxRIGHT)?iWScr-sz.GetWidth():(iWScr-sz.GetWidth())/2);
        pt.y=(((iStartPos&wxTOP)==wxTOP)?0:((iStartPos&wxBOTTOM)==wxBOTTOM)?iHScr-sz.GetHeight():(iHScr-sz.GetHeight())/2);
        Move(pt);
    }

    ConnectControls();
}

MainFrame::~MainFrame()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Destroying a \"MainFrame\" object\n"));
#endif // DEBUG
}

void MainFrame::CreateControls()
{
    // Status bar
    CreateStatusBar(2);
    wxString sTxt=wxGetApp().GetBuildInfos();
    SetStatusText(sTxt, 1);
    // Toolbar
    wxToolBar* tb=CreateToolBar();

        tb->AddTool(wxID_OPEN, wxGetStockLabel(wxID_OPEN), wxGet_open_png_Bitmap(), _("Open a ged file"));

        tb->AddTool(wxID_SAVE, wxGetStockLabel(wxID_SAVE), wxGet_save_png_Bitmap(), _("Save datas to xml file"));

        tb->AddStretchableSpace();

        tb->AddTool(wxID_PREFERENCES, wxGetStockLabel(wxID_PREFERENCES), wxGet_preferences_png_Bitmap(), _("Edit application settings"));

        tb->AddTool(wxID_ABOUT, wxGetStockLabel(wxID_ABOUT), wxGet_about_png_Bitmap(), wxGetStockHelpString(wxID_ABOUT, wxSTOCK_MENU));

    tb->Realize();

    // Controls
    wxPanel *pnl=new wxPanel(this, -1);
    wxBoxSizer *szrMain=new wxBoxSizer(wxVERTICAL);

        wxStaticBoxSizer *stbszr=new wxStaticBoxSizer(wxVERTICAL, pnl, _("Datas summary:"));

            stbszr->AddSpacer(1);

            m_lblSummary=new wxStaticText(pnl, -1, _T(" \n "), wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE);
                wxFont fnt=m_lblSummary->GetFont();
                fnt.MakeBold();
                m_lblSummary->SetFont(fnt);

        stbszr->Add(m_lblSummary, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 5);

    szrMain->Add(stbszr, 0, wxALL|wxEXPAND, 5);

    wxBoxSizer *lnszr=new wxBoxSizer(wxHORIZONTAL);

        m_lstItems=new wxListView(pnl, -1);
            m_lstItems->AppendColumn(_("Sex"), wxLIST_FORMAT_LEFT, 30);
            m_lstItems->AppendColumn(_("ID"), wxLIST_FORMAT_RIGHT);
            m_lstItems->AppendColumn(_("Last Name"));
            m_lstItems->AppendColumn(_("First Name"));
        lnszr->Add(m_lstItems, 0, wxALL|wxEXPAND, 0);

    szrMain->Add(lnszr, 1, wxALL|wxEXPAND, 5);

    pnl->SetSizer(szrMain);
}

void MainFrame::ConnectControls()
{
    // General events handlers
    Connect(wxEVT_SIZE, wxSizeEventHandler(MainFrame::OnSize));
    Connect(wxEVT_MOVE, wxMoveEventHandler(MainFrame::OnMove));
    Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(MainFrame::OnClose));
    // Menus/Toolbar items
    Connect(wxID_OPEN, wxEVT_TOOL, wxCommandEventHandler(MainFrame::OnOpenGedFileClicked));
    Connect(wxID_SAVE, wxEVT_TOOL, wxCommandEventHandler(MainFrame::OnSaveXmlFileClicked));
    Connect(wxID_PREFERENCES, wxEVT_TOOL, wxCommandEventHandler(MainFrame::OnPreferencesClicked));
    Connect(wxID_ABOUT, wxEVT_TOOL, wxCommandEventHandler(MainFrame::OnAboutClicked));
    // UpdateUI events handlers
    Connect(wxID_SAVE, wxEVT_UPDATE_UI, wxUpdateUIEventHandler(MainFrame::OnUpdateUI_Save));
}

void MainFrame::UpdateSummary()
{
    wxString sTxt=_("Gedcom file:");
    if (m_datas.HasDatas())
    {
        sTxt << _T(" ") << m_datas.GetCurrentFileName() << _T("\n");
        sTxt << wxString::Format(_("Total items: %d - Individuals: %d - Family events: %d"), m_datas.GetItemsCount(), m_datas.GetItemsCount(GIT_INDI), m_datas.GetItemsCount(GIT_FAM));
    }
    else
    {
        sTxt << _T(" ") << _("No file loaded") << _T("\n") << _("No datas present in memory");
    }

    m_lblSummary->SetLabel(sTxt);
}

void MainFrame::UpdateList()
{
    m_lstItems->DeleteAllItems();

    if (!m_datas.HasDatas()) return;

    wxXmlNode *node=m_datas.GetDatas()->GetChildren(), *subnode;
    wxString sType, sTxt, sName;
    long lItem=0;

    while(node!=NULL)
    {
        sType=node->GetAttribute(_T("Type"));
        if (sType==_T("INDI"))
        {
            sTxt=node->GetAttribute(_T("GedId"));

            lItem=m_lstItems->InsertItem(lItem+1, _T(""));
            m_lstItems->SetItem(lItem, LST_COL_ID, sTxt);
            m_lstItems->SetItemPtrData(lItem, (wxUIntPtr)node);

            UpdateListItem(lItem);
        }

        node=node->GetNext();
    }
}

void MainFrame::UpdateListItem(long item)
{
    wxXmlNode *node=(wxXmlNode*)m_lstItems->GetItemData(item);
    if (node==NULL) return;

    wxXmlNode *subNode=node->GetChildren();

    while(subNode)
    {
        wxString sType=subNode->GetAttribute(_T("Type"));

        if (sType==_T("NAME"))
        {
            wxString sName=subNode->GetAttribute(_T("Value"));
            int iPos=sName.Find(_T('/'));
            if (iPos!=wxNOT_FOUND)
            {
                wxString sFirstName=sName.Left(iPos);
                wxString sLastName=sName.Mid(iPos+1);
                if (sLastName.EndsWith(_T("/")))
                    sLastName.RemoveLast(1);
                m_lstItems->SetItem(item, LST_COL_LASTNAME, sLastName);
                m_lstItems->SetItem(item, LST_COL_FIRSTNAME, sFirstName);
            }
            else
            {
                m_lstItems->SetItem(item, LST_COL_LASTNAME, wxEmptyString);
                m_lstItems->SetItem(item, LST_COL_FIRSTNAME, sName);
            }
        }
        if (sType==_T("SEX"))
        {
            m_lstItems->SetItem(item, LST_COL_SEX, subNode->GetAttribute(_T("Value")));
        }

        subNode=subNode->GetNext();
    }
}

void MainFrame::OnSize(wxSizeEvent& event)
{
    if (!IsShown()) return;
    if (IsMaximized())
    {
        m_settings.SetLastWindowRect(wxDefaultPosition, wxDefaultSize);
    }
    else
    {
        m_settings.SetLastWindowRect(GetPosition(), GetSize());
    }
    event.Skip();
}

void MainFrame::OnMove(wxMoveEvent& event)
{
    if (!IsShown()) return;
    m_settings.SetLastWindowRect(GetPosition(), GetSize());
    event.Skip();
}

void MainFrame::OnClose(wxCloseEvent& event)
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Closing the \"MainFrame\" object\n"));
#endif // DEBUG
    Destroy();
}

void MainFrame::OnOpenGedFileClicked(wxCommandEvent& event)
{
    wxString sMsg=_("Select the \"GED\" file to open");
    wxString sWlcrd=_("Gedcom files (*.ged)|*.ged|All files (*.*)|*.*");
    wxString sFName=wxFileSelector(sMsg, wxGetCwd(), _T("base.ged"), _T("ged"), sWlcrd, wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (sFName.IsEmpty()) return;
    if (!m_datas.ReadGedFile(sFName))
    {
        wxMessageBox(_("An error occurred while reading the ged file !"), _("Error"), wxICON_EXCLAMATION|wxCENTER|wxOK);
        return;
    }

    UpdateSummary();

    UpdateList();

    wxMessageBox(_("Ged file read successfully !"), _("Done"), wxICON_EXCLAMATION|wxCENTER|wxOK);
}

void MainFrame::OnSaveXmlFileClicked(wxCommandEvent& event)
{
    if (!m_datas.HasDatas()) return;

    wxString sMsg=_("Select the \"XML\" file to create");
    wxString sWlcrd=_("Xml files (*.xml)|*.xml|All files (*.*)|*.*");
    wxString sFName=wxFileSelector(sMsg, wxGetCwd(), _T("base.xml"), _T("xml"), sWlcrd, wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (sFName.IsEmpty()) return;

    if (!m_datas.SaveXmlFile(sFName))
    {
        wxMessageBox(_("An error occurred while writing the xml file !"), _("Error"), wxICON_EXCLAMATION|wxCENTER|wxOK);
        return;
    }
    wxMessageBox(_("Datas successfully saved to") + _T("\n") + sFName, _("Done"), wxICON_EXCLAMATION|wxCENTER|wxOK);
}

void MainFrame::OnPreferencesClicked(wxCommandEvent& event)
{
    wxMessageBox(_("Sorry, but this function isn't implemented yet !"), _("Preferences"), wxICON_EXCLAMATION|wxCENTER|wxOK);
}

void MainFrame::OnAboutClicked(wxCommandEvent& event)
{
    DlgAbout dlg(this);
    dlg.ShowModal();
}

void MainFrame::OnUpdateUI_Save(wxUpdateUIEvent& event)
{
    event.Enable(m_datas.HasDatas());
}
