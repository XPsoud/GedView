#include "dlgexportpdf.h"

#include "main.h"
#include "appversion.h"

#include <wx/bitmap.h>
#include <wx/xml/xml.h>
#include <wx/statbmp.h>
#include <wx/statline.h>

#include "xpsi.png.h"

DlgExportPdf::DlgExportPdf(wxWindow *parent) : wxDialog(parent, -1, _("Export as pdf file"), wxDefaultPosition, wxDefaultSize)
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
    wxMessageBox(_("Sorry, but this function isn't implemented yet !"), _("Export as Pdf"), wxICON_EXCLAMATION|wxCENTER|wxOK);
}
