#include "dlgexportcsv.h"

#include "datasmanager.h"

#include <wx/file.h>
#include <wx/pdfdoc.h>
#include <wx/pdffont.h>
#include <wx/xml/xml.h>
#include <wx/statline.h>

const wxChar* szSepName[]={_("Tabulation"), _("Semicolon")};
const wxChar* szSepVal[]={_T("\t"), _T(";")};
const int iSepCount=2;

DlgExportCsv::DlgExportCsv(wxWindow *parent) : wxDialog(parent, -1, _("Export as csv file"), wxDefaultPosition, wxDefaultSize),
    m_datas(DatasManager::Get())
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Creating a \"DlgExportCsv\" object\n"));
#endif // __WXDEBUG__

    // Default separator = tab
    m_sSeparator=_T("\n");

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
    wxStaticText *label;
    wxBoxSizer *szrMain=new wxBoxSizer(wxVERTICAL), *hszr;

        label=new wxStaticText(this, -1, _("CSV export options:"));
        szrMain->Add(label, 0, wxALL, 5);

        hszr=new wxBoxSizer(wxHORIZONTAL);
            label=new wxStaticText(this, -1, _("Columns separator:"));
            hszr->Add(label, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
            m_cmbSeparator=new wxChoice(this, -1);
                for (int i=0; i<iSepCount; ++i)
                {
                    m_cmbSeparator->Append(wxGetTranslation(szSepName[i]));
                }
                m_cmbSeparator->SetSelection(0);
                m_sSeparator=szSepVal[0];

            hszr->Add(m_cmbSeparator, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 5);
        szrMain->Add(hszr, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 5);

        m_chkColHeaders=new wxCheckBox(this, -1, _("Add columns names on the first line"));
            m_chkColHeaders->SetValue(true);
        szrMain->Add(m_chkColHeaders, 0, wxLEFT|wxRIGHT|wxBOTTOM, 5);

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

    // Update separator from combobox
    int iSel=m_cmbSeparator->GetSelection();
    m_sSeparator=szSepVal[iSel];

    wxFile f_out(m_FName.GetFullPath(), wxFile::write);
    if (!f_out.IsOpened())
    {
        wxMessageBox(_("An error occurred while opening the output file"), _("Error"), wxICON_EXCLAMATION);
        return;
    }

    wxString sLine=wxEmptyString;
    // Write columns title if needed
    if (m_chkColHeaders->IsChecked())
    {
        sLine << _("Id") << m_sSeparator << _("Sex") << m_sSeparator;
        sLine << _("Last Name") << m_sSeparator << _("First Name") << m_sSeparator;
        sLine << _("Birth") << m_sSeparator << _("Birth place") << m_sSeparator;
        sLine << _("Death") << m_sSeparator << _("Death place");
        sLine << _T("\n");
    }
    wxXmlNode *root=m_datas.GetDatas();
    wxXmlNode *item=root->GetChildren();
    while(item!=NULL)
    {
        if (item->GetAttribute(_T("Type"))==_T("INDI"))
        {
            sLine << GedItem2CsvLine(item);
            f_out.Write(sLine, *wxConvCurrent);
            sLine=_T("\n");
        }
        item=item->GetNext();
    }
    f_out.Close();

    wxMessageBox(_("CSV file successfully created!"), _("Success"), wxICON_INFORMATION|wxCENTER|wxOK);

    EndModal(wxID_OK);
}

wxString DlgExportCsv::GedItem2CsvLine(wxXmlNode *itmNode)
{
    if (itmNode==NULL) return wxEmptyString;

    wxString sResult=itmNode->GetAttribute(_T("GedId")), sDate, sPlace;
    sResult << m_sSeparator;

    int iSex=m_datas.GetItemSex(itmNode);
    if (iSex==GIS_MALE)
    {
        sResult << _T("M");
    }
    else
    {
        if (iSex==GIS_FEMALE)
        {
            sResult << _T("F");
        }
        else
        {
            sResult << _T("?");
        }
    }
    sResult << m_sSeparator;

    sResult << m_datas.GetItemLastName(itmNode) << m_sSeparator;
    sResult << m_datas.GetItemFirstName(itmNode) << m_sSeparator;

    sDate=m_datas.GetItemBirth(itmNode, true);
    bool bBapm=false;
    if (sDate==g_sUnknownYear)
    {
        sDate=m_datas.GetItemBaptism(itmNode, true);
        bBapm=true;
    }
    if (sDate!=g_sUnknownYear)
    {
        wxChar c=sDate[0];
        if ((c<_T('0'))||(c>_T('9')))
        {
            sDate.Remove(0, 1);
            sDate.Append(c);
        }
        if (bBapm)
        {
            sDate << _T("*");
        }
    }
    sResult << _T("\"") << sDate << _T("\"") << m_sSeparator;

    sPlace=m_datas.GetItemBirthPlace(itmNode);
    if (sPlace.IsEmpty())
    {
        sPlace=m_datas.GetItemBaptismPlace(itmNode);
    }
    sResult << sPlace << m_sSeparator;

    sDate=m_datas.GetItemDeath(itmNode, true);
    bool bBuri=false;
    if (sDate==g_sUnknownYear)
    {
        sDate=m_datas.GetItemBurial(itmNode, true);
        bBuri=true;
    }
    if (sDate!=g_sUnknownYear)
    {
        wxChar c=sDate[0];
        if ((c<_T('0'))||(c>_T('9')))
        {
            sDate.Remove(0, 1);
            sDate.Append(c);
        }
        if (bBuri)
        {
            sDate << _T("*");
        }
    }

    sResult << _T("\"") << sDate << _T("\"") << m_sSeparator;
    sPlace=m_datas.GetItemDeathPlace(itmNode);
    if (sPlace.IsEmpty())
    {
        sPlace=m_datas.GetItemBurialPlace(itmNode);
    }
    sResult << sPlace;

    return sResult;

    /*

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
