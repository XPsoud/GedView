#include "mainframe.h"

#include "main.h"
#include "dlgabout.h"
#include "appversion.h"
#include "dlgtreepdf.h"
#include "dlgoptions.h"
#include "dlgexportpdf.h"
#include "dlgexportcsv.h"
#include "toolbaricons.h"
#include "datasmanager.h"
#include "settingsmanager.h"

#include <wx/display.h>
#include <wx/filedlg.h>
#include <wx/artprov.h>
#include <wx/xml/xml.h>
#include <wx/wfstream.h>

#ifndef __WXMSW__
#include "../graphx/wxwin32x32.xpm"
#endif // __WXMSW__

const wxEventType wxEVT_FILEOPEN = wxNewEventType();

enum SortColumn
{
    SORT_COL_UNKNOWN =0,
    SORTCOL_SEX,
    SORTCOL_ID,
    SORTCOL_LNAME,
    SORTCOL_FNAME
};

const int wxID_PDFLIST = wxNewId();
const int wxID_PDFTREE = wxNewId();
const int wxID_CSVFILE = wxNewId();

MainFrame::MainFrame(const wxString& title) : wxFrame(NULL, -1, title),
    m_settings(SettingsManager::Get()), m_datas(DatasManager::Get())
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Creating a \"MainFrame\" object\n"));
#endif // DEBUG

    SetIcon(wxICON(appIcon)); // Defining app icon

    m_arsHistory.Clear();
    m_iHistPos=-1;
    m_iSortCol=SORTCOL_ID;
    m_bHistClicked=false;

    CreateControls();

    UpdateSummary();
    UpdateItemDetails();

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
    m_spwSplitter->SetSashPosition(m_settings.GetLastSashPos());

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
    wxToolBar* tb=CreateToolBar(wxTB_FLAT);

        tb->AddTool(wxID_OPEN, wxGetStockLabel(wxID_OPEN), wxGet_open_png_Bitmap(), _("Open a ged file"));

        tb->AddTool(wxID_SAVE, wxGetStockLabel(wxID_SAVE), wxGet_save_png_Bitmap(), _("Save datas to xml file"));

        tb->AddTool(wxID_PDFLIST, _("Export as pdf file"), wxGet_pdf_png_Bitmap(), _("Export the datas list to a pdf file"));

        tb->AddTool(wxID_PDFTREE, _("Create pdf tree"), wxGet_pdftree_png_Bitmap(), _("Export selected item's tree to a pdf file"));

        tb->AddTool(wxID_CSVFILE, _("Export to csv"), wxGet_csv_png_Bitmap(), _("Export datas to a csv file"));

        tb->AddSeparator();

        tb->AddTool(wxID_BACKWARD, wxGetStockLabel(wxID_BACKWARD), wxArtProvider::GetBitmap(wxART_GO_BACK, wxART_TOOLBAR), _("Go backward in shown items history"));

        tb->AddTool(wxID_FORWARD, wxGetStockLabel(wxID_FORWARD), wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_TOOLBAR), _("Go forward in shown items history"));

        tb->AddSeparator();

        tb->AddTool(wxID_SPELL_CHECK, _("Compare"), wxGet_compare_png_Bitmap(), _("Compare datas with an other ged file"));

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

    m_spwSplitter=new wxSplitterWindow(pnl, -1);

        m_lstItems=new wxListView(m_spwSplitter, -1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
            int iWdth=m_settings.GetColumnWidth(LST_COL_SEX);
            if (iWdth<30) iWdth=30;
            m_lstItems->AppendColumn(_("Sex"), wxLIST_FORMAT_LEFT, iWdth);
            iWdth=m_settings.GetColumnWidth(LST_COL_ID);
            if (iWdth<60) iWdth=60;
            m_lstItems->AppendColumn(_("ID"), wxLIST_FORMAT_RIGHT, iWdth);
            iWdth=m_settings.GetColumnWidth(LST_COL_LASTNAME);
            m_lstItems->AppendColumn(_("Last Name"), wxLIST_FORMAT_LEFT, iWdth);
            iWdth=m_settings.GetColumnWidth(LST_COL_FIRSTNAME);
            m_lstItems->AppendColumn(_("First Name"), wxLIST_FORMAT_LEFT, iWdth);

        m_htwDetails=new wxHtmlWindow(m_spwSplitter, -1);

    szrMain->Add(m_spwSplitter, 1, wxALL|wxEXPAND, 5);

    m_spwSplitter->SplitVertically(m_lstItems, m_htwDetails);
    m_spwSplitter->SetMinimumPaneSize(MINIMUM_PANE_WIDTH);

    pnl->SetSizer(szrMain);

    // Timer to check for selected item
    m_tmrLstSel.SetOwner(this, wxID_ANY);
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
    Connect(wxID_PDFLIST, wxEVT_TOOL, wxCommandEventHandler(MainFrame::OnSavePdfFileClicked));
    Connect(wxID_PDFTREE, wxEVT_TOOL, wxCommandEventHandler(MainFrame::OnSavePdfTreeClicked));
    Connect(wxID_CSVFILE, wxEVT_TOOL, wxCommandEventHandler(MainFrame::OnSaveCsvFileClicked));
    Connect(wxID_BACKWARD, wxEVT_TOOL, wxCommandEventHandler(MainFrame::OnHistoryBackClicked));
    Connect(wxID_FORWARD, wxEVT_TOOL, wxCommandEventHandler(MainFrame::OnHistoryNextClicked));
    Connect(wxID_SPELL_CHECK, wxEVT_TOOL, wxCommandEventHandler(MainFrame::OnCompareClicked));
    Connect(wxID_PREFERENCES, wxEVT_TOOL, wxCommandEventHandler(MainFrame::OnPreferencesClicked));
    Connect(wxID_ABOUT, wxEVT_TOOL, wxCommandEventHandler(MainFrame::OnAboutClicked));
    // Other controls events handlers
    m_lstItems->Connect(wxEVT_LIST_ITEM_SELECTED, wxListEventHandler(MainFrame::OnListItemSelected), NULL, this);
    m_lstItems->Connect(wxEVT_LIST_ITEM_DESELECTED, wxListEventHandler(MainFrame::OnListItemDeselected), NULL, this);
    m_lstItems->Connect(wxEVT_LIST_COL_CLICK, wxListEventHandler(MainFrame::OnColumnHeaderClicked), NULL, this);
    m_lstItems->Connect(wxEVT_LIST_COL_END_DRAG, wxListEventHandler(MainFrame::OnColumnHeaderDragged), NULL, this);
    m_htwDetails->Connect(wxEVT_HTML_LINK_CLICKED, wxHtmlLinkEventHandler(MainFrame::OnHtmlLinkClicked), NULL, this);
    m_spwSplitter->Connect(wxEVT_SPLITTER_SASH_POS_CHANGED, wxSplitterEventHandler(MainFrame::OnShashPosChanged), NULL, this);
    // Custom events handlers
    Connect(wxEVT_FILEOPEN, wxCommandEventHandler(MainFrame::OnAutoOpenGedFile));
    // UpdateUI events handlers
    Connect(wxID_SAVE, wxEVT_UPDATE_UI, wxUpdateUIEventHandler(MainFrame::OnUpdateUI_Save));
    Connect(wxID_PDFLIST, wxEVT_UPDATE_UI, wxUpdateUIEventHandler(MainFrame::OnUpdateUI_Save));
    Connect(wxID_PDFTREE, wxEVT_UPDATE_UI, wxUpdateUIEventHandler(MainFrame::OnUpdateUI_PdfTree));
    Connect(wxID_CSVFILE, wxEVT_UPDATE_UI, wxUpdateUIEventHandler(MainFrame::OnUpdateUI_Save));
    Connect(wxID_BACKWARD, wxEVT_UPDATE_UI, wxUpdateUIEventHandler(MainFrame::OnUpdateUI_Backward));
    Connect(wxID_FORWARD, wxEVT_UPDATE_UI, wxUpdateUIEventHandler(MainFrame::OnUpdateUI_Forward));
    Connect(wxID_SPELL_CHECK, wxEVT_UPDATE_UI, wxUpdateUIEventHandler(MainFrame::OnUpdateUI_Compare));
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

    wxXmlNode *node=m_datas.GetDatas()->GetChildren();
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

void MainFrame::UpdateItemDetails()
{
    long lItem=m_lstItems->GetFirstSelected();
    if (lItem==wxNOT_FOUND)
    {
        wxString sPage=_T("<html><body>");
        sPage << _T("<center><h1>") << _T(PRODUCTNAME) << _T(" (v") << VERSION_MAJOR << _T(".") << VERSION_MINOR << _T(".") << VERSION_REV << _T(")");
        sPage << _T("</h1><h2>");
        sPage << wxGetApp().GetBuildInfos(false);
        sPage << _T("</h2></center></body></html>");
        m_htwDetails->SetPage(sPage);

        return;
    }
    // Clear the content of the wxHtmlWindow
    m_htwDetails->SetPage(_T("<html><body></body></html>"));

    wxXmlNode *node=(wxXmlNode*)m_lstItems->GetItemData(lItem);
    if (node==NULL) return;

    wxString sItmID=node->GetAttribute(_T("GedId"));
    if (sItmID.IsEmpty()) return;

    int iSex=m_datas.GetItemSex(node);
    wxChar c=(iSex==GIS_MALE?wxChar(0x2642):(iSex==GIS_FEMALE?wxChar(0x2640):wxChar(32)));

    wxString sPage=_T("<h3>") + m_datas.GetItemFullName(node).Prepend(c) + _T("</h3>");
    wxXmlNode *subNode=node->GetChildren();
    bool bUnions=false;
    while(subNode!=NULL)
    {
        wxString sType=subNode->GetAttribute(_T("Type"));
        if (subNode->GetName()==_T("Event"))
        {
            wxString sEvt=m_datas.ParseEvent(subNode);
            if ((sEvt==_("Dead"))&&(iSex==GIS_FEMALE))
            {
                sEvt=_("Dead_F");
            }
            sPage << _T("<br />") << sEvt;
        }

        if (sType==_T("FAMC"))
        {
            sPage << _T("<br /><hr /><h4>") << _("Parents") << _T("</h4>");
            wxArrayString arsSiblings;
            arsSiblings.Clear();
            wxXmlNode* evtNode=m_datas.FindItemByGedId(subNode->GetAttribute(_T("Value")));
            if (evtNode!=NULL)
            {
                wxXmlNode *subEvt=evtNode->GetChildren();
                wxString sSubTyp, sSubId, sEvent;
                while(subEvt!=NULL)
                {
                    sSubTyp=subEvt->GetAttribute(_T("Type"));
                    sSubId=subEvt->GetAttribute(_T("GedId"));
                    if ((sSubTyp==_T("HUSB"))||(sSubTyp==_T("WIFE")))
                    {
                        sPage << _T("<br />") << (sSubTyp==_T("HUSB")?_("Father:"):_("Mother:")) << _T(" <b><a href=\"") << sSubId << _T("\">") << m_datas.GetItemFullName(sSubId) << _T("</a></b>");
                        sEvent=m_datas.GetItemBirth(sSubId);
                        if (!sEvent.IsEmpty())
                            sPage << _T("<br /><small>&nbsp;&nbsp;&nbsp;") << sEvent << _T("</small>");
                        sEvent=m_datas.GetItemDeath(sSubId);
                        if (!sEvent.IsEmpty())
                        {
                            if ((sEvent==_("Dead"))&&(m_datas.GetItemSex(sSubId)==GIS_FEMALE))
                                sEvent=_("Dead_F");
                            sPage << _T("<br /><small>&nbsp;&nbsp;&nbsp;") << sEvent << _T("</small>");
                        }
                    }
                    if (sSubTyp==_T("CHIL"))
                    {
                        arsSiblings.Add(sSubId);
                    }
                    subEvt=subEvt->GetNext();
                }
                if (arsSiblings.GetCount()>1)
                {
                    sPage << _T("<br /><hr /><h4>") << _("Siblings") << _T("</h4>");
                    for (size_t s=0; s<arsSiblings.GetCount(); s++)
                    {
                        if (arsSiblings[s]!=sItmID)
                        {
                            sPage << _T("<br /> <b><a href=\"") << arsSiblings[s] << _T("\">")<< m_datas.GetItemFirstName(arsSiblings[s]) << _T("</a></b>");
                            sEvent=m_datas.GetItemBirth(arsSiblings[s]);
                            if (!sEvent.IsEmpty())
                                sPage << _T("<br /><small>&nbsp;&nbsp;&nbsp;") << sEvent << _T("</small>");
                            sEvent=m_datas.GetItemDeath(arsSiblings[s]);
                            if (!sEvent.IsEmpty())
                            {
                                if ((sEvent==_("Dead"))&&(m_datas.GetItemSex(arsSiblings[s])==GIS_FEMALE))
                                    sEvent=_("Dead_F");
                                sPage << _T("<br /><small>&nbsp;&nbsp;&nbsp;") << sEvent << _T("</small>");
                            }
                        }
                    }
                }
            }
        }
        if (sType==_T("FAMS"))
        {
            if (!bUnions)
            {
                sPage << _T("<br /><hr /><h4>") << _("Union(s)") << _T("</h4>");
                bUnions=true;
            }
            wxXmlNode* evtNode=m_datas.FindItemByGedId(subNode->GetAttribute(_T("Value")));
            wxString sEvent;
            if (evtNode!=NULL)
            {
                wxXmlNode *subEvt=evtNode->GetChildren();
                wxString sSubTyp;
                bool bChild=false;
                while(subEvt!=NULL)
                {
                    sSubTyp=subEvt->GetAttribute(_T("Type"));
                    wxString sEvtId=subEvt->GetAttribute(_T("GedId"));
                    if (subEvt->GetName()==_T("Event"))
                    {
                        wxString sTmp=m_datas.ParseEvent(subEvt);
                        if (!sTmp.IsEmpty())
                            sPage << _T("<br /><small>") << sTmp << _T("</small>");
                    }
                    if (((sSubTyp==_T("HUSB"))||(sSubTyp==_T("WIFE")))&&(!sEvtId.IsEmpty())&&(sEvtId!=sItmID))
                    {
                        sPage << _T("<br /> <b><a href=\"") << sEvtId << _T("\">") << m_datas.GetItemFullName(sEvtId) << _T("</a></b>");
                        sEvent=m_datas.GetItemBirth(sEvtId);
                        if (!sEvent.IsEmpty())
                            sPage << _T("<br /><small>&nbsp;&nbsp;&nbsp;") << sEvent << _T("</small>");
                        sEvent=m_datas.GetItemDeath(sEvtId);
                        if (!sEvent.IsEmpty())
                        {
                            if (sEvent==_("Dead") && (sSubTyp==_T("WIFE")))
                            {
                                sEvent=_("Dead_F");
                            }
                            sPage << _T("<br /><small>&nbsp;&nbsp;&nbsp;") << sEvent << _T("</small>");
                        }
                    }
                    if (sSubTyp==_T("CHIL"))
                    {
                        if (bChild==false)
                        {
                            sPage << _T("<ul>");
                            bChild=true;
                        }
                        sPage << _T("<li><a href=\"") << sEvtId << _T("\">") << m_datas.GetItemFirstName(sEvtId) << _T("</a>");
                        sEvent=m_datas.GetItemBirth(sEvtId);
                        if (!sEvent.IsEmpty())
                            sPage << _T("<br /><small>&nbsp;&nbsp;&nbsp;") << sEvent << _T("</small>");
                        sEvent=m_datas.GetItemDeath(sEvtId);
                        if (!sEvent.IsEmpty())
                        {
                            if ((sEvent==_("Dead")) && (m_datas.GetItemSex(sEvtId)==GIS_FEMALE))
                                sEvent=_("Dead_F");
                            sPage << _T("<br /><small>&nbsp;&nbsp;&nbsp;") << sEvent << _T("</small>");
                        }
                        sPage << _T("</li>");
                    }
                    subEvt=subEvt->GetNext();
                }
                if (bChild)
                {
                    sPage << _T("</ul>");
                    bChild=false;
                }
            }
        }
        subNode=subNode->GetNext();
    }

    sPage << _T("</body></html>");
    m_htwDetails->SetPage(_T("<html><body>") + sPage);
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
    m_arsHistory.Clear();
    m_iHistPos=-1;
    m_bHistClicked=false;

    UpdateSummary();

    UpdateList();

    wxMessageBox(_("Ged file read successfully !"), _("Done"), wxICON_EXCLAMATION|wxCENTER|wxOK);
}

void MainFrame::OnAutoOpenGedFile(wxCommandEvent& event)
{
    wxString sFName=event.GetString();
    if ((sFName.IsEmpty())||(!wxFileExists(sFName))) return;
    if (!m_datas.ReadGedFile(sFName))
    {
        wxMessageBox(_("An error occurred while reading the ged file !"), _("Error"), wxICON_EXCLAMATION|wxCENTER|wxOK);
        return;
    }
    wxSetWorkingDirectory(wxPathOnly(sFName));
    m_arsHistory.Clear();
    m_iHistPos=-1;
    m_bHistClicked=false;

    UpdateSummary();

    UpdateList();
}

void MainFrame::OnSaveXmlFileClicked(wxCommandEvent& event)
{
    if (!m_datas.HasDatas()) return;

    wxFileName fname=m_datas.GetCurrentFileName();
    fname.SetExt(_T("xml"));

    wxString sMsg=_("Select the \"XML\" file to create");
    wxString sWlcrd=_("Xml files (*.xml)|*.xml|All files (*.*)|*.*");
    wxString sFName=wxFileSelector(sMsg, fname.GetPath(), fname.GetFullName(), _T("xml"), sWlcrd, wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
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
    // Save the "KeepLang" state
    bool bOldLng=m_settings.GetProhibitI18N();
    DlgOptions dlg(this);
    dlg.ShowModal();
    if (m_settings.GetProhibitI18N()!=bOldLng)
    {
        wxMessageBox(_("You changed the translation settings.\nYou must restart the application to see this in effect."), _("Restart needed"), wxICON_INFORMATION|wxCENTER|wxOK);
    }
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

void MainFrame::OnUpdateUI_PdfTree(wxUpdateUIEvent& event)
{
    event.Enable(m_lstItems->GetSelectedItemCount()!=0);
}

void MainFrame::OnUpdateUI_Backward(wxUpdateUIEvent& event)
{
    int iCount=m_arsHistory.GetCount();
    event.Enable((m_iHistPos>0) && (iCount>=m_iHistPos));
}

void MainFrame::OnUpdateUI_Forward(wxUpdateUIEvent& event)
{
    int iCount=m_arsHistory.GetCount();
    event.Enable((m_iHistPos>-1) && (m_iHistPos<(iCount-1)));
}

void MainFrame::OnUpdateUI_Compare(wxUpdateUIEvent& event)
{
    event.Enable(m_datas.HasDatas());
}

void MainFrame::OnListItemSelected(wxListEvent& event)
{
    UpdateItemDetails();
    if (m_bHistClicked)
    {
        m_bHistClicked=false;
        return;
    }

    long lItem=m_lstItems->GetFirstSelected();
    if (lItem==wxNOT_FOUND) return;

    wxXmlNode *node=(wxXmlNode*)m_lstItems->GetItemData(lItem);
    if (node==NULL) return;

    wxString sItmID=node->GetAttribute(_T("GedId"));
    if (sItmID.IsEmpty()) return;

    int iCount=m_arsHistory.GetCount();

    if (m_iHistPos==(iCount-1))
    {
        m_arsHistory.Add(sItmID);
        m_iHistPos++;
        return;
    }
    m_iHistPos++;
    if (m_arsHistory[m_iHistPos]==sItmID) return;
    m_arsHistory.RemoveAt(m_iHistPos, iCount-m_iHistPos);
    m_arsHistory.Add(sItmID);
}

void MainFrame::OnListItemDeselected(wxListEvent& event)
{
    // Clear the content of the wxHtmlWindow
    m_htwDetails->SetPage(_T("<html><body></body></html>"));
    m_tmrLstSel.Start(250, true);
}

void MainFrame::OnColumnHeaderClicked(wxListEvent& event)
{
    int iColClicked=event.GetColumn();
    switch(iColClicked)
    {
        case LST_COL_SEX:
            if ((m_iSortCol==SORTCOL_SEX) ||(m_iSortCol==-SORTCOL_SEX))
                m_iSortCol*=-1;
            else
                m_iSortCol=SORTCOL_SEX;
            break;
        case LST_COL_ID:
            if ((m_iSortCol==SORTCOL_ID) ||(m_iSortCol==-SORTCOL_ID))
                m_iSortCol*=-1;
            else
                m_iSortCol=SORTCOL_ID;
            break;
        case LST_COL_LASTNAME:
            if ((m_iSortCol==SORTCOL_LNAME) ||(m_iSortCol==-SORTCOL_LNAME))
                m_iSortCol*=-1;
            else
                m_iSortCol=SORTCOL_LNAME;
            break;
        case LST_COL_FIRSTNAME:
            if ((m_iSortCol==SORTCOL_FNAME) ||(m_iSortCol==-SORTCOL_FNAME))
                m_iSortCol*=-1;
            else
                m_iSortCol=SORTCOL_FNAME;
            break;
        default:
            return;
    }
    m_lstItems->SortItems(MainFrame::SortCompFunction, m_iSortCol);
    // If an item is selected, be sure it is visible
    long lItem=m_lstItems->GetFirstSelected();
    if (lItem!=wxNOT_FOUND)
        m_lstItems->EnsureVisible(lItem);
}

void MainFrame::OnColumnHeaderDragged(wxListEvent& event)
{
    if (m_lstItems->GetItemCount()==0)
        return;
    int iCol=event.GetColumn();
    if (iCol==wxNOT_FOUND) return;
    int iWidth=m_lstItems->GetColumnWidth(iCol);
    m_settings.SetColumnWidth(iCol, iWidth);
}

void MainFrame::OnTimerSelectionCheck(wxTimerEvent& event)
{
    if (m_lstItems->GetFirstSelected()==wxNOT_FOUND)
        UpdateItemDetails();
}

void MainFrame::OnHtmlLinkClicked(wxHtmlLinkEvent& event)
{
    wxString sLink=event.GetLinkInfo().GetHref();
    if (sLink.IsEmpty()) return;
    int iCount=m_lstItems->GetItemCount();
    for (int i=0; i<iCount; ++i)
    {
        if (m_lstItems->GetItemText(i, 1)==sLink)
        {
            m_lstItems->Select(i, true);
            m_lstItems->EnsureVisible(i);
        }
    }
}

void MainFrame::OnSavePdfFileClicked(wxCommandEvent& event)
{
    DlgExportPdf dlg(this);

    long lItem=m_lstItems->GetFirstSelected();
    if (lItem!=wxNOT_FOUND)
    {
        wxXmlNode *node=(wxXmlNode*)m_lstItems->GetItemData(lItem);
        dlg.SetSelectedItem(node);
    }

    if (dlg.ShowModal()==wxID_OK)
        wxMessageBox(_("The pdf file creation terminated !"), _("Success"), wxICON_INFORMATION|wxOK|wxCENTER);
}

void MainFrame::OnSavePdfTreeClicked(wxCommandEvent& event)
{
    DlgTreePdf dlg(this);

    long lItem=m_lstItems->GetFirstSelected();
    if (lItem!=wxNOT_FOUND)
    {
        wxXmlNode *node=(wxXmlNode*)m_lstItems->GetItemData(lItem);
        dlg.SetSelectedItem(node);
    }

    dlg.ShowModal();
}

void MainFrame::OnSaveCsvFileClicked(wxCommandEvent& event)
{
    DlgExportCsv dlg(this);
    dlg.ShowModal();
}

void MainFrame::OnHistoryBackClicked(wxCommandEvent& event)
{
    if (m_iHistPos<1) return;
    m_iHistPos--;

    int iCount=m_lstItems->GetItemCount();
    for (int i=0; i<iCount; ++i)
    {
        if (m_lstItems->GetItemText(i, 1)==m_arsHistory[m_iHistPos])
        {
            m_bHistClicked=true;
            m_lstItems->Select(i, true);
            m_lstItems->EnsureVisible(i);
        }
    }
}

void MainFrame::OnHistoryNextClicked(wxCommandEvent& event)
{
    int iCount=m_arsHistory.GetCount();
    if (m_iHistPos>=(iCount-1)) return;
    m_iHistPos++;
    iCount=m_lstItems->GetItemCount();
    for (int i=0; i<iCount; ++i)
    {
        if (m_lstItems->GetItemText(i, 1)==m_arsHistory[m_iHistPos])
        {
            m_bHistClicked=true;
            m_lstItems->Select(i, true);
            m_lstItems->EnsureVisible(i);
        }
    }
}

void MainFrame::OnCompareClicked(wxCommandEvent& event)
{
    int iRes=wxNO;
    if (m_datas.HasCompResults(true))
    {
        wxString sMsg=_("Datas has already been compared with the following file:");
        sMsg << _T("\n") << m_datas.GetLastComparedFile() << _T("\n");
        sMsg << _("- Click \"Yes\" to see the results of this comparison") << _T("\n");
        sMsg << _("- Click \"No\" to make a new comparison");
        iRes=wxMessageBox(sMsg, _("Already done"), wxICON_QUESTION|wxYES_NO|wxCANCEL|wxCENTER);
        if (iRes==wxCANCEL) return;
    }
    if (iRes==wxNO)
    {
        wxString sMsg=_("Select the \"GED\" file to open");
        wxString sWlcrd=_("Gedcom files (*.ged)|*.ged|All files (*.*)|*.*");
        wxString sFName=wxFileSelector(sMsg, wxGetCwd(), wxEmptyString, _T("ged"), sWlcrd, wxFD_OPEN|wxFD_FILE_MUST_EXIST);
        if (sFName.IsEmpty()) return;

        if (!m_datas.CompareWithGedFile(sFName))
        {
            wxMessageBox(m_datas.GetLastError(), _("Error"), wxICON_EXCLAMATION|wxCENTER|wxOK);
            return;
        }
    }

    wxMessageBox(m_datas.GetCompResultsSummary(), _("Results"), wxICON_INFORMATION|wxCENTER|wxOK);
}

void MainFrame::OnShashPosChanged(wxSplitterEvent& event)
{
    m_settings.SetLastSashPos(m_spwSplitter->GetSashPosition());
}

int MainFrame::SortCompFunction(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData)
{
    wxXmlNode *node1=(wxXmlNode*)item1, *node2=(wxXmlNode*)item2;
    if ((node1==NULL)||(node2==NULL)) return 0;
    int iData=sortData;
    long v1, v2;
    wxString s1, s2;
    if (iData<0)
    {
        iData*=-1;
        node1=node2;
        node2=(wxXmlNode*)item1;
    }
    switch(iData)
    {
        case SORTCOL_SEX:
            v1=DatasManager::Get().GetItemSex(node1);
            v2=DatasManager::Get().GetItemSex(node2);
            if (v1!=v2)
            {
                if (v1<v2)
                    return -1;
                else
                    return 1;
            }
            else
            {
                return 0;
            }
            break;
        case SORTCOL_ID:
            s1=node1->GetAttribute(_T("GedId"));
            v1=0;
            if (!s1.IsEmpty())
            {
                s1.Replace(_T('@'), _T(' '));
                s1.Replace(_T('I'), _T(' '));
                s1.ToLong(&v1);
            }
            s2=node2->GetAttribute(_T("GedId"));
            v2=0;
            if (!s2.IsEmpty())
            {
                s2.Replace(_T('@'), _T(' '));
                s2.Replace(_T('I'), _T(' '));
                s2.ToLong(&v2);
            }
            if (v1!=v2)
            {
                if (v1<v2)
                    return -1;
                else
                    return 1;
            }
            else
            {
                return 0;
            }
            break;
        case SORTCOL_LNAME:
            return DatasManager::Get().GetItemFullName(node1, true).CompareTo(DatasManager::Get().GetItemFullName(node2, true));
            break;
        case SORTCOL_FNAME:
            return DatasManager::Get().GetItemFullName(node1, false).CompareTo(DatasManager::Get().GetItemFullName(node2, false));
            break;
        default:
            return 0;
    }
    return 0;
}
