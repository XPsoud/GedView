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
#include "dlgcheckdatas.h"
#include "dlgdatasinfos.h"
#include "settingsmanager.h"

#include <wx/display.h>
#include <wx/filedlg.h>
#include <wx/artprov.h>
#include <wx/xml/xml.h>
#include <wx/wfstream.h>

#ifndef __WXMSW__
#include "../graphx/wxwin32x32.xpm"
#endif // __WXMSW__

wxDEFINE_EVENT(wxEVT_FILEOPEN, wxCommandEvent);

enum SortColumn
{
    SORT_COL_UNKNOWN =0,
    SORTCOL_SEX,
    SORTCOL_ID,
    SORTCOL_LNAME,
    SORTCOL_FNAME
};

const int wxID_REOPEN  = wxNewId();
const int wxID_PDFLIST = wxNewId();
const int wxID_PDFTREE = wxNewId();
const int wxID_CSVFILE = wxNewId();
const int wxID_CHKDATA = wxNewId();
const int wxID_DBINFOS = wxNewId();
const int wxID_FLTRPNL = wxNewId();

#define MAINFRAME_MINIMAL_SIZE wxSize(600, 400)

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

    SetMinSize(MAINFRAME_MINIMAL_SIZE);

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

        tb->AddTool(wxID_OPEN, wxGetStockLabel(wxID_OPEN), wxGet_open_png_Bitmap(), _("Open a ged file"), wxITEM_DROPDOWN);
        m_mnuRecents = new wxMenu;
        tb->SetDropdownMenu(wxID_OPEN, m_mnuRecents);
#ifdef __WXMAC__
        tb->AddTool(wxID_REOPEN, _("Reopen file"), wxGet_reopen_png_Bitmap(), _("Reopen a ged file"));
#endif // __WXMAC__
        tb->AddTool(wxID_SAVE, wxGetStockLabel(wxID_SAVE), wxGet_save_png_Bitmap(), _("Save datas to xml file"));

        tb->AddTool(wxID_PDFLIST, _("Export as pdf file"), wxGet_pdf_png_Bitmap(), _("Export the datas list to a pdf file"));

        tb->AddTool(wxID_PDFTREE, _("Create pdf tree"), wxGet_pdftree_png_Bitmap(), _("Export selected item's tree to a pdf file"));

        tb->AddTool(wxID_CSVFILE, _("Export to csv"), wxGet_csv_png_Bitmap(), _("Export datas to a csv file"));

        tb->AddSeparator();

        tb->AddTool(wxID_BACKWARD, wxGetStockLabel(wxID_BACKWARD), wxArtProvider::GetBitmap(wxART_GO_BACK, wxART_TOOLBAR), _("Go backward in shown items history"));

        tb->AddTool(wxID_FORWARD, wxGetStockLabel(wxID_FORWARD), wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_TOOLBAR), _("Go forward in shown items history"));

        tb->AddSeparator();

        tb->AddTool(wxID_SPELL_CHECK, _("Compare"), wxGet_compare_png_Bitmap(), _("Compare datas with an other ged file"));

        tb->AddTool(wxID_CHKDATA, _("Check"), wxGet_datacheck_png_Bitmap(), _("Check loaded datas for potential errors"));

        tb->AddTool(wxID_DBINFOS, _("Informations"), wxGet_infos_png_Bitmap(), _("Show informations about the loaded gedcom file"));

        tb->AddStretchableSpace();

        tb->AddTool(wxID_PREFERENCES, wxGetStockLabel(wxID_PREFERENCES), wxGet_preferences_png_Bitmap(), _("Edit application settings"));

        tb->AddTool(wxID_ABOUT, wxGetStockLabel(wxID_ABOUT), wxGet_about_png_Bitmap(), wxGetStockHelpString(wxID_ABOUT, wxSTOCK_MENU));

    tb->Realize();

    m_settings.GetRecentsList().SetAssociatedMenu(m_mnuRecents);

    // OSX only : create the Preferences, About and Exit menu entries to be attached
    // to the OSX-specific menu
#ifdef __WXMAC__
    wxMenuBar *menuBar = new wxMenuBar();
    // Temporary menu for creating the entries
    wxMenu *tmpMenu = new wxMenu();
    tmpMenu->Append(wxID_PREFERENCES, wxGetStockLabel(wxID_PREFERENCES, wxSTOCK_WITH_MNEMONIC|wxSTOCK_WITH_ACCELERATOR), _("Edit application settings"));
    tmpMenu->Append(wxID_EXIT, wxGetStockLabel(wxID_EXIT, wxSTOCK_WITH_MNEMONIC|wxSTOCK_WITH_ACCELERATOR), wxGetStockHelpString(wxID_EXIT));
    tmpMenu->Append(wxID_ABOUT, wxGetStockLabel(wxID_ABOUT, wxSTOCK_WITH_MNEMONIC|wxSTOCK_WITH_ACCELERATOR), wxGetStockHelpString(wxID_ABOUT));
    menuBar->Append(tmpMenu, wxGetStockLabel(wxID_FILE));
    SetMenuBar(menuBar);
    // Now, the 3 menu entries have been automatically placed in the OSX menu
    // The temporary menu is empty, we can remove and delete it
    menuBar->Remove(0);
    delete tmpMenu;
#endif // __WXMAC__

    // Controls
    wxPanel *pnl=new wxPanel(this, -1);
    wxBoxSizer *szrMain=new wxBoxSizer(wxVERTICAL);

    m_pnlFilter=new PanelFilter(pnl, wxID_FLTRPNL);
    szrMain->Add(m_pnlFilter, 0, wxALL|wxEXPAND, 0);

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
    Bind(wxEVT_SIZE, &MainFrame::OnSize, this);
    Bind(wxEVT_MOVE, &MainFrame::OnMove, this);
    Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this);
    // Menus/Toolbar items
    Bind(wxEVT_TOOL, &MainFrame::OnOpenGedFileClicked, this, wxID_OPEN);
#ifdef __WXMAC__
    Bind(wxEVT_TOOL, &MainFrame::OnReopenToolClicked, this, wxID_REOPEN);
#endif // __WXMAC__
    Bind(wxEVT_TOOL, &MainFrame::OnReopenGedFileClicked, this, wxID_FILE, wxID_FILE9);
    Bind(wxEVT_TOOL, &MainFrame::OnSaveXmlFileClicked, this, wxID_SAVE);
    Bind(wxEVT_TOOL, &MainFrame::OnSavePdfFileClicked, this, wxID_PDFLIST);
    Bind(wxEVT_TOOL, &MainFrame::OnSavePdfTreeClicked, this, wxID_PDFTREE);
    Bind(wxEVT_TOOL, &MainFrame::OnSaveCsvFileClicked, this, wxID_CSVFILE);
    Bind(wxEVT_TOOL, &MainFrame::OnHistoryBackClicked, this, wxID_BACKWARD);
    Bind(wxEVT_TOOL, &MainFrame::OnHistoryNextClicked, this, wxID_FORWARD);
    Bind(wxEVT_TOOL, &MainFrame::OnCompareClicked, this, wxID_SPELL_CHECK);
    Bind(wxEVT_TOOL, &MainFrame::OnCheckDatasClicked, this, wxID_CHKDATA);
    Bind(wxEVT_TOOL, &MainFrame::OnDatasInfosClicked, this, wxID_DBINFOS);
    Bind(wxEVT_TOOL, &MainFrame::OnPreferencesClicked, this, wxID_PREFERENCES);
    Bind(wxEVT_TOOL, &MainFrame::OnAboutClicked, this, wxID_ABOUT);
    // Other controls events handlers
    m_lstItems->Bind(wxEVT_LIST_ITEM_SELECTED, &MainFrame::OnListItemSelected, this);
    m_lstItems->Bind(wxEVT_LIST_ITEM_DESELECTED, &MainFrame::OnListItemDeselected, this);
    m_lstItems->Bind(wxEVT_LIST_COL_CLICK, &MainFrame::OnColumnHeaderClicked, this);
    m_lstItems->Bind(wxEVT_LIST_COL_END_DRAG, &MainFrame::OnColumnHeaderDragged, this);
    m_htwDetails->Bind(wxEVT_HTML_LINK_CLICKED, &MainFrame::OnHtmlLinkClicked, this);
    m_spwSplitter->Bind(wxEVT_SPLITTER_SASH_POS_CHANGED, &MainFrame::OnShashPosChanged, this);
    // Custom events handlers
    Bind(wxEVT_FILEOPEN, &MainFrame::OnAutoOpenGedFile, this);
    Bind(wxEVT_FILTER_CHANGED, &MainFrame::OnFilterChanged, this);
    // UpdateUI events handlers
    Bind(wxEVT_UPDATE_UI, &MainFrame::OnUpdateUI_Save, this, wxID_SAVE);
    Bind(wxEVT_UPDATE_UI, &MainFrame::OnUpdateUI_Save, this, wxID_PDFLIST);
    Bind(wxEVT_UPDATE_UI, &MainFrame::OnUpdateUI_PdfTree, this, wxID_PDFTREE);
    Bind(wxEVT_UPDATE_UI, &MainFrame::OnUpdateUI_Save, this, wxID_CSVFILE);
    Bind(wxEVT_UPDATE_UI, &MainFrame::OnUpdateUI_Backward, this, wxID_BACKWARD);
    Bind(wxEVT_UPDATE_UI, &MainFrame::OnUpdateUI_Forward, this, wxID_FORWARD);
    Bind(wxEVT_UPDATE_UI, &MainFrame::OnUpdateUI_DatasTools, this, wxID_SPELL_CHECK);
    Bind(wxEVT_UPDATE_UI, &MainFrame::OnUpdateUI_DatasTools, this, wxID_CHKDATA);
    Bind(wxEVT_UPDATE_UI, &MainFrame::OnUpdateUI_DatasTools, this, wxID_DBINFOS);
    Bind(wxEVT_UPDATE_UI, &MainFrame::OnUpdateUI_Save, this, wxID_FLTRPNL);
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
    wxChar c=m_datas.GetSexChar(iSex);

    wxString sPage=_T("<h3>");
    sPage <<  c << _T(" ") << m_datas.GetItemFullName(node) << _T("</h3>");
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
            // Search for a "Source" and/or a "Note" sub node of the event
            wxXmlNode* subSubNode=subNode->GetChildren();
            while(subSubNode!=NULL)
            {
                if (subSubNode->GetAttribute(_T("Type"))==_T("SOUR"))
                {
                    sPage << _T("<br />&nbsp;&nbsp;<small>") << _("Source:") << _T(" ");
                    sPage << subSubNode->GetAttribute(_T("Value")) << _T("</small>");
                }
                if (subSubNode->GetAttribute(_T("Type"))==_T("NOTE"))
                {
                    sPage << _T("<br />&nbsp;&nbsp;<small>") << _("Note:") << _T(" ");
                    sPage << subSubNode->GetAttribute(_T("Value"));
                    wxXmlNode *cont=subSubNode->GetChildren();
                    while(cont!=NULL)
                    {
                        if (cont->GetAttribute(_T("Type"))==_T("CONT"))
                        {
                            sPage << _T("<br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
                            sPage << cont->GetAttribute(_T("Value"));
                        }
                        cont=cont->GetNext();
                    }
                    sPage << _T("</small>");
                }
                subSubNode=subSubNode->GetNext();
            }
        }

        if (sType==_T("OCCU"))
        {
            sPage << _T("<br />") << _("Occupation:") << _T(" ") << subNode->GetAttribute(_T("Value"));
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
                            sPage << _T("<br />") << m_datas.GetItemSexChar(arsSiblings[s]) << _T(" <b><a href=\"") << arsSiblings[s] << _T("\">")<< m_datas.GetItemFirstName(arsSiblings[s]) << _T("</a></b>");
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
                // First pass to search for the event type and the source
                while(subEvt!=NULL)
                {
                    sSubTyp=subEvt->GetAttribute(_T("Type"));
                    if (subEvt->GetName()==_T("Event"))
                    {
                        wxString sTmp=m_datas.ParseEvent(subEvt);
                        if (!sTmp.IsEmpty())
                            sPage << _T("<br /><small>") << sTmp << _T("</small>");
                        wxXmlNode *subSubNode=subEvt->GetChildren();
                        while(subSubNode!=NULL)
                        {
                            if (subSubNode->GetAttribute(_T("Type"))==_T("SOUR"))
                            {
                                sPage << _T("<br /><small>&nbsp;&nbsp;&nbsp;");
                                sPage << _("Source:") << _T(" ") << subSubNode->GetAttribute(_T("Value"));
                                sPage << _T("</small>");
                            }
                            if (subSubNode->GetAttribute(_T("Type"))==_T("NOTE"))
                            {
                                sPage << _T("<br />&nbsp;&nbsp;<small>") << _("Note:") << _T(" ");
                                sPage << subSubNode->GetAttribute(_T("Value"));
                                wxXmlNode *cont=subSubNode->GetChildren();
                                while(cont!=NULL)
                                {
                                    if (cont->GetAttribute(_T("Type"))==_T("CONT"))
                                    {
                                        sPage << _T("<br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
                                        sPage << cont->GetAttribute(_T("Value"));
                                    }
                                    cont=cont->GetNext();
                                }
                                sPage << _T("</small>");
                            }
                            subSubNode=subSubNode->GetNext();
                        }
                    }
                    if (subEvt->GetAttribute(_T("Type"))==_T("SOUR"))
                    {
                        sPage << _T("<br /><small>&nbsp;&nbsp;&nbsp;") << _("Source:") << _T(" ");
                        sPage << subEvt->GetAttribute(_T("Value")) << _T("</small>");
                    }
                    if (subEvt->GetAttribute(_T("Type"))==_T("NOTE"))
                    {
                        sPage << _T("<br /><small>&nbsp;&nbsp;&nbsp;") << _("Note:") << _T(" ");
                        sPage << subEvt->GetAttribute(_T("Value"));
                        wxXmlNode *cont=subEvt->GetChildren();
                        while(cont!=NULL)
                        {
                            if (cont->GetAttribute(_T("Type"))==_T("CONT"))
                            {
                                sPage << _T("<br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
                                sPage << cont->GetAttribute(_T("Value"));
                            }
                            cont=cont->GetNext();
                        }
                        sPage << _T("</small>");
                    }
                    subEvt=subEvt->GetNext();
                }
                // Second pass for Husband/Wife and children
                subEvt=evtNode->GetChildren();
                while(subEvt!=NULL)
                {
                    sSubTyp=subEvt->GetAttribute(_T("Type"));
                    wxString sEvtId=subEvt->GetAttribute(_T("GedId"));
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
                        sPage << _T("<li> ") << m_datas.GetItemSexChar(sEvtId) << _T("<a href=\"") << sEvtId << _T("\">") << m_datas.GetItemFirstName(sEvtId) << _T("</a>");
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
    m_settings.GetRecentsList().SetLastUsed(sFName);
    m_arsHistory.Clear();
    m_iHistPos=-1;
    m_bHistClicked=false;

    UpdateList();

    UpdateItemDetails();

    wxMessageBox(_("Ged file read successfully !"), _("Done"), wxICON_EXCLAMATION|wxCENTER|wxOK);
}

#ifdef __WXMAC__
void MainFrame::OnReopenToolClicked(wxCommandEvent& event)
{
    PopupMenu(m_mnuRecents);
}
#endif // __WXMAC__

void MainFrame::OnReopenGedFileClicked(wxCommandEvent& event)
{
    wxString sFile=m_settings.GetRecentsList().GetEntry(event.GetId()-wxID_FILE);
	if (sFile.IsEmpty()) return;
	if (!m_datas.ReadGedFile(sFile))
    {
        wxMessageBox(_("An error occurred while reading the ged file !"), _("Error"), wxICON_EXCLAMATION|wxCENTER|wxOK);
        return;
    }
    m_settings.GetRecentsList().SetLastUsed(sFile);
    m_arsHistory.Clear();
    m_iHistPos=-1;
    m_bHistClicked=false;

    UpdateList();

    UpdateItemDetails();

    wxMessageBox(_("Ged file read successfully !"), _("Done"), wxICON_EXCLAMATION|wxCENTER|wxOK);
}

void MainFrame::OnAutoOpenGedFile(wxCommandEvent& event)
{
    wxString sFName=event.GetString();
    if ((sFName.IsEmpty())||(!wxFileExists(sFName))) return;
    wxFileName fname(sFName);
    fname.Normalize();
    sFName=fname.GetFullPath();
    if (!m_datas.ReadGedFile(sFName))
    {
        wxMessageBox(_("An error occurred while reading the ged file !"), _("Error"), wxICON_EXCLAMATION|wxCENTER|wxOK);
        return;
    }
    m_settings.GetRecentsList().SetLastUsed(sFName);
    wxSetWorkingDirectory(wxPathOnly(sFName));
    m_arsHistory.Clear();
    m_iHistPos=-1;
    m_bHistClicked=false;

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

void MainFrame::OnUpdateUI_DatasTools(wxUpdateUIEvent& event)
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

void MainFrame::OnCheckDatasClicked(wxCommandEvent& event)
{
    DlgCheckDatas dlg(this);
    dlg.ShowModal();
}

void MainFrame::OnDatasInfosClicked(wxCommandEvent& event)
{
    DlgDatasInfos dlg(this);
    dlg.ShowModal();
}

void MainFrame::OnShashPosChanged(wxSplitterEvent& event)
{
    m_settings.SetLastSashPos(m_spwSplitter->GetSashPosition());
}

void MainFrame::OnFilterChanged(wxCommandEvent& event)
{
	wxMessageBox(_T("Filter changed"));
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
            return DatasManager::Get().GetItemFullName(node1, true).CmpNoCase(DatasManager::Get().GetItemFullName(node2, true));
            break;
        case SORTCOL_FNAME:
            return DatasManager::Get().GetItemFullName(node1, false).CmpNoCase(DatasManager::Get().GetItemFullName(node2, false));
            break;
        default:
            return 0;
    }
    return 0;
}
