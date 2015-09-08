#include "dlgexportpdf.h"

#include "datasmanager.h"

#include <wx/pdfdoc.h>
#include <wx/pdffont.h>
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
    m_optExportType[0]->Connect(wxEVT_RADIOBUTTON, wxCommandEventHandler(DlgExportPdf::OnOptExportTypeClicked), NULL, this);
    m_optExportType[1]->Connect(wxEVT_RADIOBUTTON, wxCommandEventHandler(DlgExportPdf::OnOptExportTypeClicked), NULL, this);
    m_btnExport->Connect(wxEVT_BUTTON, wxCommandEventHandler(DlgExportPdf::OnBtnExportClicked), NULL, this);
}

void DlgExportPdf::UpdateControlsStates()
{
    if (m_SelectedItem==NULL)
    {
        m_optExportType[0]->Disable();
        m_optExportType[1]->SetValue(true);
        m_chkSummary->SetValue(true);
        m_chkSummary->Enable();
        m_chkSplitPdf->Enable();
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
    wxFileName fname=m_datas.GetCurrentFileName();
    fname.SetExt(_T("pdf"));

    wxString sMsg=_("Select the \"PDF\" file to create");
    wxString sWlcrd=_("Pdf files (*.pdf)|*.pdf|All files (*.*)|*.*");
    wxString sFName=wxFileSelector(sMsg, fname.GetPath(), fname.GetFullName(), _T("pdf"), sWlcrd, wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (sFName.IsEmpty()) return;
    fname.Assign(sFName);

    double dTotal=m_datas.GetItemsCount(GIT_INDI), dDone=0., dPrct;

    wxPdfDocument *doc=new wxPdfDocument(wxPORTRAIT, _T("mm"), wxPAPER_A4);
    doc->SetFont(_T("Helvetica"), _T(""), 14);
    doc->AliasNbPages();

    if ((m_optExportType[1]->GetValue() && (m_chkSummary->IsChecked())))
    {
        Summary2Pdf(doc);
        if (m_chkSplitPdf->IsChecked())
        {
            dTotal++;
            dDone=1;
            wxString sName=fname.GetName() + _T("-") + _("Summary");
            fname.SetName(sName);
            doc->SaveAsFile(fname.GetFullPath());
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
    if ((m_optExportType[0]->GetValue()) && (m_SelectedItem!=NULL))
    {
        GedItem2Pdf(m_SelectedItem, doc);
        doc->SaveAsFile(sFName);
        m_pgbProgress->SetValue(100);
        m_pgbProgress->Refresh();
    }
    else
    {
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
                    fname.Assign(sFName);
                    wxString sName=fname.GetName() + _T("-") + item->GetAttribute(_T("GedId"));
                    fname.SetName(sName);
                    doc->SaveAsFile(fname.GetFullPath());
                    delete doc;
                    doc=new wxPdfDocument(wxPORTRAIT, _T("mm"), wxPAPER_A4);
                    doc->SetFont(_T("Helvetica"), _T(""), 14);
                    doc->AliasNbPages();
                }
            }

            item=item->GetNext();
        }
    }

    m_pgbProgress->SetValue(100);
    m_pgbProgress->Refresh();
    delete doc;

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

void DlgExportPdf::Summary2Pdf(wxPdfDocument *doc)
{
    doc->AddPage();
    doc->SetFont(_T("Helvetica"), _T(""), 20);
    wxString sHtml=_T("<table border=\"0\" width=\"100%\"><tbody><tr><td align=\"center\">");
    sHtml << _("Alphabetical Index") << _T("</td></tr></tbody></table><br />");
    doc->WriteXml(sHtml);
    doc->SetFont(_T("Helvetica"), _T(""), 12);
    doc->SetLineHeight(5);
    sHtml=_T("<table border=\"1\" cellpadding=\"2\"><colgroup><col width=\"30\" span=\"1\" /><col width=\"160\" span=\"1\" /></colgroup><thead><tr><td align=\"center\"><b>");
    sHtml << _("Id") << _T("</b></td><td align=\"center\"><b>") << _("Full name") << _T("</b></td></tr></thead>");
    sHtml << _T("<tbody>");
    wxXmlNode *root=m_datas.GetDatas();
    wxXmlNode *node=root->GetChildren();
    wxArrayString arsItems;
    arsItems.Clear();
    while(node!=NULL)
    {
        if (node->GetAttribute(_T("Type"))==_T("INDI"))
        {
            arsItems.Add(m_datas.GetItemFullName(node) + wxString(_T("          ") + node->GetAttribute(_T("GedId"))).Right(10));
        }
        node=node->GetNext();
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

    doc->AddPage();
    wxPdfArrayDouble dash;
    wxPdfLineStyle lstyle(0.5, wxPDF_LINECAP_BUTT, wxPDF_LINEJOIN_MITER, dash, 0., wxColour(0, 0, 0));
    doc->SetLineStyle(lstyle);

    doc->SetXY(10, 10);

    doc->SetCellMargin(2);

    doc->SetFont(_T("Arial"), _T(" "), 15);
    doc->Cell(30, 10, sItmID, wxPDF_BORDER_FRAME, 0, wxPDF_ALIGN_RIGHT);
    doc->SetFont(_T("Helvetica"), _T("B"), 18);
    doc->Cell(160, 10, m_datas.GetItemFullName(itmNode), wxPDF_BORDER_FRAME, 1, wxPDF_ALIGN_LEFT);

    wxString sTxt;
    doc->SetFont(_T("Arial"), _T(" "), 12);

    wxXmlNode *subNode=itmNode->GetChildren();
    bool bUnions=false;
    int iSex=GIS_UNKNOWN;
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
            doc->Cell(190, 10, sEvt, wxPDF_BORDER_NONE, 1);
        }
        if (sType==_T("SEX"))
        {
            wxString sSex=subNode->GetAttribute(_T("Value")).Upper();
            iSex=(sSex==_T("M")?GIS_MALE:(sSex==_T("F")?GIS_FEMALE:GIS_UNKNOWN));
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
                        sTxt=(sSubTyp==_T("HUSB")?_("Father:"):_("Mother:")) + _T(" ") + sSubId + _T(" - ") + m_datas.GetItemFullName(sSubId);
                        doc->Cell(190, 8, sTxt, wxPDF_BORDER_NONE, 1);
                        sEvent=m_datas.GetItemBirth(sSubId);
                        doc->SetFontSize(10);
                        if (!sEvent.IsEmpty())
                            doc->Cell(190, 5, _T("   ") + sEvent, wxPDF_BORDER_NONE, 1);
                        sEvent=m_datas.GetItemDeath(sSubId);
                        if (!sEvent.IsEmpty())
                            doc->Cell(190, 5, _T("   ") + sEvent, wxPDF_BORDER_NONE, 1);
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
                            sTxt=arsSiblings[s] + _T(" - ") + m_datas.GetItemFirstName(arsSiblings[s]);
                            doc->Cell(190, 8, sTxt, wxPDF_BORDER_NONE, 1);
                            sEvent=m_datas.GetItemBirth(arsSiblings[s]);
                            doc->SetFontSize(10);
                            if (!sEvent.IsEmpty())
                                doc->Cell(190, 5, _T("   ") + sEvent, wxPDF_BORDER_NONE, 1);
                            sEvent=m_datas.GetItemDeath(arsSiblings[s]);
                            if (!sEvent.IsEmpty())
                                doc->Cell(190, 5, _T("   ") + sEvent, wxPDF_BORDER_NONE, 1);
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
            if (evtNode!=NULL)
            {
                wxXmlNode *subEvt=evtNode->GetChildren();
                wxString sSubTyp;
                while(subEvt!=NULL)
                {
                    sSubTyp=subEvt->GetAttribute(_T("Type"));
                    wxString sEvtId=subEvt->GetAttribute(_T("GedId"));
                    if (subEvt->GetName()==_T("Event"))
                    {
                        sTxt=m_datas.ParseEvent(subEvt);
                        if (!sTxt.IsEmpty())
                        {
                            doc->SetFontSize(10);
                            doc->Cell(190, 5, sTxt, wxPDF_BORDER_NONE, 1);
                            doc->SetFontSize(12);
                        }
                    }
                    if (((sSubTyp==_T("HUSB"))||(sSubTyp==_T("WIFE")))&&(!sEvtId.IsEmpty())&&(sEvtId!=sItmID))
                    {
                        doc->Cell(190, 8, sEvtId + _T(" - ") + m_datas.GetItemFullName(sEvtId), wxPDF_BORDER_NONE, 1);
                        sEvent=m_datas.GetItemBirth(sEvtId);
                        doc->SetFontSize(10);
                        if (!sEvent.IsEmpty())
                            doc->Cell(190, 5, _T("   ") + sEvent, wxPDF_BORDER_NONE, 1);
                        sEvent=m_datas.GetItemDeath(sEvtId);
                        if (!sEvent.IsEmpty())
                            doc->Cell(190, 5, _T("   ") + sEvent, wxPDF_BORDER_NONE, 1);
                        doc->SetFontSize(12);
                    }
                    if (sSubTyp==_T("CHIL"))
                    {
                        sTxt=_T("      ") + sEvtId + _T(" - ") + m_datas.GetItemFirstName(sEvtId);
                        doc->Cell(190, 8, sTxt, wxPDF_BORDER_NONE, 1);
                        sEvent=m_datas.GetItemBirth(sEvtId);
                        doc->SetFontSize(10);
                        if (!sEvent.IsEmpty())
                            doc->Cell(190, 5, _T("         ") + sEvent, wxPDF_BORDER_NONE, 1);
                        sEvent=m_datas.GetItemDeath(sEvtId);
                        if (!sEvent.IsEmpty())
                            doc->Cell(190, 5, _T("         ") + sEvent, wxPDF_BORDER_NONE, 1);
                        doc->SetFontSize(12);
                    }
                    subEvt=subEvt->GetNext();
                }
            }
        }

        subNode=subNode->GetNext();
    }
}
