#include "dlgtreepdf.h"

#include "treepdfdoc.h"
#include "datasmanager.h"

#include <wx/xml/xml.h>
#include <wx/statline.h>

DlgTreePdf::DlgTreePdf(wxWindow *parent) : wxDialog(parent, -1, _("Create pdf tree"), wxDefaultPosition, wxDefaultSize),
    m_datas(DatasManager::Get())
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Creating a \"DlgTreePdf\" object\n"));
#endif // __WXDEBUG__

    m_SelectedItem=NULL;

	CreateControls();

	ConnectControls();

	CenterOnParent();
}

DlgTreePdf::~DlgTreePdf()
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Destroying a \"DlgTreePdf\" object\n"));
#endif // __WXDEBUG__
}

void DlgTreePdf::CreateControls()
{
    wxStaticText *label;
    wxBoxSizer *szrMain=new wxBoxSizer(wxVERTICAL), *lnszr;

        lnszr=new wxBoxSizer(wxHORIZONTAL);
            label=new wxStaticText(this, -1, _("Tree options:"));
            lnszr->Add(label, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
            lnszr->Add(new wxStaticLine(this, -1), 1, wxLEFT|wxALIGN_CENTER_VERTICAL, 5);
        szrMain->Add(lnszr, 0, wxALL|wxEXPAND, 5);

        lnszr=new wxBoxSizer(wxHORIZONTAL);
            label=new wxStaticText(this, -1, _("Maximum levels of the tree:"));
            lnszr->Add(label, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
            m_cmbLevels=new wxChoice(this, -1);
                for (int i=2; i<MAX_ITER; ++i)
                    m_cmbLevels->Append(wxString::Format(_T("%d"), i));
            lnszr->Add(m_cmbLevels, 1, wxLEFT|wxALIGN_CENTER_VERTICAL, 5);
        szrMain->Add(lnszr, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 5);

        lnszr=new wxBoxSizer(wxHORIZONTAL);
            label=new wxStaticText(this, -1, _("Pdf file format:"));
            lnszr->Add(label, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
            lnszr->Add(new wxStaticLine(this, -1), 1, wxLEFT|wxALIGN_CENTER_VERTICAL, 5);
        szrMain->Add(lnszr, 0, wxALL|wxEXPAND, 5);

        lnszr=new wxBoxSizer(wxHORIZONTAL);
            m_optSize[0]=new wxRadioButton(this, -1, _("Predefined size:"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
            lnszr->Add(m_optSize[0], 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
            m_cmbFormat=new wxChoice(this, -1);
            lnszr->Add(m_cmbFormat, 1, wxLEFT|wxALIGN_CENTER_VERTICAL, 5);
        szrMain->Add(lnszr, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 5);

    SetSizer(szrMain);

    szrMain->SetSizeHints(this);
}

void DlgTreePdf::ConnectControls()
{
    //
}


void DlgTreePdf::SetSelectedItem(wxXmlNode* itmNode)
{
    m_SelectedItem=itmNode;
}
