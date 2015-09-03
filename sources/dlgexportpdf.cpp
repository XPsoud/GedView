#include "dlgexportpdf.h"

#include "datasmanager.h"

#include <wx/pdfdoc.h>
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
    }
    else
    {
        m_optExportType[0]->Enable();
        m_optExportType[0]->SetValue(true);
        m_chkSummary->SetValue(false);
        m_chkSummary->Disable();
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
    }
    else
    {
        m_chkSummary->Enable();
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

    wxPdfDocument doc(wxPORTRAIT, _T("mm"), wxPAPER_A4);
    doc.SetFont(_T("Helvetica"), _T(""), 14);
    doc.AliasNbPages();

    if ((m_optExportType[1]->GetValue() && (m_chkSummary->IsChecked())))
    {
        Summary2Pdf(&doc);
    }

    doc.SaveAsFile(sFName);

    EndModal(wxID_OK);
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
    //
}
