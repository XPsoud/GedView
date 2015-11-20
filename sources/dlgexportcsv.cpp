#include "dlgexportcsv.h"

#include "datasmanager.h"

#include <wx/pdfdoc.h>
#include <wx/pdffont.h>
#include <wx/xml/xml.h>
#include <wx/statline.h>

DlgExportCsv::DlgExportCsv(wxWindow *parent) : wxDialog(parent, -1, _("Export as csv file"), wxDefaultPosition, wxDefaultSize),
    m_datas(DatasManager::Get())
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Creating a \"DlgExportCsv\" object\n"));
#endif // __WXDEBUG__

	CreateControls();

	ConnectControls();

	CenterOnParent();
}

DlgExportCsv::~DlgExportCsv()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Destroying a \"DlgExportCsv\" object\n"));
#endif // __WXDEBUG__
}

void DlgExportCsv::CreateControls()
{
    wxBoxSizer *szrMain=new wxBoxSizer(wxVERTICAL), *hszr;

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

void DlgExportCsv::ConnectControls()
{
    m_btnExport->Connect(wxEVT_BUTTON, wxCommandEventHandler(DlgExportCsv::OnBtnExportClicked), NULL, this);
}

void DlgExportCsv::OnBtnExportClicked(wxCommandEvent& event)
{
    m_FName=m_datas.GetCurrentFileName();
    m_FName.SetExt(_T("csv"));

    wxString sMsg=_("Select the \"CSV\" file to create");
    wxString sWlcrd=_("csv files (*.csv)|*.csv|All files (*.*)|*.*");
    wxString sFName=wxFileSelector(sMsg, m_FName.GetPath(), m_FName.GetFullName(), _T("csv"), sWlcrd, wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (sFName.IsEmpty()) return;
    m_FName.Assign(sFName);

    EndModal(wxID_OK);
}

wxString DlgExportCsv::GedItem2CsvLine(wxXmlNode *itmNode)
{
    /*
    if (itmNode==NULL) return;
    wxString sItmID=itmNode->GetAttribute(_T("GedId"));

    doc->AddPage();
    doc->SetMargins(10, 10, 10);
    doc->SetAutoPageBreak(true, 10);
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
            doc->Cell(190, 8, sEvt, wxPDF_BORDER_NONE, 1);
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
                            sTxt=arsSiblings[s] + _T(" - ") + m_datas.GetItemFirstName(arsSiblings[s]);
                            doc->Cell(190, 8, sTxt, wxPDF_BORDER_NONE, 1);
                            sEvent=m_datas.GetItemBirth(arsSiblings[s]);
                            doc->SetFontSize(10);
                            if (!sEvent.IsEmpty())
                                doc->Cell(190, 5, _T("   ") + sEvent, wxPDF_BORDER_NONE, 1);
                            sEvent=m_datas.GetItemDeath(arsSiblings[s]);
                            if (!sEvent.IsEmpty())
                            {
                                if ((sEvent==_("Dead")) && (m_datas.GetItemSex(arsSiblings[s])==GIS_FEMALE))
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
                        {
                            if ((sEvent==_("Dead")) && (sSubTyp==_T("WIFE")))
                                sEvent=_("Dead_F");
                            doc->Cell(190, 5, _T("   ") + sEvent, wxPDF_BORDER_NONE, 1);
                        }
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
                        {
                            if ((sEvent==_("Dead")) && (m_datas.GetItemSex(sEvtId)==GIS_FEMALE))
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
    */
    return wxEmptyString;
}
