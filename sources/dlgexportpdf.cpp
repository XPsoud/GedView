#include "dlgexportpdf.h"

#include "datasmanager.h"

#include <wx/pdfdoc.h>
#include <wx/pdffont.h>
#include <wx/pdffontmanager.h>
#include <wx/xml/xml.h>
#include <wx/statline.h>

DlgExportPdf::DlgExportPdf(wxWindow *parent) : wxDialog(parent, -1, _("Export as pdf file"), wxDefaultPosition, wxDefaultSize),
    m_datas(DatasManager::Get())
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Creating a \"DlgExportPdf\" object\n"));
#endif // __WXDEBUG__

    m_SelectedItem=NULL;

	CreateControls();

	UpdateControlsStates();

	ConnectControls();

	CenterOnParent();
}

DlgExportPdf::~DlgExportPdf()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Destroying a \"DlgExportPdf\" object\n"));
#endif // __WXDEBUG__
}

void DlgExportPdf::CreateControls()
{
    wxStaticText *label;
    wxBoxSizer *szrMain=new wxBoxSizer(wxVERTICAL), *vszr, *hszr;

        label=new wxStaticText(this, -1, _("What do you want to export:"));
        szrMain->Add(label, 0, wxLEFT|wxRIGHT|wxTOP, 5);

        szrMain->AddSpacer(1);

        vszr=new wxBoxSizer(wxVERTICAL);

            m_optExportType[0]=new wxRadioButton(this, -1, _("The selected item only"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
            vszr->Add(m_optExportType[0], 0, wxALL, 5);
            m_optExportType[1]=new wxRadioButton(this, -1, _("All the items in memory"));
            vszr->Add(m_optExportType[1], 0, wxLEFT|wxRIGHT|wxBOTTOM, 5);
            m_optExportType[2]=new wxRadioButton(this, -1, _("Items affected by the last comparison"));
            vszr->Add(m_optExportType[2], 0, wxLEFT|wxRIGHT|wxBOTTOM, 5);

        szrMain->Add(vszr, 0, wxLEFT, 5);

        m_chkSummary=new wxCheckBox(this, -1, _("Add a summary of the exported items"));
        szrMain->Add(m_chkSummary, 0, wxLEFT|wxRIGHT|wxBOTTOM, 5);

        m_chkSplitPdf=new wxCheckBox(this, -1, _("Create one pdf file per item"));
        szrMain->Add(m_chkSplitPdf, 0, wxLEFT|wxRIGHT|wxBOTTOM, 5);

        m_pgbProgress=new wxGauge(this, -1, 100);
        szrMain->Add(m_pgbProgress, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 5);

        szrMain->Add(new wxStaticLine(this, -1), 0, wxALL|wxEXPAND, 5);

        hszr=new wxBoxSizer(wxHORIZONTAL);

            m_btnExport=new wxButton(this, -1, _("Export"));
            hszr->Add(m_btnExport, 0, wxALL, 0);

            hszr->AddSpacer(10);

            m_btnCancel=new wxButton(this, wxID_CANCEL, _("Cancel"));
            hszr->Add(m_btnCancel, 0, wxALL, 0);

        szrMain->Add(hszr, 0, wxALL|wxALIGN_RIGHT, 5);


    SetSizer(szrMain);

    szrMain->SetSizeHints(this);
}

void DlgExportPdf::ConnectControls()
{
    for (int i=0; i<3; ++i)
        m_optExportType[i]->Bind(wxEVT_RADIOBUTTON, &DlgExportPdf::OnOptExportTypeClicked, this);
    m_btnExport->Bind(wxEVT_BUTTON, &DlgExportPdf::OnBtnExportClicked, this);
}

void DlgExportPdf::UpdateControlsStates()
{
    if (m_SelectedItem==NULL)
    {
        if (!m_datas.HasCompResults())
        {
            m_optExportType[0]->Disable();
            m_optExportType[1]->SetValue(true);
            m_optExportType[2]->Disable();
            m_chkSummary->SetValue(true);
            m_chkSummary->Enable();
            m_chkSplitPdf->Enable();
        }
        else
        {
            m_optExportType[0]->Disable();
            m_optExportType[1]->SetValue(false);
            m_optExportType[2]->SetValue(true);
            m_chkSummary->SetValue(true);
            m_chkSummary->Enable();
            m_chkSplitPdf->Enable();
        }
    }
    else
    {
        if (!m_datas.HasCompResults())
        {
            m_optExportType[0]->Enable();
            m_optExportType[0]->SetValue(true);
            m_optExportType[2]->Disable();
            m_chkSummary->SetValue(false);
            m_chkSummary->Disable();
            m_chkSplitPdf->Disable();
        }
        else
        {
            m_optExportType[0]->Enable();
            m_optExportType[0]->SetValue(true);
            m_chkSummary->SetValue(false);
            m_chkSummary->Disable();
            m_chkSplitPdf->Disable();
        }
    }
}

void DlgExportPdf::SetSelectedItem(wxXmlNode* itmNode)
{
    m_SelectedItem=itmNode;
    UpdateControlsStates();
}

void DlgExportPdf::OnOptExportTypeClicked(wxCommandEvent& event)
{
    if (m_optExportType[0]->GetValue())
    {
        m_chkSummary->Disable();
        m_chkSplitPdf->Disable();
    }
    else
    {
        m_chkSummary->Enable();
        m_chkSplitPdf->Enable();
    }
}

void DlgExportPdf::OnBtnExportClicked(wxCommandEvent& event)
{
    m_FName=m_datas.GetCurrentFileName();
    m_FName.SetExt(_T("pdf"));

    wxString sMsg=_("Select the \"PDF\" file to create");
    wxString sWlcrd=_("Pdf files (*.pdf)|*.pdf|All files (*.*)|*.*");
    wxString sFName=wxFileSelector(sMsg, m_FName.GetPath(), m_FName.GetFullName(), _T("pdf"), sWlcrd, wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (sFName.IsEmpty()) return;
    m_FName.Assign(sFName);

    int iExpType=-1;
    for (int i=0; i<3; ++i)
    {
        if (m_optExportType[i]->GetValue())
            iExpType=i;
    }
    switch(iExpType)
    {
        case 0:
            DoExportSelectedItem();
            break;
        case 1:
            DoExportAllItems();
            break;
        case 2:
            DoExportCompResults();
            break;
    }

    EndModal(wxID_OK);
}


void DlgExportPdf::AddHrTitle(double Y, const wxString& title, wxPdfDocument *doc)
{
    wxString sName=doc->GetFontFamily();
    wxString sStyle=doc->GetFontStyle();
    double dSize=doc->GetFontSize();

    doc->SetFont(_T("Helvetica"), _T("BI"), 16);
    double dWidth=doc->GetStringWidth(title);
    doc->SetXY(10, Y);
    doc->Cell(dWidth, 10, title, wxPDF_BORDER_NONE, 1);
    doc->Line(dWidth+15, Y+5, 200, Y+5);

    doc->SetFont(sName, sStyle, dSize);
}

void DlgExportPdf::Summary2Pdf(wxPdfDocument *doc, wxArrayString* pArsItems)
{
    doc->AddPage();
    doc->SetMargins(10, 10, 10);
    doc->SetAutoPageBreak(true, 10);
    doc->SetFont(_T("Helvetica"), _T(""), 20);
    wxString sHtml=_T("<table border=\"0\" width=\"100%\"><tbody><tr><td align=\"center\">");
    sHtml << _("Alphabetical Index") << _T("</td></tr></tbody></table><br />");
    doc->WriteXml(sHtml);
    doc->SetFont(_T("Helvetica"), _T(""), 12);
    doc->SetLineHeight(3);
    sHtml=_T("<table border=\"1\" cellpadding=\"2\"><colgroup><col width=\"30\" span=\"1\" /><col width=\"160\" span=\"1\" /></colgroup><thead><tr><td align=\"center\"><b>");
    sHtml << _("Id") << _T("</b></td><td align=\"center\"><b>") << _("Full name") << _T("</b></td></tr></thead>");
    sHtml << _T("<tbody>");

    wxArrayString arsItems;
    arsItems.Clear();
    if (pArsItems==NULL)
    {
        wxXmlNode *root=m_datas.GetDatas();
        wxXmlNode *node=root->GetChildren();
        while(node!=NULL)
        {
            if (node->GetAttribute(_T("Type"))==_T("INDI"))
            {
                arsItems.Add(m_datas.GetItemFullName(node) + wxString(_T("          ") + node->GetAttribute(_T("GedId"))).Right(10));
            }
            node=node->GetNext();
        }
    }
    else
    {
        wxArrayString& items=*pArsItems;
        for (size_t i=0; i<items.GetCount(); ++i)
        {
            int iPos=items[i].Find(_T(' '));
            wxString sItem=items[i].Right(items[i].Length()-iPos);
            sItem.Trim(false);
            sItem.Trim(true);
            wxString sId=_T("          ") + items[i].Left(iPos);
            sId.Trim();
            sItem << sId.Right(10);
            arsItems.Add(sItem);
        }
    }
    arsItems.Sort();
    for (size_t i=0; i<arsItems.GetCount(); ++i)
    {
        sHtml << _T("<tr>");
        sHtml << _T("<td align=\"right\">") << arsItems[i].Right(10) << _T("</td>");
        sHtml << _T("<td>") << arsItems[i].Left(arsItems[i].Length()-10).Trim(false) << _T("</td>");
        sHtml << _T("</tr>");
    }
    sHtml << _T("</tbody></table>"),
    doc->WriteXml(sHtml);
}

void DlgExportPdf::GedItem2Pdf(wxXmlNode *itmNode, wxPdfDocument *doc)
{
    if (itmNode==NULL) return;
    wxString sItmID=itmNode->GetAttribute(_T("GedId"));

    doc->AddPage(wxPORTRAIT, wxPAPER_A4);
    doc->SetMargins(10, 10, 10);
    doc->SetAutoPageBreak(true, 10);
    wxPdfArrayDouble dash;
    wxPdfLineStyle lstyle(0.5, wxPDF_LINECAP_BUTT, wxPDF_LINEJOIN_MITER, dash, 0., wxColour(0, 0, 0));
    doc->SetLineStyle(lstyle);

    doc->SetXY(10, 10);

    doc->SetCellMargin(2);

    int iSex=m_datas.GetItemSex(itmNode);

    wxString sTxt=wxEmptyString;

    doc->SetFont(_T("Arial"), _T(" "), 15);
    doc->Cell(30, 10, sItmID, wxPDF_BORDER_FRAME, 0, wxPDF_ALIGN_RIGHT);
    doc->SetFontSize(18);
    sTxt << m_datas.GetSexChar(iSex) << _T(" ") << m_datas.GetItemFullName(itmNode);
    doc->Cell(160, 10, sTxt, wxPDF_BORDER_FRAME, 1, wxPDF_ALIGN_LEFT);

    wxXmlNode *subNode=itmNode->GetChildren();
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
            doc->SetFont(_T("Arial"), _T(""), 10);
            doc->Cell(190, 8, sEvt, wxPDF_BORDER_NONE, 1);
            // Search for a "Source" sub node of the event
            wxXmlNode* subSubNode=subNode->GetChildren();
            while(subSubNode!=NULL)
            {
                if (subSubNode->GetAttribute(_T("Type"))==_T("SOUR"))
                {
                    sEvt=_T("    ");
                    sEvt << _("Source:") << _T(" ") << subSubNode->GetAttribute(_T("Value"));
                    doc->SetFont(_T("Arial"), _T(""), 8);
                    doc->Cell(190, 4, sEvt, wxPDF_BORDER_NONE, 1);
                }
                subSubNode=subSubNode->GetNext();
            }
        }

        doc->SetFont(_T("Arial"), _T(""), 10);
        if (sType==_T("OCCU"))
        {
            sTxt= _("Occupation:") + _T(" ") + subNode->GetAttribute(_T("Value"));
            doc->Cell(190, 8, sTxt, wxPDF_BORDER_NONE, 1);
        }

        if (sType==_T("FAMC"))
        {
            AddHrTitle(doc->GetY(), _("Parents"), doc);
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
                        doc->SetFontSize(12);
                        sTxt=(sSubTyp==_T("HUSB")?_("Father:"):_("Mother:")) + _T(" ") + sSubId + _T(" - ") + m_datas.GetItemFullName(sSubId);
                        doc->Cell(190, 6, sTxt, wxPDF_BORDER_NONE, 1);
                        sEvent=m_datas.GetItemBirth(sSubId);
                        doc->SetFontSize(10);
                        if (!sEvent.IsEmpty())
                            doc->Cell(190, 5, _T("   ") + sEvent, wxPDF_BORDER_NONE, 1);
                        sEvent=m_datas.GetItemDeath(sSubId);
                        if (!sEvent.IsEmpty())
                        {
                            if ((sEvent==_("Dead")) && (sSubTyp==_T("WIFE")))
                                sEvent=_("Dead_F");
                            doc->Cell(190, 5, _T("   ") + sEvent, wxPDF_BORDER_NONE, 1);
                        }
                        doc->SetFontSize(12);
                    }
                    if (sSubTyp==_T("CHIL"))
                    {
                        arsSiblings.Add(sSubId);
                    }
                    subEvt=subEvt->GetNext();
                }

                if (arsSiblings.GetCount()>1)
                {
                    AddHrTitle(doc->GetY(), _("Siblings"), doc);
                    for (size_t s=0; s<arsSiblings.GetCount(); s++)
                    {
                        if (arsSiblings[s]!=sItmID)
                        {
                            int iSSex=m_datas.GetItemSex(arsSiblings[s]);
                            sTxt=arsSiblings[s] + _T(" - ") + m_datas.GetItemFirstName(arsSiblings[s]);
                            sTxt << _T(" (") << (iSSex==GIS_MALE?_T('M'):iSSex==GIS_FEMALE?_T('F'):_T('?')) << _T(")";)
                            doc->Cell(190, 6, sTxt, wxPDF_BORDER_NONE, 1);
                            sEvent=m_datas.GetItemBirth(arsSiblings[s]);
                            doc->SetFontSize(10);
                            if (!sEvent.IsEmpty())
                                doc->Cell(190, 5, _T("   ") + sEvent, wxPDF_BORDER_NONE, 1);
                            sEvent=m_datas.GetItemDeath(arsSiblings[s]);
                            if (!sEvent.IsEmpty())
                            {
                                if ((sEvent==_("Dead")) && (iSSex==GIS_FEMALE))
                                    sEvent=_("Dead_F");
                                doc->Cell(190, 5, _T("   ") + sEvent, wxPDF_BORDER_NONE, 1);
                            }
                            doc->SetFontSize(12);
                        }
                    }
                }
            }
        }
        if (sType==_T("FAMS"))
        {
            if (!bUnions)
            {
                AddHrTitle(doc->GetY(), _("Union(s)"), doc);
                bUnions=true;
            }
            wxXmlNode* evtNode=m_datas.FindItemByGedId(subNode->GetAttribute(_T("Value")));
            wxString sEvent;
            doc->SetFontSize(10);
            if (evtNode!=NULL)
            {
                wxXmlNode *subEvt=evtNode->GetChildren();
                wxString sSubTyp;
                // First pass to search for the event type and the source
                while(subEvt!=NULL)
                {
                    sSubTyp=subEvt->GetAttribute(_T("Type"));
                    if (subEvt->GetName()==_T("Event"))
                    {
                        sTxt=m_datas.ParseEvent(subEvt);
                        if (!sTxt.IsEmpty())
                        {
                            doc->Cell(190, 5, sTxt, wxPDF_BORDER_NONE, 1);
                        }
                        wxXmlNode *subSubNode=subEvt->GetChildren();
                        while(subSubNode!=NULL)
                        {
                            if (subSubNode->GetAttribute(_T("Type"))==_T("SOUR"))
                            {
                                sTxt = _T("   ");
                                sTxt << _("Source:") << _T(" ") << subSubNode->GetAttribute(_T("Value"));
                                doc->Cell(190, 5, sTxt, wxPDF_BORDER_NONE, 1);
                            }
                            subSubNode=subSubNode->GetNext();
                        }
                    }
                    subEvt=subEvt->GetNext();
                }
                doc->SetFontSize(12);
                // Second pass for Husband/Wife and children
                subEvt=evtNode->GetChildren();
                while(subEvt!=NULL)
                {
                    wxString sEvtId=subEvt->GetAttribute(_T("GedId"));
                    sSubTyp=subEvt->GetAttribute(_T("Type"));
                    if (((sSubTyp==_T("HUSB"))||(sSubTyp==_T("WIFE")))&&(!sEvtId.IsEmpty())&&(sEvtId!=sItmID))
                    {
                        doc->Cell(190, 6, sEvtId + _T(" - ") + m_datas.GetItemFullName(sEvtId), wxPDF_BORDER_NONE, 1);
                        sEvent=m_datas.GetItemBirth(sEvtId);
                        doc->SetFontSize(10);
                        if (!sEvent.IsEmpty())
                            doc->Cell(190, 5, _T("   ") + sEvent, wxPDF_BORDER_NONE, 1);
                        sEvent=m_datas.GetItemDeath(sEvtId);
                        if (!sEvent.IsEmpty())
                        {
                            if ((sEvent==_("Dead")) && (sSubTyp==_T("WIFE")))
                                sEvent=_("Dead_F");
                            doc->Cell(190, 5, _T("   ") + sEvent, wxPDF_BORDER_NONE, 1);
                        }
                        doc->SetFontSize(12);
                    }
                    if (sSubTyp==_T("CHIL"))
                    {
                        int iCSex=m_datas.GetItemSex(sEvtId);
                        sTxt=_T("      ");
                        sTxt << m_datas.GetSexChar(iCSex) << _T(" ") << sEvtId << _T(" - ") << m_datas.GetItemFirstName(sEvtId);
                        doc->Cell(190, 6, sTxt, wxPDF_BORDER_NONE, 1);
                        sEvent=m_datas.GetItemBirth(sEvtId);
                        doc->SetFontSize(10);
                        if (!sEvent.IsEmpty())
                            doc->Cell(190, 5, _T("         ") + sEvent, wxPDF_BORDER_NONE, 1);
                        sEvent=m_datas.GetItemDeath(sEvtId);
                        if (!sEvent.IsEmpty())
                        {
                            if ((sEvent==_("Dead")) && (iCSex==GIS_FEMALE))
                                sEvent=_("Dead_F");
                            doc->Cell(190, 5, _T("         ") + sEvent, wxPDF_BORDER_NONE, 1);
                        }
                        doc->SetFontSize(12);
                    }
                    subEvt=subEvt->GetNext();
                }
            }
        }

        subNode=subNode->GetNext();
    }
}

wxPdfDocument* DlgExportPdf::InitPdfDocument()
{
    wxPdfDocument *doc=new wxPdfDocument(wxPORTRAIT, _T("mm"), wxPAPER_A4);

    wxFont fontArial(14, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL, false, wxT("Arial"), wxFONTENCODING_DEFAULT);
    wxPdfFontManager* fntMngr = wxPdfFontManager::GetFontManager();
    fntMngr->RegisterFont(fontArial);
    SetFont(fontArial);
    doc->AliasNbPages();

    return doc;
}

void DlgExportPdf::DoExportSelectedItem()
{
    wxPdfDocument *doc=InitPdfDocument();

    GedItem2Pdf(m_SelectedItem, doc);
    doc->SaveAsFile(m_FName.GetFullPath());
    m_pgbProgress->SetValue(100);
    m_pgbProgress->Refresh();

}

void DlgExportPdf::DoExportAllItems()
{
    double dTotal=m_datas.GetItemsCount(GIT_INDI), dDone=0., dPrct;

    wxPdfDocument *doc=InitPdfDocument();

    wxString sBaseFName=m_FName.GetName();

    if (m_chkSummary->IsChecked())
    {
        Summary2Pdf(doc);
        if (m_chkSplitPdf->IsChecked())
        {
            dTotal++;
            dDone=1;
            wxString sName=sBaseFName + _T("-") + _("Summary");
            m_FName.SetName(sName);
            doc->SaveAsFile(m_FName.GetFullPath());
            delete doc;
            doc=new wxPdfDocument(wxPORTRAIT, _T("mm"), wxPAPER_A4);
            doc->SetFont(_T("Helvetica"), _T(""), 14);
            doc->AliasNbPages();
            dPrct=100./dTotal;
            m_pgbProgress->SetValue(dPrct);
            m_pgbProgress->Refresh();
            wxTheApp->Yield();
        }
    }
    // Create a page for each individual item
    wxXmlNode *root=m_datas.GetDatas();
    wxXmlNode *item=root->GetChildren();
    while(item!=NULL)
    {
        if (item->GetAttribute(_T("Type"))==_T("INDI"))
        {
            dDone++;
            dPrct=dDone*100./dTotal;
            m_pgbProgress->SetValue(dPrct);
            m_pgbProgress->Refresh();
            wxTheApp->Yield();
            GedItem2Pdf(item, doc);
            if (m_chkSplitPdf->IsChecked())
            {
                wxString sName=sBaseFName + _T("-") + item->GetAttribute(_T("GedId"));
                m_FName.SetName(sName);
                doc->SaveAsFile(m_FName.GetFullPath());
                delete doc;
                doc=new wxPdfDocument(wxPORTRAIT, _T("mm"), wxPAPER_A4);
                doc->SetFont(_T("Helvetica"), _T(""), 14);
                doc->AliasNbPages();
            }
        }

        item=item->GetNext();
    }
    m_pgbProgress->SetValue(100);
    m_pgbProgress->Refresh();
    if (!m_chkSplitPdf->IsChecked())
        doc->SaveAsFile(m_FName.GetFullPath());
    delete doc;
}

void DlgExportPdf::DoExportCompResults()
{
    if (!m_datas.HasCompResults())
    {
        wxMessageBox(_("No exportable comparison results!"), _("Error"), wxICON_EXCLAMATION|wxCENTER|wxOK);
        return;
    }

    wxArrayString arsItems;
    arsItems.Clear();
    const wxArrayString& mitems=m_datas.GetModifiedCompResults();
    for (size_t i=0; i<mitems.GetCount(); ++i)
        arsItems.Add(mitems[i]);
    const wxArrayString& aitems=m_datas.GetAddedCompResults();
    for (size_t i=0; i<aitems.GetCount(); ++i)
        arsItems.Add(aitems[i]);

    double dTotal=arsItems.GetCount(), dDone=0., dPrct;

    wxPdfDocument *doc=InitPdfDocument();

    wxString sBaseFName=m_FName.GetName();

    if (m_chkSummary->IsChecked())
    {
        Summary2Pdf(doc, &arsItems);
        if (m_chkSplitPdf->IsChecked())
        {
            dTotal++;
            dDone=1;
            wxString sName=sBaseFName + _T("-") + _("Summary");
            m_FName.SetName(sName);
            doc->SaveAsFile(m_FName.GetFullPath());
            delete doc;
            doc=new wxPdfDocument(wxPORTRAIT, _T("mm"), wxPAPER_A4);
            doc->SetFont(_T("Helvetica"), _T(""), 14);
            doc->AliasNbPages();
            dPrct=100./dTotal;
            m_pgbProgress->SetValue(dPrct);
            m_pgbProgress->Refresh();
            wxTheApp->Yield();
        }
    }
    // Create a page for each individual item
    wxXmlNode *item;
    for (size_t i=0; i<arsItems.GetCount(); ++i)
    {
        int iPos=arsItems[i].Find(_T(' '));
        wxString sId=arsItems[i].Left(iPos);
        item=m_datas.FindItemByGedId(sId);
        if (item!=NULL)
        {
            dDone++;
            dPrct=dDone*100./dTotal;
            m_pgbProgress->SetValue(dPrct);
            m_pgbProgress->Refresh();
            wxTheApp->Yield();
            GedItem2Pdf(item, doc);
            if (m_chkSplitPdf->IsChecked())
            {
                wxString sName=sBaseFName + _T("-") + item->GetAttribute(_T("GedId"));
                m_FName.SetName(sName);
                doc->SaveAsFile(m_FName.GetFullPath());
                delete doc;
                doc=new wxPdfDocument(wxPORTRAIT, _T("mm"), wxPAPER_A4);
                doc->SetFont(_T("Helvetica"), _T(""), 14);
                doc->AliasNbPages();
            }
        }
    }
    m_pgbProgress->SetValue(100);
    m_pgbProgress->Refresh();
    if (!m_chkSplitPdf->IsChecked())
        doc->SaveAsFile(m_FName.GetFullPath());
    delete doc;
}
