#include "dlgtreepdf.h"

#include "treepdfdoc.h"
#include "datasmanager.h"

#include <wx/xml/xml.h>
#include <wx/statline.h>

const int g_iPaperDim[6]={210, 297, 420, 594, 841, 1189};

DlgTreePdf::DlgTreePdf(wxWindow *parent) : wxDialog(parent, -1, _("Create pdf tree"), wxDefaultPosition, wxDefaultSize),
    m_datas(DatasManager::Get())
{
#ifdef __WXDEBUG__
    wxPrintf(_T("Creating a \"DlgTreePdf\" object\n"));
#endif // __WXDEBUG__

    m_SelectedItem=NULL;
    m_RootItem=NULL;
    m_TreePdf=NULL;

	CreateControls();

	UpdateControlsStates();

	ConnectControls();

	CenterOnParent();
}

DlgTreePdf::~DlgTreePdf()
{
    if (m_RootItem!=NULL)
        delete m_RootItem;
    if (m_TreePdf!=NULL)
        delete m_TreePdf;
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
            lnszr->Add(m_cmbLevels, 1, wxLEFT|wxALIGN_CENTER_VERTICAL, 5);
        szrMain->Add(lnszr, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 5);

        m_chkMarrDate=new wxCheckBox(this, -1, _("Add marriage year at each junction point"));
            m_chkMarrDate->SetValue(true);
        szrMain->Add(m_chkMarrDate, 0, wxLEFT|wxRIGHT|wxBOTTOM, 5);

        m_chkSosaNmbr=new wxCheckBox(this, -1, _("Add sosa number under each item"));
            m_chkSosaNmbr->SetValue(true);
        szrMain->Add(m_chkSosaNmbr, 0, wxLEFT|wxRIGHT|wxBOTTOM, 5);

        m_chkSummary=new wxCheckBox(this, -1, _("Add a small summary in the bottom-left corner"));
            m_chkSummary->SetValue(true);
        szrMain->Add(m_chkSummary, 0, wxLEFT|wxRIGHT|wxBOTTOM, 5);

        lnszr=new wxBoxSizer(wxHORIZONTAL);
            label=new wxStaticText(this, -1, _("Pdf file format:"));
            lnszr->Add(label, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
            lnszr->Add(new wxStaticLine(this, -1), 1, wxLEFT|wxALIGN_CENTER_VERTICAL, 5);
        szrMain->Add(lnszr, 0, wxALL|wxEXPAND, 5);

        lnszr=new wxBoxSizer(wxHORIZONTAL);
            m_optSize[0]=new wxRadioButton(this, -1, _("Predefined size:"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
            lnszr->Add(m_optSize[0], 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
            m_cmbFormat=new wxChoice(this, -1);
                for (int i=0; i<5; ++i)
                {
                    m_cmbFormat->Append(wxString::Format(_("A%d Landscape (%d mm x %d mm)"), int(4-i), g_iPaperDim[i+1], g_iPaperDim[i]));
                }
                m_cmbFormat->SetSelection(1); // A3
            lnszr->Add(m_cmbFormat, 1, wxLEFT|wxALIGN_CENTER_VERTICAL, 5);
        szrMain->Add(lnszr, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 5);

        lnszr=new wxBoxSizer(wxHORIZONTAL);
            m_optSize[1]=new wxRadioButton(this, -1, _("Variable width and fixed height of"));
                m_optSize[1]->SetValue(true);
            lnszr->Add(m_optSize[1], 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
            m_cmbHeight=new wxChoice(this, -1);
                for (int i=0; i<6; ++i)
                    m_cmbHeight->Append(wxString::Format(_("%d mm"), g_iPaperDim[i]));
                m_cmbHeight->SetSelection(2); // 420mm
            lnszr->Add(m_cmbHeight, 1, wxLEFT|wxALIGN_CENTER_VERTICAL, 5);
            label=new wxStaticText(this, -1, _("mm"));
            lnszr->Add(label, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 5);
        szrMain->Add(lnszr, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 5);

        lnszr=new wxBoxSizer(wxHORIZONTAL);
            m_optSize[2]=new wxRadioButton(this, -1, _("Variable height and fixed width of"));
            lnszr->Add(m_optSize[2], 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
            m_cmbWidth=new wxChoice(this, -1);
                for (int i=0; i<6; ++i)
                    m_cmbWidth->Append(wxString::Format(_("%d mm"), g_iPaperDim[i]));
                m_cmbWidth->SetSelection(2); // 420mm
            lnszr->Add(m_cmbWidth, 1, wxLEFT|wxALIGN_CENTER_VERTICAL, 5);
            label=new wxStaticText(this, -1, _("mm"));
            lnszr->Add(label, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 5);
        szrMain->Add(lnszr, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 5);

        szrMain->Add(new wxStaticLine(this, -1), 0, wxALL|wxEXPAND, 5);

        lnszr=new wxBoxSizer(wxHORIZONTAL);

            m_btnSave=new wxButton(this, wxID_SAVEAS);
            lnszr->Add(m_btnSave, 0, wxALL, 0);

            lnszr->AddSpacer(10);

            m_btnClose=new wxButton(this, wxID_CLOSE);
            lnszr->Add(m_btnClose, 0, wxALL, 0);

        szrMain->Add(lnszr, 0, wxALL|wxALIGN_RIGHT, 5);

    SetSizer(szrMain);

    szrMain->SetSizeHints(this);

    SetEscapeId(wxID_CLOSE);
}

void DlgTreePdf::ConnectControls()
{
    Bind(wxEVT_BUTTON, &DlgTreePdf::OnSaveAsClicked, this, wxID_SAVEAS);
    Bind(wxEVT_UPDATE_UI, &DlgTreePdf::OnUpdateUI_BtnSaveAs, this, wxID_SAVEAS);
    for (int i=0; i<3; ++i)
    {
        m_optSize[i]->Bind(wxEVT_RADIOBUTTON, &DlgTreePdf::OnOptSizeClicked, this);
    }
}

void DlgTreePdf::UpdateCmbLevels(int maxLvl)
{
    int iMax=(maxLvl==-1?MAX_ITER:maxLvl);
    m_cmbLevels->Clear();
    for (int i=1; i<iMax; ++i)
        m_cmbLevels->Append(wxString::Format(_T("%d"), i));
    m_cmbLevels->SetSelection(m_cmbLevels->GetCount()-1);
}

void DlgTreePdf::UpdateControlsStates()
{
    int iSel=-1;
    for (int i=0; i<3; ++i)
        if (m_optSize[i]->GetValue())
            iSel=i;
    switch(iSel)
    {
        case 0: // Predefined size
            m_cmbHeight->Disable();
            m_cmbWidth->Disable();
            m_cmbFormat->Enable();
            break;
        case 1: // Fixed height
            m_cmbHeight->Enable();
            m_cmbWidth->Disable();
            m_cmbFormat->Disable();
            break;
        case 2: // Fixed width
            m_cmbHeight->Disable();
            m_cmbWidth->Enable();
            m_cmbFormat->Disable();
            break;
    }
}

void DlgTreePdf::GetPaperSize(int* width, int* height)
{
    int iType=-1;
    long lSel=-1;
    for (int i=0; i<3; ++i)
        if (m_optSize[i]->GetValue())
            iType=i;
    switch(iType)
    {
        case 0: // Predefined size
            lSel=m_cmbFormat->GetSelection();
            *width=g_iPaperDim[lSel+1];
            *height=g_iPaperDim[lSel];
            break;
        case 1: // Fixed height
            lSel=m_cmbHeight->GetSelection();
            *width=-1;
            *height=g_iPaperDim[lSel];
            break;
        case 2: // Fixed width
            lSel=m_cmbWidth->GetSelection();
            *width=g_iPaperDim[lSel];
            *height=-1;
            break;
    }
}

void DlgTreePdf::SetSelectedItem(wxXmlNode* itmNode)
{
    m_SelectedItem=itmNode;
    if (itmNode==NULL) return;
    if (m_RootItem!=NULL) delete m_RootItem;
    m_RootItem=new MyTreeItem(0, itmNode);
    if (m_TreePdf!=NULL) delete m_TreePdf;
    m_TreePdf=new TreePdfDoc(m_RootItem);
    m_TreePdf->CreateTree();
    int iLvlMax=m_TreePdf->GetLevelMax();
    UpdateCmbLevels(iLvlMax+1);
}

void DlgTreePdf::OnSaveAsClicked(wxCommandEvent& event)
{
    if (m_TreePdf!=NULL)
        delete m_TreePdf;
    long lMaxLvl=0;
    m_cmbLevels->GetStringSelection().ToLong(&lMaxLvl);
    if (lMaxLvl==0) return;

    wxFileName fname=m_datas.GetCurrentFileName();
    fname.SetExt(_T("pdf"));

    wxString sMsg=_("Select the \"PDF\" file to create");
    wxString sWlcrd=_("Pdf files (*.pdf)|*.pdf|All files (*.*)|*.*");
    wxString sFName=wxFileSelector(sMsg, fname.GetPath(), fname.GetFullName(), _T("pdf"), sWlcrd, wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (sFName.IsEmpty()) return;
    fname.Assign(sFName);

    m_TreePdf=new TreePdfDoc(m_RootItem);
    int iWdth, iHght;
    GetPaperSize(&iWdth, &iHght);

    m_TreePdf->CreateTree(lMaxLvl);
    m_TreePdf->Generate(iWdth, iHght);
    m_TreePdf->DrawConnexions(m_chkMarrDate->IsChecked());

    if (m_chkSosaNmbr->IsChecked())
        m_TreePdf->DrawSosaNumbers();

    if (m_chkSummary->IsChecked())
    {
        m_TreePdf->WriteSummary();
    }

    m_TreePdf->SaveAsFile(sFName);
    wxMessageBox(_("Pdf file successfully created!"), _("Success"), wxICON_INFORMATION|wxCENTER|wxOK);
}

void DlgTreePdf::OnOptSizeClicked(wxCommandEvent& event)
{
    UpdateControlsStates();
}

void DlgTreePdf::OnUpdateUI_BtnSaveAs(wxUpdateUIEvent& event)
{
    bool bEnable=(m_cmbLevels->GetSelection()!=wxNOT_FOUND);
    if (m_optSize[0]->GetValue() && (m_cmbFormat->GetSelection()==wxNOT_FOUND))
        bEnable=false;
    if (m_optSize[1]->GetValue() && (m_cmbHeight->GetSelection()==wxNOT_FOUND))
        bEnable=false;
    if (m_optSize[2]->GetValue() && (m_cmbWidth->GetSelection()==wxNOT_FOUND))
        bEnable=false;

    event.Enable(bEnable);
}
